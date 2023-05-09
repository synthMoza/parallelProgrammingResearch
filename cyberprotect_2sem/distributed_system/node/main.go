package node

import (
	"bytes"
	"encoding/gob"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"math/rand"
	"net"
	"net/http"
	"net/url"
	"os"
	"path/filepath"
	"strconv"
	"time"

	"github.com/synthMoza/parallelProgrammingResearch/cyberprotect_2sem/distributed_system/node/common"
)

type NodeCommandType int

const (
	ReadCommand = iota
	ReadReplicaCommand
	WriteCommand
	WriteReplicaCommand
)

type NodeCommand struct {
	Type       NodeCommandType
	Path       string
	statusChan chan int
	dataChan   chan []byte
}

type NodeStatus int

const (
	Ok   NodeStatus = iota // node is alive and works as expected
	Down                   // node failed to reply and marked as down, need to sync data from it
	Dead                   // node is dead and can't execute tasks
)

type NodeInfo struct {
	Status  NodeStatus
	Address string
}

type Node struct {
	Mode string

	cmds        chan NodeCommand
	nodes       map[int]NodeInfo
	id          int
	storagePath string
	httpClient  http.Client
}

func (n *Node) receiveNodesList() (map[int]NodeInfo, error) {
	var nodes map[int]NodeInfo

	// Broadcast this node to be discovered by master
	pc, err := net.ListenPacket("udp4", "")
	if err != nil {
		return nodes, err
	}
	defer pc.Close()

	address, err := net.ResolveUDPAddr("udp4", net.IPv4bcast.String()+":"+strconv.Itoa(common.UDPPort))
	if err != nil {
		return nodes, err
	}

	buffer := make([]byte, 8) // empty message
	if err != nil {
		return nodes, err
	}

	_, err = pc.WriteTo(buffer, address)
	if err != nil {
		return nodes, err
	}

	// Receive nodes list
	buffer = make([]byte, 1024)

	_, _, err = pc.ReadFrom(buffer)
	if err != nil {
		return nodes, err
	}

	ioBuffer := bytes.NewBuffer(buffer)
	gobobj := gob.NewDecoder(ioBuffer)
	gobobj.Decode(&nodes)

	return nodes, nil
}

func (n *Node) ServeHTTPReplica(w http.ResponseWriter, r *http.Request) {
	// Parse the URL
	parsedURL, err := url.Parse(r.RequestURI)
	if err != nil {
		// Wrong url provided
		w.WriteHeader(http.StatusBadRequest)
		return
	}

	path := filepath.Join(n.storagePath, parsedURL.Path)
	common.InfoLogger.Printf("%s: request for %s\n", n.Mode, path)
	if r.Method == "GET" {
		statusChan := make(chan int)
		dataChan := make(chan []byte)

		common.InfoLogger.Printf("%s: adding read command to queue\n", n.Mode)
		n.cmds <- NodeCommand{
			Type:       ReadCommand,
			Path:       path,
			statusChan: statusChan,
			dataChan:   dataChan,
		}

		replyStatus := <-statusChan
		replyData := <-dataChan
		common.InfoLogger.Printf("%s: got reply from read command\n", n.Mode)

		w.WriteHeader(replyStatus)
		w.Write(replyData)
	} else if r.Method == "POST" {
		data, err := io.ReadAll(r.Body)
		if err != nil {
			common.InfoLogger.Printf("%s: failed to read body from request\n", n.Mode)
			w.WriteHeader(http.StatusNoContent)
			return
		}

		statusChan := make(chan int)
		dataChan := make(chan []byte)

		common.InfoLogger.Printf("%s: adding write command to queue\n", n.Mode)
		n.cmds <- NodeCommand{
			Type:       WriteCommand,
			Path:       path,
			statusChan: statusChan,
			dataChan:   dataChan,
		}

		dataChan <- data
		replyStatus := <-statusChan
		common.InfoLogger.Printf("%s: got reply from write command\n", n.Mode)
		w.WriteHeader(replyStatus)
	} else {
		// Unsupported method
		w.WriteHeader(http.StatusMethodNotAllowed)
	}
}

