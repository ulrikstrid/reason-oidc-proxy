FROM node:lts-alpine as base

ENV TERM=dumb \
  LD_LIBRARY_PATH=/usr/local/lib:/usr/lib:/lib

WORKDIR /revouch

RUN apk add --no-cache \
  libev libev-dev jq \
  ca-certificates wget \
  bash curl perl-utils \
  git patch gcc g++ musl-dev \
  make m4 util-linux zlib-dev gmp-dev

RUN wget -q -O /etc/apk/keys/sgerrand.rsa.pub https://alpine-pkgs.sgerrand.com/sgerrand.rsa.pub
RUN wget https://github.com/sgerrand/alpine-pkg-glibc/releases/download/2.28-r0/glibc-2.28-r0.apk
RUN apk add --no-cache glibc-2.28-r0.apk

RUN npm install -g esy@next --unsafe-perm

COPY package.json /revouch/esy.json
COPY esy.lock /revouch/esy.lock

RUN esy install
RUN esy b dune build

COPY . /revouch

RUN esy install
RUN esy dune build --profile=docker

RUN mv $(esy command-env --json | jq --raw-output .cur__target_dir)/default/bin/revouch.exe /revouch/main.exe

RUN strip main.exe

FROM scratch

WORKDIR /revouch

COPY --from=base /revouch/main.exe main.exe

ENTRYPOINT ["/revouch/main.exe"]
