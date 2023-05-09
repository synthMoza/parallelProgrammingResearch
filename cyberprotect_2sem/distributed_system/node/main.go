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
	Dead                   // node is dead and can't execute tasks
)

type NodeInfo struct {
	Delivered int
	Status    NodeStatus
	Address   string
	Mode      string
}

type BroadcastMessage struct {
	Type      NodeCommandType
	Path      string
	SenderId  int
	SenderSeq int
}

type Node struct {
	Mode string

	receiveBuffer []BroadcastMessage
	sendSeq       int
	cmds          chan NodeCommand
	nodes         map[int]NodeInfo
	id            int
	storagePath   string
	httpClient    http.Client
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
	common.InfoLogger.Printf("%s_%d: request for %s\n", n.Mode, n.id, path)
	if r.Method == "GET" {
		statusChan := make(chan int)
		dataChan := make(chan []byte)

		common.InfoLogger.Printf("%s_%d: adding read command to queue\n", n.Mode, n.id)
		n.cmds <- NodeCommand{
			Type:       ReadCommand,
			Path:       path,
			statusChan: statusChan,
			dataChan:   dataChan,
		}

		replyStatus := <-statusChan
		replyData := <-dataChan
		common.InfoLogger.Printf("%s_%d: got reply from read command\n", n.Mode, n.id)

		w.WriteHeader(replyStatus)
		w.Write(replyData)
	} else if r.Method == "POST" {
		data, err := io.ReadAll(r.Body)
		if err != nil {
			common.InfoLogger.Printf("%s_%d: failed to read body from request\n", n.Mode, n.id)
			w.WriteHeader(http.StatusNoContent)
			return
		}

		// BROADCAST RECEIVE
		senderIdStr := r.Header.Get("id")
		senderSeqStr := r.Header.Get("sendSeq")
		senderMode := r.Header.Get("sendMode")

		senderId, err := strconv.Atoi(senderIdStr)
		if err != nil {
			// Wrong message, these fields must be included
			common.WarningLogger.Printf("%s_%d: no required header values in HTTP request\n", n.Mode, n.id)
			w.WriteHeader(http.StatusBadRequest)
			return
		}

		senderSeq, err := strconv.Atoi(senderSeqStr)
		if err != nil {
			// Wrong message, these fields must be included
			common.WarningLogger.Printf("%s_%d: no required header values in HTTP request\n", n.Mode, n.id)
			w.WriteHeader(http.StatusBadRequest)
			return
		}

		// BROADCAST RECEIVE - broadcast to every other replica
		if senderMode == "master" {
			for idx, node := range n.nodes {
				if node.Status == Ok && node.Mode != "master" && idx != n.id {
					common.InfoLogger.Printf("%s_%d: broadcasting to replica_%d\n", n.Mode, n.id, idx)
					n.WriteReplica(idx, parsedURL.Path, data)
				}
			}
		}

		// BROADCAST RECEIVE - Save message to buffer
		broadcastMessage := BroadcastMessage{
			Type:      WriteCommand,
			Path:      path,
			SenderId:  senderId,
			SenderSeq: senderSeq,
		}
		common.InfoLogger.Printf("%s_%d: got broadcast message; path = %s, sender id = %d, sender seq = %d", n.Mode, n.id, path, senderId, senderSeq)
		n.receiveBuffer = append(n.receiveBuffer, broadcastMessage)

		// BROADCAST RECEIVE - Check buffer for existing messages
		hasMessage := true
		for hasMessage {
			hasMessage = false
			for _, msg := range n.receiveBuffer {
				if n.nodes[msg.SenderId].Delivered == msg.SenderSeq {
					hasMessage = true

					// BROADCAST RECEIVE - deliver to the application
					statusChan := make(chan int)
					dataChan := make(chan []byte)

					common.InfoLogger.Printf("%s_%d: adding write command to queue\n", n.Mode, n.id)
					n.cmds <- NodeCommand{
						Type:       msg.Type,
						Path:       msg.Path,
						statusChan: statusChan,
						dataChan:   dataChan,
					}

					dataChan <- data
					replyStatus := <-statusChan
					common.InfoLogger.Printf("%s_%d: got reply from write command\n", n.Mode, n.id)
					w.WriteHeader(replyStatus)

					nodeInfo := n.nodes[msg.SenderId]
					nodeInfo.Delivered++
					n.nodes[msg.SenderId] = nodeInfo
				}
			}
		}

	} else if r.Method == http.MethodPut {
		// Receive new nodes list from master
		data, err := io.ReadAll(r.Body)
		if err != nil {
			common.InfoLogger.Printf("%s_%d: failed to read body from request\n", n.Mode, n.id)
			w.WriteHeader(http.StatusNoContent)
			return
		}

		var nodes map[int]NodeInfo
		ioBuffer := bytes.NewBuffer(data)
		gobobj := gob.NewDecoder(ioBuffer)
		gobobj.Decode(&nodes)

		// Add missing nodes or modify nodes that changed status
		for idx, info := range nodes {
			if _, ok := n.nodes[idx]; !ok {
				n.nodes[idx] = NodeInfo{
					Status:    info.Status,
					Delivered: 0,
					Mode:      info.Mode,
					Address:   info.Address,
				}
			} else {
				if n.nodes[idx].Status != info.Status {
					tmpInfo := n.nodes[idx]
					tmpInfo.Status = info.Status
					n.nodes[idx] = tmpInfo
				}
			}
		}

		w.WriteHeader(http.StatusOK)

		bytes, err := json.Marshal(n.nodes)
		if err != nil {
			common.ErrorLogger.Printf("%s_%d: failed to marshall nodes list to JSON", n.Mode, n.id)
		} else {
			common.InfoLogger.Printf("%s_%d: recent nodes list is %s\n", n.Mode, n.id, string(bytes))
		}

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
		common.ErrorLogger.Printf("%s_%d: couldn't create HTTP request\n", n.Mode, n.id)
		return replyBody, http.StatusForbidden
	}

	reply, err := n.httpClient.Do(req)
	if err != nil {
		common.ErrorLogger.Printf("%s_%d: replica %s failed to reply\n", n.Mode, n.id, n.nodes[replicaId].Address)
		return replyBody, http.StatusForbidden
	}

	// Some error occured with file
	if reply.StatusCode != http.StatusOK {
		common.InfoLogger.Printf("%s_%d: replica %s reply status is not OK\n", n.Mode, n.id, n.nodes[replicaId].Address)
		return replyBody, reply.StatusCode
	}

	// Read body
	replyBody, err = io.ReadAll(reply.Body)
	if err != nil {
		common.ErrorLogger.Printf("%s_%d: failed to read reply from replica %s\n", n.Mode, n.id, n.nodes[replicaId].Address)
		return replyBody, http.StatusForbidden
	}

	return replyBody, http.StatusOK
}

