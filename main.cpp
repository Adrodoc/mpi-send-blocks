#include <chrono>
#include <iomanip>
#include <iostream>
#include <mpi.h>
#include <unistd.h>

void spin(std::chrono::nanoseconds duration)
{
    using clock = std::chrono::high_resolution_clock;
    auto start = clock::now();
    auto end = start + duration;
    while (clock::now() < end)
        ;
}

std::string formatted_time()
{
    using namespace std::chrono;
    using clock = high_resolution_clock;

    // get current time
    auto now = clock::now();
    std::ostringstream oss;

    // convert to std::time_t in order to convert to std::tm (broken time)
    auto timer = clock::to_time_t(now);
    // convert to broken time
    std::tm *bt = std::localtime(&timer);
    oss << std::put_time(bt, "%T"); // HH:MM:SS

    auto time_since_epoch = now.time_since_epoch();

    // get number of milliseconds for the current second (remainder after division into seconds)
    auto millis = duration_cast<milliseconds>(time_since_epoch) % 1000;
    oss << '.' << std::setfill('0') << std::setw(3) << millis.count();

    // get number of microseconds for the current millisecond (remainder after division into milliseconds)
    auto micros = duration_cast<microseconds>(time_since_epoch) % 1000;
    oss << '.' << std::setfill('0') << std::setw(3) << micros.count();

    return oss.str();
}

int main(int argc, char *argv[])
{
    std::cout << "Initializing..." << std::endl;
    MPI_Init(NULL, NULL);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int min_rank = 0;
    int max_rank = size - 1;
    bool participate = rank == min_rank || rank == max_rank;

    if (participate)
        std::cout << formatted_time() << ' ' << rank << ": Allocating window..." << std::endl;

    MPI_Aint win_size = rank == 0 ? 1 : 0;
    uint8_t *mem;
    MPI_Win win;
    MPI_Win_allocate(win_size * sizeof(uint8_t), sizeof(uint8_t), MPI_INFO_NULL, MPI_COMM_WORLD, &mem, &win);
    if (rank == 0)
        *mem = 0;
    MPI_Win_lock_all(0, win);
    MPI_Barrier(MPI_COMM_WORLD);

    using seconds = std::chrono::seconds;

    std::cout << formatted_time() << ' ' << rank << ": Waiting..." << std::endl;

    if (rank == min_rank)
    {
        spin(seconds{2});
        std::cout << formatted_time() << ' ' << rank << ": Receiving..." << std::endl;
        MPI_Recv(NULL, 0, MPI_UINT8_T, max_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    else if (rank == max_rank)
    {
        spin(seconds{1});
        std::cout << formatted_time() << ' ' << rank << ": Sending..." << std::endl;
        MPI_Send(NULL, 0, MPI_UINT8_T, min_rank, 0, MPI_COMM_WORLD);
        std::cout << formatted_time() << ' ' << rank << ": Waiting..." << std::endl;
        spin(seconds{2});
    }
    else
        spin(seconds{3});

    std::cout << formatted_time() << ' ' << rank << ": Finished" << std::endl;
    MPI_Win_unlock_all(win);

    MPI_Finalize();
    return 0;
}
