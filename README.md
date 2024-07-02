# Blink
- Chat application for terminal, written purely in C++.



## Client
### Build
```
git clone https://github.com/Hikari03/Blink.git && \
cd Blink/src && \
make -j$(nproc)
```
or use `build.sh`

### Usage
in Blink/src
```
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
- to control the server use `docker attach messenger-server`
- `docker compose down` or `docker-compose down`

### From Source
#### Build
```
git clone https://github.com/Hikari03/Blink.git && \
cd Blink/server && \
make -j$(nproc)
```
or use `build.sh`
#### Usage: 
in Blink/server
```
./blink-server
```


### Controls
- `help` help
- `q` quit server
- `list` list all connected clients
- `kick <name>` kick out client with name

# TODO

- [x] better resource management
- [ ] better server terminal
- [x] fix kicking out users
- [x] sending only last *n* messages so everything is stable
- [ ] ability to see online users
- [x] better syncing of chat when someone connects or leaves
  - this is when new user connects and doesn't see the chat history
    or when someone leaves and the chat history is not updated
- [x] fix server having unexpected behavior when closing server with clients connected
- [ ] implement code for graceful stop in docker 
