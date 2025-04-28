# QtMessenger
Simple C++/Qt chat application with client-server architecture

# Build and Run

## Prerequisites

- **C++17 compiler**
- **Qt 6.5.0+**
- **Boost libraries**
- **CMake 3.5+**

## Dependencies Installation

**Ubuntu/Debian**

```bash
sudo apt-get update
sudo apt-get install -y \
    cmake \
    qt6-base-dev \
    libboost-system-dev \
    libsqlite3-dev
```

## Building the Project

```bash
git clone https://github.com/niklvdanya/QtMessenger.git
cd QtMessenger
```
```bash
mkdir build
cd build
```

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

## Running the Application
```bash
# Start the server
./chat_server_mt

# Launch the client
./chat_client
```