# Blink
- Chat application for terminal, written purely in C++.

## Client
- build:
```
git clone https://github.com/Hikari03/Blink.git && \
cd Blink/src && \
make -j$(nproc)
```
- usage: in Blink/src
```
./messenger
```
- then follow instructions
- to exit in connected chat, type `exit` and then enter

## Server
### Use Docker
- download the docker-compose.yml
- `docker compose up -d` or `docker-compose up -d`
- to control the server use `docker attach messenger-server`

### From Source
- build:
```
git clone https://github.com/Hikari03/Blink.git && \
cd Blink/server && \
make -j$(nproc)
```
- usage: in Blink/server
```
./server
```


### Controls
- `help` help
- `q` quit server
- `list` list all connected clients
- `kick <name>` kick out client with name

# TODO

- [ ] better resource management 
  - as of now, it hogs 1 core of CPU for client and in servers case its 1 core per client connected. 
  this is because receiving threads are non-blocking, thus busy waiting - horrible

- [ ] better server terminal
- [x] fix kicking out users
- [x] sending only last *n* messages so everything is stable
- [ ] ability to see online users
- [ ] better syncing of chat when someone connects or leaves
  - this is when new user connects and doesn't see the chat history
    or when someone leaves and the chat history is not updated
