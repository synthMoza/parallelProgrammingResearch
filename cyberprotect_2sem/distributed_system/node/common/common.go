package common

import (
	"log"
	"net"
	"os"
)

const ServerAddress string = "127.0.0.1"
const ServerPort int = 8000
const UDPPort int = 5432

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

func GetOutboundIP() net.IP {
	conn, err := net.Dial("udp", "8.8.8.8:80")
	if err != nil {
		log.Fatal(err)
	}
	defer conn.Close()

	localAddr := conn.LocalAddr().(*net.UDPAddr)

	return localAddr.IP
}