func (n *Node) WriteReplica(replicaId int, path string, data []byte) int {
	// Generate a HTTP request to replica
	requestURL, err := url.JoinPath("http://"+n.nodes[replicaId].Address, path)
	if err != nil {
		common.ErrorLogger.Printf("%s_%d: couldn't create URL: %s\n", n.Mode, n.id, requestURL)
		return http.StatusForbidden
	}

	bodyReader := bytes.NewReader(data)
	req, err := http.NewRequest(http.MethodPost, requestURL, bodyReader)
	if err != nil {
		common.ErrorLogger.Printf("%s_%d: couldn't create HTTP request\n", n.Mode, n.id)
		return http.StatusForbidden
	}

	// BROADCAST SEND - include Node ID and send sequence in message
	req.Header.Add("id", strconv.Itoa(n.id))
	req.Header.Add("sendSeq", strconv.Itoa(n.sendSeq))
	req.Header.Add("sendMode", n.Mode)

	common.InfoLogger.Printf("%s_%d: write to %s, method %s\n", n.Mode, n.id, requestURL, http.MethodPost)
	n.sendSeq++ // BROADCAST SEND - increment send sequence

	reply, err := n.httpClient.Do(req)
	if err != nil {
		common.ErrorLogger.Printf("%s_%d: replica %s failed to reply\n", n.Mode, n.id, n.nodes[replicaId].Address)
		return http.StatusForbidden
	}

	// Some error occured with write
	if reply.StatusCode != http.StatusOK {
		common.InfoLogger.Printf("%s_%d: replica %s reply status is %d\n", n.Mode, n.id, n.nodes[replicaId].Address, reply.StatusCode)
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

		common.InfoLogger.Printf("%s_%d: adding read replica command to queue\n", n.Mode, n.id)
		n.cmds <- NodeCommand{
			Type:       ReadReplicaCommand,
			Path:       parsedURL.Path,
			statusChan: statusChan,
			dataChan:   dataChan,
		}

		replyStatus := <-statusChan
		replyData := <-dataChan
		common.InfoLogger.Printf("%s_%d: got reply from read replica command\n", n.Mode, n.id)

		w.WriteHeader(replyStatus)
		w.Write(replyData)
	} else if r.Method == http.MethodPost {
		reqBody, err := io.ReadAll(r.Body)
		if err != nil {
			common.ErrorLogger.Printf("%s_%d: failed to read HTTP request body", n.Mode, n.id)
			w.WriteHeader(http.StatusForbidden)
			return
		}

		statusChan := make(chan int)
		dataChan := make(chan []byte)

		common.InfoLogger.Printf("%s_%d: adding write replica command to queue\n", n.Mode, n.id)
		n.cmds <- NodeCommand{
			Type:       WriteReplicaCommand,
			Path:       parsedURL.Path,
			statusChan: statusChan,
			dataChan:   dataChan,
		}

		dataChan <- reqBody
		replyStatus := <-statusChan
		common.InfoLogger.Printf("%s_%d: got reply from write replica command\n", n.Mode, n.id)
		w.WriteHeader(replyStatus)
	} else {
		// Unsupported method
		w.WriteHeader(http.StatusMethodNotAllowed)
	}
}

