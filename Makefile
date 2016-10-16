MODULE=jukebox
EXECUTABLE=jukebox
DEST=/usr
DAEMON=jukebox

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

INCLUDES+= \

LIBS+= \
    -L/usr/lib \
    -ljukebox_libmpg123 \
    -ljukebox_libout123 \
    -L/lib/x86_64-linux-gnu \
    -L/usr/lib/x86_64-linux-gnu \
    -lasound \
    -lexpat \
    -lm \
    -lpthread \

SOURCES= \
    audio.c \
    common.c \
    compat.c \
    control_generic.c \
    equalizer.c \
    Filter.c \
    genre.c \
    getlopt.c \
    httpget.c \
    Jukebox.c \
    local.c \
    metaprint.c \
    mpg123.c \
    playlist.c \
    resolver.c \
    streamdump.c \
    sysutil.c \
    term.c \
    XmlReader.c \

include Makefile.template
