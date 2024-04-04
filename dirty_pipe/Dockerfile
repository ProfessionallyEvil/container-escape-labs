FROM ubuntu:22.04

RUN apt update && apt install -y gcc libc6 libc-bin

COPY wait_and_overwrite.sh /
RUN chmod +x wait_and_overwrite.sh
COPY dirty_pipe.c /

WORKDIR /
RUN gcc dirty_pipe.c -o dirty_pipe

CMD ["./wait_and_overwrite.sh"]
