FROM alpine:latest
LABEL authors="hikari03"

RUN apk add --no-cache g++ make git && \
    git clone https://github.com/Hikari03/messenger.git && \
    cd messenger/server && \
    make -j$(nproc)

ENTRYPOINT ["/messenger/server/server"]