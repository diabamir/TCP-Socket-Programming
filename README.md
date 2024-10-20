# Client-Server Proxy Application

This repository contains the implementation of a client-server proxy application using C programming language. The two main components are the client (`CLIENT.c`) and the proxy server (`PROXY.c`). 

## Features

- **Client**: A client that interacts with a server via a proxy.
- **Proxy Server**: A proxy server that handles requests from the client and forwards them to the server.
- **Network Communication**: Communication between client, proxy, and server using sockets.
- **Error Handling**: Basic error handling for connection issues and response failures.

## Installation and Setup

### Prerequisites

- GCC or any compatible C compiler
- Linux or Unix-based system (Windows users might need to use WSL or a compatible environment)
- Basic knowledge of socket programming

### Compilation

To compile the `CLIENT.c` and `PROXY.c` files, use the following commands:

```bash
gcc -o client CLIENT.c
gcc -o proxy PROXY.c
