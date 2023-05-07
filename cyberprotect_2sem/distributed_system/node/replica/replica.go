package replica

import (
	"bytes"
	"encoding/binary"
	"encoding/gob"
	"io/ioutil"
	"net"
	"os"
	"path/filepath"
	"strconv"
	"strings"

	"github.com/synthMoza/parallelProgrammingResearch/cyberprotect_2sem/distributed_system/node/common"
)

type Replica struct {
	storagePath         string
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
	r.replicasConnections = make([]*net.TCPConn, len(message.Addresses))
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

func (r *Replica) initStorage() error {
	initPath := "storage_replica_" + strconv.Itoa(r.replicaId)
	if _, err := os.Stat(initPath); os.IsNotExist(err) {
		err := os.Mkdir(initPath, os.ModePerm)
		if err != nil {
			return err
		}
	} else if err != nil {
		return err
	}

	r.storagePath = initPath
	return nil
}

func (r *Replica) Read(path string) ([]byte, error) {
	data, err := ioutil.ReadFile(filepath.Join(r.storagePath, path))
	return data, err
}

func (r *Replica) Routine() error {
	// Receive broadcast message from master
	common.InfoLogger.Println("Replica: connect to master")
	err := r.connectToMaster()
	if err != nil {
		return err
	}

	// Receive replicas addresses and this replica id, connect to all other replicas
	common.InfoLogger.Println("Replica: connect to other replicas")
	err = r.connectToReplicas()
	if err != nil {
		return err
	}

	// Init this replica storage
	r.initStorage()

	// Run loop for master commands
	for {
		common.InfoLogger.Println("Replica: waiting for commands from master")

		// Receive message from master
		buffer := make([]byte, 1024)
		_, err := r.masterConnection.Read(buffer)
		if err != nil {
			common.ErrorLogger.Printf("Replica: failed to receive command from master, closing")
			return err
		}

		// Decode message
		message := common.Command{}

		ioBuffer := bytes.NewBuffer(buffer)
		gobobj := gob.NewDecoder(ioBuffer)
		gobobj.Decode(&message)

		common.InfoLogger.Printf("Replica: got command from master; type = %d, size = %d, path = %s\n", message.Type, message.Size, message.Path)
		if message.Type == common.Read {
			// Read file
			data, err := r.Read(message.Path)
			if err != nil {
				common.ErrorLogger.Printf("Replica: failed to read file %s\n", message.Path)

				// Send data to master
				reply := common.Command{
					Type: common.Read,
					Size: -1,
					Path: message.Path,
				}

				_, err = r.masterConnection.Write(common.EncodeCommand(reply))
				if err != nil {
					common.ErrorLogger.Fatalf("Replica: failed to send reply to master")
					continue
				}

				continue
			}

			// Sene data to master
			reply := common.Command{
				Type: common.Read,
				Size: len(data),
				Path: message.Path,
			}

			// TODO: handle dead master
			common.InfoLogger.Println("Replica: sending reply to master")
			_, err = r.masterConnection.Write(common.EncodeCommand(reply))
			if err != nil {
				common.ErrorLogger.Fatalf("Replica: failed to send reply to master")
				continue
			}

			common.InfoLogger.Println("Replica: sending data to master")
			_, err = r.masterConnection.Write(data)
			if err != nil {
				common.ErrorLogger.Fatalf("Replica: failed to send data to master")
				continue
			}
		} else if message.Type == common.Write {
			// data, err := r.Write(message.Path)
			// if err != nil {
			// 	common.ErrorLogger.Printf("Replica: failed to read file %s\n", message.Path)
			// }
		} else {
			common.ErrorLogger.Println("Replica: ignoring unknown command from server")
		}
	}
}