func (n *Node) ReadReplica(replicaId int, path string) ([]byte, int) {
	replyBody := make([]byte, 0)

	// Generate a HTTP request to replica
	requestURL := fmt.Sprintf("http://%s/%s", n.nodes[replicaId].Address, path)
	req, err := http.NewRequest(http.MethodGet, requestURL, nil)
	if err != nil {
		common.ErrorLogger.Printf("%s: couldn't create HTTP request\n", n.Mode)
		return replyBody, http.StatusForbidden
	}

	reply, err := n.httpClient.Do(req)
	if err != nil {
		common.ErrorLogger.Printf("%s: replica %s failed to reply\n", n.Mode, n.nodes[replicaId].Address)
		return replyBody, http.StatusForbidden
	}

	// Some error occured with file
	if reply.StatusCode != http.StatusOK {
		common.InfoLogger.Printf("%s: replica %s reply status is not OK\n", n.Mode, n.nodes[replicaId].Address)
		return replyBody, reply.StatusCode
	}

	// Read body
	replyBody, err = io.ReadAll(reply.Body)
	if err != nil {
		common.ErrorLogger.Printf("%s: failed to read reply from replica %s\n", n.Mode, n.nodes[replicaId].Address)
		return replyBody, http.StatusForbidden
	}

	return replyBody, http.StatusOK
}

func (n *Node) WriteReplica(replicaId int, path string, data []byte) int {
	// Generate a HTTP request to replica
	requestURL := fmt.Sprintf("http://%s%s", n.nodes[replicaId].Address, path)
	bodyReader := bytes.NewReader(data)
	req, err := http.NewRequest(http.MethodPost, requestURL, bodyReader)
	if err != nil {
		common.ErrorLogger.Printf("%s: couldn't create HTTP request\n", n.Mode)
		return http.StatusForbidden
	}

	common.InfoLogger.Printf("%s: write to %s, method %s\n", n.Mode, requestURL, http.MethodPost)
	reply, err := n.httpClient.Do(req)
	if err != nil {
		// TODO: choose another replica
		common.ErrorLogger.Printf("%s: replica %s failed to reply\n", n.Mode, n.nodes[replicaId].Address)
		return http.StatusForbidden
	}

	// Some error occured with write
	if reply.StatusCode != http.StatusOK {
		common.InfoLogger.Printf("%s: replica %s reply status is %d\n", n.Mode, n.nodes[replicaId].Address, reply.StatusCode)
		return reply.StatusCode
	}

	return http.StatusOK
}

func (n *Node) ServeHTTPMaster(w http.ResponseWriter, r *http.Request) {
	// Parse the URL
	parsedURL, err := url.Parse(r.RequestURI)
	if err != nil {
		// Wrong url provided
		w.WriteHeader(http.StatusBadRequest)
		return
	}

	if r.Method == http.MethodGet {
		statusChan := make(chan int)
		dataChan := make(chan []byte)

		common.InfoLogger.Printf("%s: adding read replica command to queue\n", n.Mode)
		n.cmds <- NodeCommand{
			Type:       ReadReplicaCommand,
			Path:       parsedURL.Path,
			statusChan: statusChan,
			dataChan:   dataChan,
		}

		replyStatus := <-statusChan
		replyData := <-dataChan
		common.InfoLogger.Printf("%s: got reply from read replica command\n", n.Mode)

		w.WriteHeader(replyStatus)
		w.Write(replyData)
	} else if r.Method == http.MethodPost {
		reqBody, err := io.ReadAll(r.Body)
		if err != nil {
			common.ErrorLogger.Printf("%s: failed to read HTTP request body", n.Mode)
			w.WriteHeader(http.StatusForbidden)
			return
		}

		statusChan := make(chan int)
		dataChan := make(chan []byte)

		common.InfoLogger.Printf("%s: adding write replica command to queue\n", n.Mode)
		n.cmds <- NodeCommand{
			Type:       WriteReplicaCommand,
			Path:       parsedURL.Path,
			statusChan: statusChan,
			dataChan:   dataChan,
		}

		dataChan <- reqBody
		replyStatus := <-statusChan
		common.InfoLogger.Printf("%s: got reply from write replica command\n", n.Mode)
		w.WriteHeader(replyStatus)
	} else {
		// Unsupported method
		w.WriteHeader(http.StatusMethodNotAllowed)
	}
}

