MODULE=jukebox_libout123
SHARED_LIB=libjukebox_libout123.so
DEST=/usr
ROOTDIR=..

CFLAGS+= \
    -fPIC \

DEFINES+= \
    -DOPT_MULTI \
    -DOPT_X86_64 \
    -DOPT_GENERIC \
    -DOPT_GENERIC_DITHER \
    -DREAL_IS_FLOAT \
    -DOPT_AVX \
    -DHAVE_CONFIG_H \
    -DBUILDING_OUTPUT_MODULES=1 \

INCLUDES+= \
    -I.. \

LIBS+= \
    -L/lib/x86_64-linux-gnu \
    -L/usr/lib/x86_64-linux-gnu \
    -lasound \
    -lm \

SOURCES= \
    buffer.c \
    compat.c \
    legacy_module.c \
    libout123.c \
    modules/alsa.c \
    stringlists.c \
    wav.c \
    xfermem.c \

include ../Makefile.template
