package main

import (
	"os"

	"github.com/synthMoza/parallelProgrammingResearch/cyberprotect_2sem/distributed_system/node"
	"github.com/synthMoza/parallelProgrammingResearch/cyberprotect_2sem/distributed_system/node/common"
)

func main() {
	common.InitLoggers()
	defer common.CloseLoggers()

	// Get node mode from command line arguments
	if len(os.Args) != 2 {
		common.ErrorLogger.Fatal("Node mode must be specified (master/replica)")
	}

	nodeMode := os.Args[1]
	if nodeMode != "master" && nodeMode != "replica" {
		common.ErrorLogger.Fatalf("Unknown mode \"%s\" specified", nodeMode)
	}

	// Create node instance and launch its routine
	thisNode := node.Node{Mode: nodeMode}
	err := thisNode.Init()
	if err != nil {
		common.ErrorLogger.Fatal(err)
	}
}
