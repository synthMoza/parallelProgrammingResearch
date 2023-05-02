package common

const ServerAddress string = "127.0.0.1"
const ServerPort string = ":8000"
const UDPPort string = ":5432"
const TCPPort string = ":4321"

type ReplicaInfoMessage struct {
	ID        int
	Addresses []string
}
