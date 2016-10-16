MODULE=jukebox_libmpg123
SHARED_LIB=libjukebox_libmpg123.so
DEST=/usr
ROOTDIR=..

# Needed for server.
DEFINES+= \
    -DNEWOLD_WRITE_SAMPLE \
    -DOPT_3DNOW \
    -DOPT_3DNOW_VINTAGE \
    -DOPT_3DNOWEXT \
    -DOPT_3DNOWEXT_VINTAGE \
    -DOPT_GENERIC \
    -DOPT_GENERIC_DITHER \
    -DOPT_I386 \
    -DOPT_I586 \
    -DOPT_I586_DITHER \
    -DOPT_MMX \
    -DOPT_MULTI \
    -DOPT_SSE \
    -DOPT_SSE_VINTAGE \
    -DREAL_IS_FLOAT \

# Needed for Lenovo Thinkpad.
#DEFINES+= \
#    -DHAVE_CONFIG_H \
#    -DOPT_AVX \
#    -DOPT_GENERIC \
#    -DOPT_GENERIC_DITHER \
#    -DOPT_MULTI \
#    -DOPT_X86_64 \
#    -DREAL_IS_FLOAT \

INCLUDES+= \
    -I.. \

LIBS+= \
    -L/lib/x86_64-linux-gnu \
    -L/usr/lib/x86_64-linux-gnu \
    -lasound \
    -lm \

SOURCES= \
    compat.c \
    dct64.c \
    dct64_i386.c \
    dither.c \
    equalizer.c \
    feature.c \
    format.c \
    frame.c \
    icy2utf8.c \
    icy.c \
    id3.c \
    index.c \
    layer1.c \
    layer2.c \
    layer3.c \
    libmpg123.c \
    lfs_alias.c \
    ntom.c \
    optimize.c \
    parse.c \
    readers.c \
    stringbuf.c \
    synth.c \
    synth_8bit.c \
    synth_real.c \
    synth_s32.c \
    tabinit.c \

# Needed for server.
ASSEMBLY= \
    dct36_3dnow.S \
    dct36_3dnowext.S \
    dct36_sse.S \
    dct64_3dnow.S \
    dct64_3dnowext.S \
    dct64_mmx.S \
    dct64_sse.S \
    dct64_sse_float.S \
    equalizer_3dnow.S \
    getcpuflags.S \
    synth_3dnow.S \
    synth_3dnowext.S \
    synth_i586.S \
    synth_i586_dither.S \
    synth_mmx.S \
    synth_sse.S \
    synth_sse_float.S \
    synth_sse_s32.S \
    synth_stereo_sse_float.S \
    synth_stereo_sse_s32.S \
    tabinit_mmx.S \

# Needed for Lenovo Thinkpad.
#ASSEMBLY= \
#    dct36_avx.S \
#    dct36_x86_64.S \
#    dct64_avx.S \
#    dct64_avx_float.S \
#    dct64_x86_64.S \
#    dct64_x86_64_float.S \
#    getcpuflags_x86_64.S \
#    synth_x86_64.S \
#    synth_x86_64_float.S \
#    synth_x86_64_s32.S \
#    synth_stereo_x86_64.S \
#    synth_stereo_x86_64_float.S \
#    synth_stereo_x86_64_s32.S \
#    synth_stereo_avx.S \
#    synth_stereo_avx_float.S \
#    synth_stereo_avx_s32.S \

include ../Makefile.template
