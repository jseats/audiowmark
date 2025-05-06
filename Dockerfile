FROM debian:bookworm

RUN apt-get update && apt-get install -y \
  build-essential automake autoconf libtool autoconf-archive gettext \
  libfftw3-dev libsndfile1-dev libgcrypt20-dev libzita-resampler-dev \
  libmpg123-dev

# Debian's dev package lacks a .pc file; create one so pkg-config can find it
RUN cat >/usr/lib/pkgconfig/zita-resampler.pc <<'EOF'
prefix=/usr
exec_prefix=${prefix}
libdir=${exec_prefix}/lib/x86_64-linux-gnu
includedir=${prefix}/include

Name: zita-resampler
Description: Zita high-quality audio resampler
Version: 1.8.0
Libs: -L${libdir} -lzita-resampler
Cflags: -I${includedir}
EOF

ENV PKG_CONFIG_PATH=/usr/lib/pkgconfig:/usr/lib/x86_64-linux-gnu/pkgconfig

ADD . /audiowmark
WORKDIR /audiowmark

RUN ./autogen.sh
RUN make
RUN make install

VOLUME ["/data"]
WORKDIR /data

ENTRYPOINT ["/usr/local/bin/audiowmark"]
