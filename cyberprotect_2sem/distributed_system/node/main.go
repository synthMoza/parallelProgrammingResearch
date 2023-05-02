package main

import (
	"fmt"
	"log"
	"os"
	"strconv"

	"github.com/synthMoza/parallelProgrammingResearch/cyberprotect_2sem/distributed_system/node/master"
	"github.com/synthMoza/parallelProgrammingResearch/cyberprotect_2sem/distributed_system/node/replica"
)

func main() {
	if len(os.Args) < 2 {
		log.Fatal("Node mode must be specified (master/replica)")
	}

	nodeMode := os.Args[1]
	if nodeMode == "master" {
		if len(os.Args) != 3 {
			log.Fatal("In master mode amount of replicas must be specified")
		}

		replicasAmount, err := strconv.Atoi(os.Args[2])
		if err != nil {
			log.Fatalf("Can't convert \"%s\" to integer", os.Args[2])
		}

		fmt.Println("Starting in master mode")
		server := master.Server{ReplicasAmount: replicasAmount}

		err = server.Routine()
		if err != nil {
			log.Fatal(err)
		}
	} else if nodeMode == "replica" {
		fmt.Println("Starting in replica mode")
		replica := replica.Replica{}

		err := replica.Routine()
		if err != nil {
			log.Fatal(err)
		}
	} else {
		log.Fatalf("Unknown mode \"%s\" specified", nodeMode)
	}
}
