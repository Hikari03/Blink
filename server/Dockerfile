FROM alpine:latest
LABEL authors="hikari03"

COPY .. /messenger

RUN apk add --no-cache g++ make git && \
    cd messenger/server && \
    make clean_o clean_e && \
    make -j$(nproc)

EXPOSE 6999

ENTRYPOINT ["/messenger/server/blink-server"]