func (n *Node) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	common.InfoLogger.Printf("%s: got %s request from %s\n", n.Mode, r.Method, r.RemoteAddr)

	if n.Mode == "master" {
		n.ServeHTTPMaster(w, r)
	} else if n.Mode == "replica" {
		n.ServeHTTPReplica(w, r)
	}
}

func (n *Node) startHTTPServer() error {
	// Launch manager goroutine to read commands and perform them
	n.cmds = make(chan NodeCommand)
	go func() {
		common.InfoLogger.Printf("%s: starting manager routine, waiting for commands\n", n.Mode)

		for cmd := range n.cmds {
			common.InfoLogger.Printf("%s: got new command %d\n", n.Mode, cmd.Type)

			switch cmd.Type {
			case ReadCommand:
				data, err := os.ReadFile(cmd.Path)
				if err != nil {
					common.InfoLogger.Printf("%s: %s not found\n", n.Mode, cmd.Path)
					cmd.statusChan <- http.StatusNotFound
					cmd.dataChan <- []byte{}
				}

				cmd.statusChan <- http.StatusOK
				cmd.dataChan <- data
			case WriteCommand:
				err := os.WriteFile(cmd.Path, <-cmd.dataChan, os.ModePerm)
				if err != nil {
					common.InfoLogger.Printf("%s: failed to write to %s\n", n.Mode, cmd.Path)
					cmd.statusChan <- http.StatusNoContent
				}

				cmd.statusChan <- http.StatusOK
			case ReadReplicaCommand:
				for {
					// Choose randomly to whom send this read request
					aliveReplicas := make([]int, 0)
					for id, info := range n.nodes {
						if info.Status == Ok && id != n.id {
							aliveReplicas = append(aliveReplicas, id)
						}
					}

					if len(aliveReplicas) == 0 {
						// No replicas left
						common.ErrorLogger.Fatalf("%s: no alive replicas left\n", n.Mode)
					}

					replicaIdIdx := rand.Intn(len(aliveReplicas))
					replicaId := aliveReplicas[replicaIdIdx]

					replyBody, status := n.ReadReplica(replicaId, cmd.Path)
					if status == http.StatusOK || status == http.StatusNotFound {
						// Successful read or not found, reply to client
						cmd.statusChan <- status
						cmd.dataChan <- replyBody
						break
					} else {
						// Replica failed to read - mark it as dead and try again with other replicas
						common.WarningLogger.Printf("%s: replica with id %d failed to read, marking it as dead\n", n.Mode, replicaId)
						replicaInfo := n.nodes[replicaId]
						replicaInfo.Status = Dead
						n.nodes[replicaId] = replicaInfo
					}
				}
			case WriteReplicaCommand:
				// Broadcast write message to everyone
				successfulWrites := 0
				data := <-cmd.dataChan
				for id, info := range n.nodes {
					if id != n.id && info.Status == Ok {
						status := n.WriteReplica(id, cmd.Path, data)
						if status != http.StatusOK {
							common.WarningLogger.Printf("%s: replica with id %d failed to write, marking it as dead\n", n.Mode, id)
							info.Status = Dead
							n.nodes[id] = info
						} else {
							successfulWrites++
						}
					}
				}

				if successfulWrites > 0 {
					cmd.statusChan <- http.StatusOK
				} else {
					cmd.statusChan <- http.StatusForbidden
					common.ErrorLogger.Fatalf("%s: no successfull writes to replicas performed", n.Mode)
				}
			}
		}
	}()

	// Set up HTTP server for clients
	http.Handle("/", n)

	err := http.ListenAndServe(n.nodes[n.id].Address, nil)
	if errors.Is(err, http.ErrServerClosed) {
		common.InfoLogger.Println(n.Mode + ": closing the server for clients")
	}

	return err
}

