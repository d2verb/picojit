#include <stdio.h>
#include "picojit.h"

int main(void)
{
    CodeSeq *cs = newCodeSeq();

    emitU8s(cs, 7, 0x48, 0xc7, 0xc0, 0x2a, 0x00, 0x00, 0x00);
    emitU8(cs, 0xc3);

    typedef int (*JIT42)(void);
    JIT42 fn = genJITMem(cs);
    printf("%d\n", fn());
}
