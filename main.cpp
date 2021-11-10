#include <chrono>
#include <iostream>
#include <mpi.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    std::cout << "Initializing..." << std::endl;
    MPI_Init(NULL, NULL);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::cout << rank << ": Allocating window..." << std::endl;

    MPI_Aint win_size = rank == 0 ? 1 : 0;
    uint8_t *mem;
    MPI_Win win;
    MPI_Win_allocate(win_size * sizeof(uint8_t), sizeof(uint8_t), MPI_INFO_NULL, MPI_COMM_WORLD, &mem, &win);
    if (rank == 0)
        *mem = 0;
    MPI_Win_lock_all(0, win);
    MPI_Barrier(MPI_COMM_WORLD);

    using clock = std::chrono::high_resolution_clock;
    using seconds = std::chrono::seconds;

    switch (rank)
    {
    case 0:
    {
        std::cout << "0: Waiting..." << std::endl;
        auto start = clock::now();
        auto end = start + seconds{1};
        while (clock::now() < end)
            ;
        std::cout << "0: Receiving..." << std::endl;
        MPI_Recv(NULL, 0, MPI_UINT8_T, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "0: Finished" << std::endl;
        break;
    }
    case 1:
    {
        std::cout << "1: Sending..." << std::endl;
        MPI_Send(NULL, 0, MPI_UINT8_T, 0, 0, MPI_COMM_WORLD);
        std::cout << "1: Waiting..." << std::endl;
        auto start = clock::now();
        auto end = start + seconds{2};
        while (clock::now() < end)
            ;
        std::cout << "1: Finished" << std::endl;
        break;
    }
    default:
        break;
    }
    MPI_Win_unlock_all(win);

    MPI_Finalize();
    return 0;
}
