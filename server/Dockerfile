FROM alpine:latest
LABEL authors="hikari03"

COPY . /blink

RUN apk add --no-cache g++ make git libsodium-dev && \
    cd blink/server && \
    make

EXPOSE 6999

ENTRYPOINT ["/blink/server/docker-entrypoint.sh"]