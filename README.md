# Blink
- Chat application for terminal, written purely in C++.
- As of right now, sending and receiving messages works but only so much and then server crashed. Easy to fix tho.

## Client
- installation:
```
git@github.com:Hikari03/Blink.git \
cd Blink/src \
make -j$(nproc)
```
- usage: in Blink/src
```
./messenger
```
- then follow instrucions
- to exit in connected chat, type `exit` and then enter

## Server
- download the docker-compose.yml
- `docker compose up -d` or `docker-compose up -d`
- to control the server use `docker attach messenger-server`

### Controls
- so far only `q` is implemented (quit server)

# TODO

- [ ] better server terminal
- [ ] sending only last *n* messages so everything is stable
- [ ] ability to see online users
- [ ] better syncing of chat when someone connects or leaves 