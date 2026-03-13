# ITCH Parser - CMake Build Instructions

This project has been migrated from Visual Studio to CMake for cross-platform compatibility.

## Project Structure

- **itch-core**: Core ITCH parsing library (static library)
- **bist-algo**: Algorithm library for BIST trading (static library)
- **bist-algo-demo**: Demo application for BIST algorithms (executable)
- **itch-monitor**: ITCH monitoring application (executable)
- **orderbook-perf-test**: Order book performance testing tool (executable)

## Requirements

- CMake 3.15 or higher
- C++20 compatible compiler:
  - GCC 10+ (Linux)
  - Clang 10+ (macOS/Linux)
  - MSVC 2019+ (Windows)
  - AppleClang 12+ (macOS)
- Boost 1.70 or higher (required for MultiIndex and Asio)
- fmt 11.x or higher (optional if CMake can fetch dependencies during configure)

## Building

### Quick Build (All Platforms)

```bash
# Create build directory
mkdir build
cd build

# Configure
cmake ..

# Build
cmake --build .

# Optionally specify build type
cmake --build . --config Release
```

### Linux / macOS

```bash
# Debug build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)

# Release build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### Windows (Visual Studio)

```bash
# Generate Visual Studio solution
mkdir build && cd build
cmake -G "Visual Studio 17 2022" -A x64 ..

# Build with CMake
cmake --build . --config Release

# Or open ITCH_Parser.sln in Visual Studio
```

### Windows (MinGW)

```bash
mkdir build && cd build
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
mingw32-make -j8
```

### macOS (Xcode)

```bash
mkdir build && cd build
cmake -G Xcode ..
# Open the generated Xcode project or build from command line:
cmake --build . --config Release
```

## Build Output

Executables will be in: `build/bin/`
Libraries will be in: `build/lib/`

## Running the Applications

```bash
# From build directory
./bin/bist-algo-demo
./bin/itch-monitor
./bin/orderbook-perf-test
```

## IDE Integration

### Visual Studio Code

The project includes `compile_commands.json` generation. Use:
- C/C++ extension
- CMake Tools extension

### CLion

Simply open the project root directory - CLion will automatically detect CMakeLists.txt.

### Visual Studio

Generate the solution:
```bash
cmake -G "Visual Studio 17 2022" -A x64 -B build
```

Then open `build/ITCH_Parser.sln`.
Installing Boost

### Linux

```bash
# Ubuntu/Debian
sudo apt-get install libboost-all-dev

# Fedora/RHEL
sudo dnf install boost-devel

# Arch Linux
sudo pacman -S boost
```

### macOS

```bash
# Using Homebrew
brew install boost
```

### Windows

**Option 1: vcpkg (Recommended)**
```bash
vcpkg install boost
cmake -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake ..
```

**Option 2: Pre-built binaries**
Download from [SourceForge](https://sourceforge.net/projects/boost/files/boost-binaries/) and set `BOOST_ROOT`:
```bash
cmake -DBOOST_ROOT="C:/path/to/boost" ..
```

**Option 3: Build from source**
```bash
# Download and extract boost, then:
bootstrap.bat
b2 install
```

## Troubleshooting

### Boost Not Found

If CMake cannot find Boost, set the `BOOST_ROOT` environment variable:
```bash
# Linux/macOS
export BOOST_ROOT=/path/to/boost
cmake ..

# Windows (PowerShell)
$env:BOOST_ROOT="C:\path\to\boost"
cmake ..
```

Or specify it directly to CMake:
```bash
cmake -DBOOST_ROOT=/path/to/boost ..
```
## Troubleshooting

### Missing Dependencies

The build prefers system-installed dependencies, but `fmt` can also be downloaded automatically during CMake configure with `FetchContent`.

On Linux, you may need to install dependencies:
```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake libfmt-dev

# Fedora/RHEL
sudo dnf install gcc-c++ cmake fmt-devel
```

### Installing fmt

If your environment does not allow network access during CMake configure, install `fmt` manually before configuring:

```bash
# macOS
brew install fmt

# Ubuntu/Debian
sudo apt-get install libfmt-dev

# Fedora/RHEL
sudo dnf install fmt-devel
```

### C++20 Support Issues

Ensure your compiler supports C++20. Check versions:
```bash
g++ --version      # Should be 10+
clang++ --version  # Should be 10+
```

## Migration Notes

- The original Visual Studio solution files (.sln, .vcxproj) are preserved for reference
- Platform-specific definitions (_WIN32_WINNT, _CRT_SECURE_NO_WARNINGS) are automatically applied on Windows
- The CMake build maintains the same project structure and dependencies as the original VS solution
