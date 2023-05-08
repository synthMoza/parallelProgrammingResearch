package common

import (
	"bytes"
	"encoding/gob"
	"log"
	"os"
)

const ServerAddress string = "127.0.0.1"
const ServerPort int = 8000
const UDPPort int = 5432

type ReplicaInfoMessage struct {
	ID        int
	Addresses []string
}

type CommandType int

const (
	Read CommandType = iota
	Write
)

type Command struct {
	Type CommandType
	Size int
	Path string
}

func EncodeCommand(c Command) []byte {
	binaryBuffer := new(bytes.Buffer)
	gobobj := gob.NewEncoder(binaryBuffer)
	gobobj.Encode(c)
	return binaryBuffer.Bytes()
}

func DecodeCommand(b []byte) Command {
	c := Command{}

	ioBuffer := bytes.NewBuffer(b)
	gobobj := gob.NewDecoder(ioBuffer)
	gobobj.Decode(&c)

	return c
}

var (
	file          *os.File
	WarningLogger *log.Logger
	InfoLogger    *log.Logger
	ErrorLogger   *log.Logger
)

func InitLoggers() {
	file, err := os.OpenFile("system_logs.txt", os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0666)
	if err != nil {
		log.Fatal(err)
	}

	InfoLogger = log.New(file, "INFO: ", log.Lmicroseconds|log.Ldate|log.Ltime|log.Lshortfile)
	WarningLogger = log.New(file, "WARNING: ", log.Lmicroseconds|log.Ldate|log.Ltime|log.Lshortfile)
	ErrorLogger = log.New(file, "ERROR: ", log.Lmicroseconds|log.Ldate|log.Ltime|log.Lshortfile)
}

func CloseLoggers() {
	file.Close()
}
