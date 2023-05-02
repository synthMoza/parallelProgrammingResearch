package master

import (
	"bytes"
	"encoding/binary"
	"encoding/gob"
	"errors"
	"fmt"
	"io"
	"net"
	"net/http"
	"sync"

	"github.com/synthMoza/parallelProgrammingResearch/cyberprotect_2sem/distributed_system/node/common"
)

// HTTP server handlers
func getRoot(w http.ResponseWriter, r *http.Request) {
	fmt.Println("Master: Got root request")
	io.WriteString(w, "Consider this is a request of data, so there we'll deliever request to replicas")
}

type Server struct {
	ReplicasAmount int

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

// Connect "replicasAmount" replicas via TCP
func (s *Server) connectReplicas() error {
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

	// Connect all replicas
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

	return nil
}

func (s *Server) configureReplicas() error {
	// Extract all addresses
	replicasAddresses := make([]string, len(s.replicasConnection))
	for _, connection := range s.replicasConnection {
		replicasAddresses = append(replicasAddresses, connection.RemoteAddr().String())
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

// Master routine
func (s *Server) Routine() error {
	var wg sync.WaitGroup

	var connectionErr error
	wg.Add(1)
	go func() {
		defer wg.Done()

		// Conect to all replicas via TC
		fmt.Println("Master: connect to replicas")
		connectionErr = s.connectReplicas()
	}()

	// Broadcast master address to clients
	fmt.Println("Master: network broadcast")
	err := s.broadcastNetwork()
	if err != nil {
		return err
	}

	// Wait for all connections
	wg.Wait()
	if connectionErr != nil {
		return connectionErr
	}

	// Send replicas their ID and addresses of all replicas (for broadcast)
	fmt.Println("Master: configure replicas")
	err = s.configureReplicas()
	if err != nil {
		return err
	}

	// Set up HTTP server for clients
	http.HandleFunc("/", getRoot)

	err = http.ListenAndServe(common.ServerAddress+common.ServerPort, nil)
	if errors.Is(err, http.ErrServerClosed) {
		fmt.Println("Master: closing the server for clients")
	}

	return err
}
