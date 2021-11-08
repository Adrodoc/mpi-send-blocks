#include <chrono>
#include <iostream>
#include <mpi.h>
#include <unistd.h>

constexpr size_t ITERATIONS = 1000000;

std::chrono::nanoseconds benchmark_mpi_get(MPI_Win win)
{
    using clock = std::chrono::high_resolution_clock;

    auto start = clock::now();
    for (size_t i = 0; i < ITERATIONS; i++)
    {
        uint8_t dummy;
        MPI_Get(&dummy, 1, MPI_UINT8_T, 0, 0, 1, MPI_UINT8_T, win);
        MPI_Win_flush(0, win);
    }
    auto end = clock::now();
    return end - start;
}

int main(int argc, char *argv[])
{
    std::cout << "Initializing..." << std::endl;
    MPI_Init(NULL, NULL);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::cout << rank << ": Allocating window..." << std::endl;

    MPI_Aint win_size = rank == 0 ? 1 : 0;
    uint8_t *mem;
    MPI_Win win;
    MPI_Win_allocate(win_size * sizeof(uint8_t), sizeof(uint8_t), MPI_INFO_NULL, MPI_COMM_WORLD, &mem, &win);
    if (rank == 0)
        *mem = 0;
    MPI_Win_lock_all(0, win);
    MPI_Barrier(MPI_COMM_WORLD);

    // warm up
    benchmark_mpi_get(win);
    MPI_Barrier(MPI_COMM_WORLD);

    for (int r = 0; r < 8; r++)
        for (int i = 0; i < size; i++)
        {
            if (rank == i)
            {
                auto duration = benchmark_mpi_get(win);
                std::cout << rank << ": Took " << duration.count() << "ns in total or " << (duration.count() / ITERATIONS) << "ns per iteration" << std::endl;
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }

    MPI_Win_unlock_all(win);

    MPI_Finalize();
    return 0;
}
