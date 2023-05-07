package master

import (
	"bytes"
	"encoding/binary"
	"encoding/gob"
	"errors"
	"io"
	"math/rand"
	"net"
	"net/http"
	"net/url"

	"github.com/synthMoza/parallelProgrammingResearch/cyberprotect_2sem/distributed_system/node/common"
)

// HTTP server handlers
func (s *Server) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	common.InfoLogger.Printf("Master: Got %s request from %s\n", r.Method, r.RemoteAddr)

	// Parse the URL
	parsedURL, err := url.Parse(r.RequestURI)
	if err != nil {
		// Wrong url provided
		w.WriteHeader(http.StatusBadRequest)
		return
	}

	if r.Method == "GET" {
		// Choose randomly to whom send this read request
		replicaId := rand.Int() % s.ReplicasAmount
		message := common.Command{
			Type: common.Read,
			Size: 0,
			Path: parsedURL.Path,
		}

		// Send data to replica
		// TODO: handle dead replicas, hold their status somewhere
		common.InfoLogger.Printf("Master: sending read request to replica %d\n", replicaId)
		encodedCommand := common.EncodeCommand(message)
		_, err := s.replicasConnection[replicaId].Write(encodedCommand)
		if err != nil {
			common.ErrorLogger.Fatalf("Failed to send data to replica %d, considering it is dead", replicaId)
			return
		}

		// Wait for answer
		common.InfoLogger.Printf("Master: wait for reply from replica %d\n", replicaId)
		_, err = s.replicasConnection[replicaId].Read(encodedCommand)
		if err != nil {
			common.ErrorLogger.Fatalf("Failed to receive results from replica %d, considering it is dead", replicaId)
			return
		}

		// Receive read bytes
		common.InfoLogger.Printf("Master: receive data from replica %d\n", replicaId)
		message = common.DecodeCommand(encodedCommand)
		if message.Size <= 0 {
			// Some error occured
			// TODO: include error in output
			w.WriteHeader(http.StatusNoContent)
			return
		}

		buffer := make([]byte, message.Size+1)
		_, err = s.replicasConnection[replicaId].Read(buffer)
		if err != nil {
			common.ErrorLogger.Fatalf("Failed to receive results from replica %d, considering it is dead", replicaId)
			return
		}

		io.WriteString(w, string(buffer))
	} else if r.Method == "POST" {
		// Broadcast write message to everyone
		io.WriteString(w, "Request for file: "+parsedURL.Path)
	} else {
		// Unsupported method
		w.WriteHeader(http.StatusMethodNotAllowed)
	}
}

type innerMasterCommand struct {
	cmd  common.CommandType
	size uint32
	name string
	data []byte
}

type Server struct {
	ReplicasAmount     int
	replicasConnection []*net.TCPConn
}

// Broadcast server address for replicas to discover initial master
func (s *Server) broadcastNetwork() error {
	pc, err := net.ListenPacket("udp4", "")
	if err != nil {
		return err
	}
	defer pc.Close()

	address, err := net.ResolveUDPAddr("udp4", net.IPv4bcast.String()+common.UDPPort)
	if err != nil {
		return err
	}

	buffer := make([]byte, 16)
	unsignedReplicasAmount := uint32(s.ReplicasAmount)
	binary.LittleEndian.PutUint32(buffer, unsignedReplicasAmount)
	if err != nil {
		return err
	}

	_, err = pc.WriteTo(buffer, address)
	if err != nil {
		return err
	}

	return nil
}

func (s *Server) configureReplicas() error {
	// Extract all addresses
	replicasAddresses := make([]string, len(s.replicasConnection))
	for idx, connection := range s.replicasConnection {
		replicasAddresses[idx] = connection.RemoteAddr().String()
	}

	// Send a message with info to everyone
	for idx, connection := range s.replicasConnection {
		message := common.ReplicaInfoMessage{
			ID:        idx,
			Addresses: replicasAddresses,
		}

		binaryBuffer := new(bytes.Buffer)

		// create a encoder object
		gobobj := gob.NewEncoder(binaryBuffer)
		gobobj.Encode(message)

		writtenBytes, err := connection.Write(binaryBuffer.Bytes())
		if writtenBytes == 0 || err != nil {
			return err
		}
	}

	return nil
}

func (s *Server) startHTTPServer() error {
	// Set up HTTP server for clients
	http.Handle("/", s)

	err := http.ListenAndServe(common.ServerAddress+common.ServerPort, nil)
	if errors.Is(err, http.ErrServerClosed) {
		common.InfoLogger.Println("Master: closing the server for clients")
	}

	return err
}

// Master routine
func (s *Server) Routine() error {
	// Resolve TCP Address
	address, err := net.ResolveTCPAddr("tcp", common.TCPPort)
	if err != nil {
		return err
	}

	// Start TCP Listener
	listener, err := net.ListenTCP("tcp", address)
	if err != nil {
		return err
	}

	// Broadcast master address to clients
	common.InfoLogger.Println("Master: network broadcast")
	err = s.broadcastNetwork()
	if err != nil {
		return err
	}

	// Wait for all connections, connect all replicas
	connectedReplicasAmount := 0
	for connectedReplicasAmount < s.ReplicasAmount {
		// Accept connection
		connection, err := listener.AcceptTCP()
		if err != nil {
			return err
		}

		// Enable Keepalives
		err = connection.SetKeepAlive(true)
		if err != nil {
			return err
		}

		// Add to slice of replicas connections
		s.replicasConnection = append(s.replicasConnection, connection)
		connectedReplicasAmount++
	}

	// Send replicas their ID and addresses of all replicas (for broadcast)
	common.InfoLogger.Println("Master: configure replicas")
	err = s.configureReplicas()
	if err != nil {
		return err
	}

	common.InfoLogger.Println("Master: starting HTTP server on " + common.ServerAddress + common.ServerPort)
	err = s.startHTTPServer()

	return err
}
