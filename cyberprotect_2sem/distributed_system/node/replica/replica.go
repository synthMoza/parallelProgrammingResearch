package replica

import (
	"bytes"
	"encoding/binary"
	"encoding/gob"
	"fmt"
	"net"
	"strings"

	"github.com/synthMoza/parallelProgrammingResearch/cyberprotect_2sem/distributed_system/node/common"
)

type Replica struct {
	replicasAmount      int
	replicaId           int
	masterConnection    *net.TCPConn
	replicasConnections []*net.TCPConn
}

// Receive broadcast message from master, extract address and replicas count
func (r *Replica) connectToMaster() error {
	// Receive master address and replicas amount from broadcast message
	pc, err := net.ListenPacket("udp4", common.UDPPort)
	if err != nil {
		panic(err)
	}
	defer pc.Close()

	buffer := make([]byte, 16)
	_, address, err := pc.ReadFrom(buffer)
	if err != nil {
		return err
	}

	unsignedReplicasAmount := binary.LittleEndian.Uint32(buffer)
	r.replicasAmount = int(unsignedReplicasAmount)

	// Resolve TCP Address
	tcpAddress, err := net.ResolveTCPAddr("tcp", strings.Split(address.String(), ":")[0]+common.TCPPort)
	if err != nil {
		return err
	}

	// Open TCP Connection
	r.masterConnection, err = net.DialTCP("tcp", nil, tcpAddress)
	if err != nil {
		return err
	}

	// Enable Keepalives
	err = r.masterConnection.SetKeepAlive(true)
	if err != nil {
		return err
	}

	return nil
}

// Receive this replica id and all other replicas addresses, connect to them
func (r *Replica) connectToReplicas() error {
	// Receive message
	buffer := make([]byte, 1024)
	readBytes, err := r.masterConnection.Read(buffer)
	if readBytes == 0 || err != nil {
		return err
	}

	// Decode message
	message := common.ReplicaInfoMessage{}

	ioBuffer := bytes.NewBuffer(buffer)
	gobobj := gob.NewDecoder(ioBuffer)
	gobobj.Decode(&message)

	r.replicaId = message.ID
	for idx, address := range message.Addresses {
		if idx == r.replicaId {
			continue
		}

		// Resolve TCP Address
		tcpAddress, err := net.ResolveTCPAddr("tcp", address)
		if err != nil {
			return err
		}

		// Open TCP Connection
		r.replicasConnections[idx], err = net.DialTCP("tcp", nil, tcpAddress)
		if err != nil {
			return err
		}

		// Enable Keepalives
		err = r.masterConnection.SetKeepAlive(true)
		if err != nil {
			return err
		}
	}

	return nil
}

func (r *Replica) Routine() error {
	// Receive broadcast message from master
	fmt.Println("Replica: connect to master")
	err := r.connectToMaster()
	if err != nil {
		return err
	}

	// Receive replicas addresses and this replica id, connect to all other replicas
	fmt.Println("Replica: connect to other replicas")
	err = r.connectToReplicas()
	if err != nil {
		return err
	}

	return nil
}
