AUTOMAKE_OPTIONS = dist-zstd no-dist-gzip

# Home-brew Apple-silicon paths
AM_CPPFLAGS = -I/opt/homebrew/include
AM_LDFLAGS  = -L/opt/homebrew/lib 

# build & test everything (HLS included) during make distcheck
AM_DISTCHECK_CONFIGURE_FLAGS = \
    --with-ffmpeg \
    PKG_CONFIG_PATH=/opt/homebrew/lib/pkgconfig

SUBDIRS = src tests
ACLOCAL_AMFLAGS = -I m4

if COND_WITH_DOCS
  SUBDIRS += docs
endif

EXTRA_DIST = README.adoc Dockerfile
