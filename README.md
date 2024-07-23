# Blink
- Chat application for terminal, written purely in C++.

## Dependencies
### Shared
- `make`, `g++` with c++23 support
### Client
- `ncursesw` for terminal UI
### Server
- `docker` and `docker-compose` optionally for server

## Client
### Build
``` bash
git clone https://github.com/Hikari03/Blink.git && \
cd Blink/src && \
make
```

#### Static build
If you want to build static binary, you can do so by running in src directory:
``` bash
make static
```

### Usage
in Blink/src
``` bash
./blink
```
- then follow instructions
- to exit in connected chat, type `/exit` and then enter

## Server
### Use Docker
#### With scripts
You can get the `docker-update-start.sh`, `docker-attach.sh` and `docker-shutdown.sh`.
With them, you can control the server with ease.
- `docker-update-start.sh` will automatically update the image if needed and start the server
- `docker-attach.sh` will attach you to sever console. Use `help` to print available commands. When you want to exit console, use `CTRL + P, CTRL + Q`.
- `docker-shutdown.sh` will shutdown the container

#### Manually
- download the docker-compose.yml
- `docker compose up -d` or `docker-compose up -d`
- to control the server use `docker attach blink-server`
- `docker compose down` or `docker-compose down`

### From Source
#### Build
``` bash
git clone https://github.com/Hikari03/Blink.git && \
cd Blink/server && \
make
```
#### Usage: 
in Blink/server
``` bash
./blink-server
```


### Controls
- `help` help
- `q` quit server
- `list` list all connected clients
- `kick <name>` kick out client with name
- `ipban <name>` ban client with name
- `ipunban <name>` unban client with name
- `ipbans` list all banned clients

# TODO

- [x] better resource management
- [x] better server terminal
- [x] fix kicking out users
- [x] sending only last *n* messages so everything is stable
- [ ] ability to see online users
- [x] better syncing of chat when someone connects or leaves
  - this is when new user connects and doesn't see the chat history
    or when someone leaves and the chat history is not updated
- [x] fix server having unexpected behavior when closing server with clients connected
- [ ] implement code for graceful stop in docker 
- [ ] transfer ncurses static build to librender that will make static library
