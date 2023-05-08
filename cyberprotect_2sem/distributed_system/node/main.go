package node

import (
	"bytes"
	"encoding/gob"
	"encoding/json"
	"errors"
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

	if r.Method == "GET" {
		path := filepath.Join(n.storagePath, parsedURL.Path)
		common.InfoLogger.Printf("%s: request for %s\n", n.Mode, path)
		data, err := os.ReadFile(path)
		if err != nil {
			common.InfoLogger.Printf("%s: %s not found\n", n.Mode, path)
			w.WriteHeader(http.StatusNotFound)
			return
		}

		w.Write(data)
	} else if r.Method == "POST" {
		// Broadcast write message to everyone
		w.WriteHeader(http.StatusBadRequest)
	} else {
		// Unsupported method
		w.WriteHeader(http.StatusMethodNotAllowed)
	}
}

func (n *Node) ServeHTTPMaster(w http.ResponseWriter, r *http.Request) {
	// Parse the URL
	parsedURL, err := url.Parse(r.RequestURI)
	if err != nil {
		// Wrong url provided
		w.WriteHeader(http.StatusBadRequest)
		return
	}

	if r.Method == "GET" {
		// Choose randomly to whom send this read request
		aliveReplicas := make([]int, 0)
		for id, info := range n.nodes {
			if info.Status == Ok && id != n.id {
				aliveReplicas = append(aliveReplicas, id)
			}
		}

		replicaIdIdx := rand.Intn(len(aliveReplicas))
		replicaId := aliveReplicas[replicaIdIdx]

		// Generate a HTTP request to client
		req, _ := http.NewRequest("GET", n.nodes[replicaId].Address+"/"+parsedURL.Path, nil)
		req.Header.Add("Command", "read")

		reply, err := n.httpClient.Do(req)
		if err != nil {
			// TODO: choose another replica
			common.ErrorLogger.Printf("%s: replica %s failed to reply\n", n.Mode, n.nodes[replicaId].Address)
			w.WriteHeader(http.StatusForbidden)
			return
		}

		// Some error occured with file
		if reply.StatusCode != http.StatusOK {
			common.InfoLogger.Printf("%s: replica %s reply status is not OK\n", n.Mode, n.nodes[replicaId].Address)
			w.WriteHeader(reply.StatusCode)
			return
		}

		// Read body
		replyBody, err := io.ReadAll(reply.Body)
		if err != nil {
			// TODO: choose another replica
			common.ErrorLogger.Printf("%s: failed to read reply from replica %s\n", n.Mode, n.nodes[replicaId].Address)
			w.WriteHeader(http.StatusForbidden)
			return
		}

		w.Write(replyBody)
	} else if r.Method == "POST" {
		// Broadcast write message to everyone
		w.WriteHeader(http.StatusBadRequest)
	} else {
		// Unsupported method
		w.WriteHeader(http.StatusMethodNotAllowed)
	}
}

func (n *Node) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	common.InfoLogger.Printf("%s: Got %s request from %s\n", n.Mode, r.Method, r.RemoteAddr)

	if n.Mode == "master" {
		n.ServeHTTPMaster(w, r)
	} else if n.Mode == "replica" {
		n.ServeHTTPReplica(w, r)
	}
}

func (n *Node) startHTTPServer() error {
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
		// TODO: fix concurrent access to map
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

		common.InfoLogger.Printf("%s:recent nodes list is %s\n", n.Mode, string(bytes))
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
			Address: common.ServerAddress + ":" + strconv.Itoa(common.ServerPort+n.id),
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
