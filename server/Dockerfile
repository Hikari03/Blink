FROM alpine:latest
LABEL authors="hikari03"

COPY .. /messenger

RUN apk add --no-cache g++ make git libsodium-dev && \
    cd messenger/server && \
    make clean_o clean_e && \
    make

EXPOSE 6999

ENTRYPOINT ["/messenger/server/docker-entrypoint.sh"]