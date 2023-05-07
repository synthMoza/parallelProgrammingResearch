package main

import (
	"os"
	"strconv"

	"github.com/synthMoza/parallelProgrammingResearch/cyberprotect_2sem/distributed_system/node/common"
	"github.com/synthMoza/parallelProgrammingResearch/cyberprotect_2sem/distributed_system/node/master"
	"github.com/synthMoza/parallelProgrammingResearch/cyberprotect_2sem/distributed_system/node/replica"
)

func main() {
	common.InitLoggers()
	defer common.CloseLoggers()

	if len(os.Args) < 2 {
		common.ErrorLogger.Fatal("Node mode must be specified (master/replica)")
	}

	nodeMode := os.Args[1]
	if nodeMode == "master" {
		if len(os.Args) != 3 {
			common.ErrorLogger.Fatal("In master mode amount of replicas must be specified")
		}

		replicasAmount, err := strconv.Atoi(os.Args[2])
		if err != nil {
			common.ErrorLogger.Fatalf("Can't convert \"%s\" to integer", os.Args[2])
		}

		common.InfoLogger.Println("Starting in master mode")
		server := master.Server{ReplicasAmount: replicasAmount}

		err = server.Routine()
		if err != nil {
			common.ErrorLogger.Fatal(err)
		}
	} else if nodeMode == "replica" {
		common.InfoLogger.Println("Starting in replica mode")
		replica := replica.Replica{}

		err := replica.Routine()
		if err != nil {
			common.ErrorLogger.Fatal(err)
		}
	} else {
		common.ErrorLogger.Fatalf("Unknown mode \"%s\" specified", nodeMode)
	}
}
