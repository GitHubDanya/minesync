# minesync
A tool for syncing Minecraft worlds across multiple devices.

## Workflow
minesync is composed of two parts - a server and a client. 
- The **Server** is an HTTP server which servers as a central source of truth for all *clients*. Persistent data is stored here, and routed per request.
- The **Client** is an application which probes the server. So far, it has only two functions - uploading and syncing existing local saves.
This workflow provides a really simple and reliable system for persisting state.


## Architecture
The server uses [Crow](https://crowcpp.org/master/) routes for exposing API endpoints.
The client uses an MVP (Model, View, Presenter) pattern, and all client specifications must follow it.

## Quick Start

To get the app running, you can download the latest release from the [releases](https://github.com/GitHubDanya/minesync/releases) tab.
It is strongly recommended to run the server on Linux or WSL.

A make task is written for quick setup of the server, which builds the server and loads it
into a systemd process.

Set up the server using `make install-server`.

Uninstall the server using `make uninstall-server`.

## Building
### Building on Linux using make
Ensure these dependencies are installed (example for Ubuntu/Debian, adjust as needed):

```shell
sudo apt update && sudo apt install -y cmake ninja-build libfltk1.3-dev libcairo2-dev pkg-config libssl-dev libx11-dev
```

The quickest way to build the application is using `make`:

| Project | Command
| :--     | :--
| Both    | `make build`
| Client only | `make build-client`
| Server only | `make build-client`

### Building on Linux manually

Linux target:
```shell
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

cmake --build build --config Release -j$(nproc)

cd build && cpack -G TGZ
```

Windows target:
```shell
PKG_CONFIG_PATH=/usr/x86_64-w64-mingw32/lib/pkgconfig \
cmake -B build-win \
  -DCMAKE_TOOLCHAIN_FILE=mingw-w64.cmake \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build-win --config Release -j$(nproc)

cd build-win && cpack -G ZIP
```

### Building on Windows

Install the following packages using the **MSYS2 UCRT64** terminal:

```shell
pacman -S --needed \
  mingw-w64-ucrt-x86_64-toolchain \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-fltk \
  mingw-w64-ucrt-x86_64-cairo \
  mingw-w64-ucrt-x86_64-pkgconf
```

Windows target:
```shell
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

cmake --build build --config Release

cd build && cpack -G ZIP
```

Linux target:
Build for Linux using `WSL` - follow the same instructions as specified in the Linux section.
