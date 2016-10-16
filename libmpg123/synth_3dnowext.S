#include "libmpg123/mangle.h"
#define MPL_DCT64 ASM_NAME(dct64_3dnowext)
#define SYNTH_NAME ASM_NAME(synth_1to1_3dnowext_asm)
#include "libmpg123/synth_sse3d.h"

NONEXEC_STACK
