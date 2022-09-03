#include <mpi.h>
#include <stdio.h>

#include "core.h"
#include "comm.h"

int main(int argc, char** argv) {
    mpi::Initialize(&argc, &argv);

    int worldSize = mpi::Comm_World::GetSize();
    int worldRank = mpi::Comm_World::GetRank();

    if (worldRank == 0)
    {
        const int MSG_TAG = 3;
        int msg[] = {7, 2, 7};

        // Send to others message
        for (int j = 1; j < worldSize; ++j)
            mpi::Comm_World::Send(msg, j, MSG_TAG);
    }
    else
    {
        // Receive msg and show it to others
        int msg[3] = {};
        mpi::Comm_World::Recv(msg, 0);
        std::cout << "I'm thread number " << worldRank << ", got message: ";
        for (int j = 0; j < 3; ++j)
            std::cout << msg[j] << " ";
        std::cout << std::endl;
    }
}