func (n *Node) receiveReplicas() {
	pc, err := net.ListenPacket("udp4", ":"+strconv.Itoa(common.UDPPort))
	if err != nil {
		panic(err)
	}
	defer pc.Close()

	for {
		// Receive broadcast from new replica
		buffer := make([]byte, 16)
		_, address, err := pc.ReadFrom(buffer)
		if err != nil {
			common.ErrorLogger.Printf("%s: failed to receive new replica broadcast", n.Mode)
			continue
		}

		common.InfoLogger.Printf("%s: got broadcast from replica %s\n", n.Mode, address.String())
		// Add new entry to nodes map
		newId := len(n.nodes)
		n.nodes[newId] = NodeInfo{
			Address: address.String(),
			Status:  Ok,
		}

		// Send new list of nodes
		binaryBuffer := new(bytes.Buffer)
		gobobj := gob.NewEncoder(binaryBuffer)
		gobobj.Encode(n.nodes)

		_, err = pc.WriteTo(binaryBuffer.Bytes(), address)
		if err != nil {
			common.ErrorLogger.Printf("%s: failed to send nodes list to %s", n.Mode, address.String())
			continue
		}

		bytes, err := json.Marshal(n.nodes)
		if err != nil {
			common.ErrorLogger.Printf("%s: failed to marshall nodes list to JSON", n.Mode)
			continue
		}

		common.InfoLogger.Printf("%s: recent nodes list is %s\n", n.Mode, string(bytes))
	}
}

func (n *Node) InitStorage() error {
	if _, err := os.Stat(n.storagePath); os.IsNotExist(err) {
		os.Mkdir(n.storagePath, os.ModePerm)
	}

	return nil
}

func (n *Node) Init() error {
	common.InfoLogger.Println(n.Mode + ": initializing")

	if n.Mode == "replica" {

		// Broadcast message that will be received by master, then
		// receive list of all replicas from master
		common.InfoLogger.Println(n.Mode + ": receive nodes list from master")
		nodes, err := n.receiveNodesList()
		if err != nil {
			return err
		}

		common.InfoLogger.Println(n.Mode + ": received nodes list succesfully")

		n.nodes = nodes
		n.id = len(nodes) - 1 // consider master gives monotonic ids

		n.storagePath = "storage_r" + strconv.Itoa(n.id)
		err = n.InitStorage()
		if err != nil {
			common.ErrorLogger.Fatalf("%s: failed to create storage\n", n.Mode)
		}
	} else {
		// Initialize list of nodes containing only this node
		n.nodes = make(map[int]NodeInfo)

		n.id = 0
		n.nodes[0] = NodeInfo{
			Address: common.GetOutboundIP().String() + ":" + strconv.Itoa(common.ServerPort+n.id),
			Status:  Ok,
		}

		// Run goroutine to listen for UDP broadcasts and connect new clients
		common.InfoLogger.Printf("%s: starting routine to receive replicas\n", n.Mode)
		go n.receiveReplicas()
	}

	// Initialize HTTP client
	defaultTimeout := 5 * time.Second
	n.httpClient = http.Client{Timeout: defaultTimeout}

	// Start HTTP server
	common.InfoLogger.Printf("%s: start HTTP server, address = %s\n", n.Mode, n.nodes[n.id].Address)
	err := n.startHTTPServer()
	if err != nil {
		return err
	}

	return nil
}