func (n *Node) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	common.InfoLogger.Printf("%s_%d: got %s request from %s\n", n.Mode, n.id, r.Method, r.RemoteAddr)

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
		common.InfoLogger.Printf("%s_%d: starting manager routine, waiting for commands\n", n.Mode, n.id)

		for cmd := range n.cmds {
			common.InfoLogger.Printf("%s_%d: got new command %d\n", n.Mode, n.id, cmd.Type)

			switch cmd.Type {
			case ReadCommand:
				data, err := os.ReadFile(cmd.Path)
				if err != nil {
					common.InfoLogger.Printf("%s_%d: %s not found\n", n.Mode, n.id, cmd.Path)
					cmd.statusChan <- http.StatusNotFound
					cmd.dataChan <- []byte{}
				}

				cmd.statusChan <- http.StatusOK
				cmd.dataChan <- data
			case WriteCommand:
				err := os.WriteFile(cmd.Path, <-cmd.dataChan, os.ModePerm)
				if err != nil {
					common.InfoLogger.Printf("%s_%d: failed to write to %s\n", n.Mode, n.id, cmd.Path)
					cmd.statusChan <- http.StatusNoContent
				}

				cmd.statusChan <- http.StatusOK
			case ReadReplicaCommand:
				updateNodesList := false
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
						common.WarningLogger.Printf("%s_%d: replica with id %d failed to read, marking it as dead\n", n.Mode, n.id, replicaId)
						replicaInfo := n.nodes[replicaId]
						replicaInfo.Status = Dead
						n.nodes[replicaId] = replicaInfo

						updateNodesList = true
					}
				}

				if updateNodesList {
					common.InfoLogger.Printf("%s_%d: updating replicas nodes list\n", n.Mode, n.id)
					binaryBuffer := new(bytes.Buffer)
					gobobj := gob.NewEncoder(binaryBuffer)
					gobobj.Encode(n.nodes)

					for idx, info := range n.nodes {
						if info.Status == Ok && idx != n.id {
							common.InfoLogger.Printf("%s_%d: sending recent nodes list to replica_%d", n.Mode, n.id, idx)
							err := n.sendNodeList(idx, binaryBuffer.Bytes())
							if err != nil {
								common.ErrorLogger.Printf("%s_%d: failed to send nodes list to replica_%d", n.Mode, n.id, idx)
							}
						}
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
							// we will mark replicas as dead only when we an not read from them, hope than some of broadcast will reach them
							// TODO: check replicas consistency
							common.WarningLogger.Printf("%s_%d: replica with id %d failed to write\n", n.Mode, n.id, id)
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

func (n *Node) sendNodeList(idx int, list []byte) error {
	bodyReader := bytes.NewReader(list)
	req, err := http.NewRequest(http.MethodPut, "http://"+n.nodes[idx].Address, bodyReader)
	if err != nil {
		common.ErrorLogger.Printf("%s_%d: couldn't create HTTP request: %s\n", n.Mode, n.id, err.Error())
		return err
	}

	reply, err := n.httpClient.Do(req)
	if err != nil {
		common.ErrorLogger.Printf("%s_%d: failed to receive reply from replica_%d\n", n.Mode, n.id, idx)
		return err
	}

	if reply.StatusCode != http.StatusOK {
		return errors.New("Bad replica reply")
	}

	return nil
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
			common.ErrorLogger.Printf("%s_%d: failed to receive new replica broadcast", n.Mode, n.id)
			continue
		}

		common.InfoLogger.Printf("%s_%d: got broadcast from replica %s\n", n.Mode, n.id, address.String())
		// Add new entry to nodes map
		newId := len(n.nodes)
		n.nodes[newId] = NodeInfo{
			Address:   address.String(),
			Status:    Ok,
			Delivered: 0,
			Mode:      "replica",
		}

		// Send new list of nodes to this node via UDP
		binaryBuffer := new(bytes.Buffer)
		gobobj := gob.NewEncoder(binaryBuffer)
		gobobj.Encode(n.nodes)

		_, err = pc.WriteTo(binaryBuffer.Bytes(), address)
		if err != nil {
			common.ErrorLogger.Printf("%s_%d: failed to send nodes list to %s", n.Mode, n.id, address.String())
			continue
		}

		// Update nodes list on EVERY already connected replicas
		for idx, info := range n.nodes {
			if info.Status == Ok && idx != n.id && idx != newId {
				common.InfoLogger.Printf("%s_%d: sending recent nodes list to replica_%d", n.Mode, n.id, idx)
				err := n.sendNodeList(idx, binaryBuffer.Bytes())
				if err != nil {
					common.ErrorLogger.Printf("%s_%d: failed to send nodes list to replica_%d", n.Mode, n.id, idx)
				}
			}
		}

		bytes, err := json.Marshal(n.nodes)
		if err != nil {
			common.ErrorLogger.Printf("%s_%d: failed to marshall nodes list to JSON", n.Mode, n.id)
			continue
		}

		common.InfoLogger.Printf("%s_%d: recent nodes list is %s\n", n.Mode, n.id, string(bytes))
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
			Address:   common.GetOutboundIP().String() + ":" + strconv.Itoa(common.ServerPort+n.id),
			Status:    Ok,
			Delivered: 0,
			Mode:      n.Mode,
		}

		// Run goroutine to listen for UDP broadcasts and connect new clients
		common.InfoLogger.Printf("%s_%d: starting routine to receive replicas\n", n.Mode, n.id)
		go n.receiveReplicas()
	}

	n.sendSeq = 0

	// Initialize HTTP client
	defaultTimeout := 5 * time.Second
	n.httpClient = http.Client{Timeout: defaultTimeout}

	// Start HTTP server
	common.InfoLogger.Printf("%s_%d: start HTTP server, address = %s\n", n.Mode, n.id, n.nodes[n.id].Address)
	err := n.startHTTPServer()
	if err != nil {
		return err
	}

	return nil
}
