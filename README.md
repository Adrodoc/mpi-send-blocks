# Setup

## Install prerequisites

### On Ubuntu
```bash
sudo apt install cmake
# MPICH is recommended, but other MPI implementations might work too.
sudo apt install mpich
```

## Setup project
```bash
# Check out the repository:
git clone https://github.com/Adrodoc/mpi-send-blocks.git
# Go to the root directory:
cd mpi-send-blocks
# Make a build directory to place the build output:
cmake -E make_directory "build"
# Generate build system files with cmake:
cmake -E chdir "build" cmake -DCMAKE_BUILD_TYPE=Release ..
# or, starting with CMake 3.13, use a simpler form:
# cmake -DCMAKE_BUILD_TYPE=Release -S . -B "build"
```

# Build
```bash
cmake --build build --config Release
```

# Run
## Locally
```bash
mpirun -n 2 build/main
```

## At LRZ
```bash
salloc --nodes=2 --ntasks-per-node=1 --partition=cm2_inter
mpiexec build/main
exit
```
