#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <assert.h>
#include "picojit.h"

static void expandCodeArea(CodeSeq *cs)
{
    uint8_t *new_code = malloc(sizeof(uint8_t) * cs->cap * 2);

    if (!new_code) {
        perror("malloc: Can't reallocate code area.");
        exit(1);
    }

    uint32_t i = 0;
    memcpy(new_code, cs->code, sizeof(uint8_t) * cs->len);
    free(cs->code);
    cs->code = new_code;
    cs->cap = cs->cap * 2;
}

CodeSeq *newCodeSeq(void)
{
    CodeSeq *cs = malloc(sizeof(CodeSeq));

    if (!cs->code) {
        perror("malloc: Can't allocate CodeSeq area.");
        exit(1);
    }

    cs->code = malloc(sizeof(uint8_t) * 16);

    if (!cs->code) {
        perror("malloc: Can't allocate code area.");
        exit(1);
    }

    cs->len = 0;
    cs->cap = 16;
    return cs;
}

void delCodeSeq(CodeSeq *cs)
{
    if (cs && cs->code) free(cs->code);
    if (cs) free(cs);
}

void emitU8(CodeSeq *cs, uint8_t v)
{
    if (cs->len >= cs->cap)
        expandCodeArea(cs);
    cs->code[cs->len] = v;
    cs->len++;
}

void emitU8s(CodeSeq *cs, int n, ...)
{
    va_list ap;
    va_start(ap, n);
    for (int i = 0; i < n; i++)
        emitU8(cs, va_arg(ap, int));
    va_end(ap);
}

void emitU32(CodeSeq *cs, uint32_t v)
{
    emitU8(cs, v & 0xff);
    emitU8(cs, (v >> 8) & 0xff);
    emitU8(cs, (v >> 16) & 0xff);
    emitU8(cs, (v >> 24) & 0xff);
}

void emitU64(CodeSeq *cs, uint64_t v)
{
    emitU32(cs, v & 0xffffffff);
    emitU32(cs, (v >> 32) & 0xffffffff);
}

void replU8At(CodeSeq *cs, size_t off, uint8_t v)
{
    cs->code[off] = v;
}

void replU32At(CodeSeq *cs, size_t off, uint32_t v)
{
    replU8At(cs, off, v & 0xff);
    replU8At(cs, off + 1, (v >> 8) & 0xff);
    replU8At(cs, off + 2, (v >> 16) & 0xff);
    replU8At(cs, off + 3, (v >> 24) & 0xff);
}

typedef void (*JITFun)(void);

static JITFun newJITMem(CodeSeq *cs)
{
    void *jit_mem = mmap(NULL, sizeof(uint8_t) * cs->len, PROT_WRITE | PROT_EXEC,
            MAP_ANON | MAP_PRIVATE, -1, 0);
    memcpy(jit_mem, cs->code, sizeof(uint8_t) * cs->len);
    return (JITFun)(jit_mem);
}

static void delJITMem(JITFun fn, size_t len)
{
    munmap(fn, len);
}

void execCode(CodeSeq *cs)
{
    JITFun fn = newJITMem(cs);
    fn();
    delJITMem(fn, cs->len);
}

uint32_t calcRel32Off(size_t jfrom, size_t jto)
{
    if (jto >= jfrom) {
        size_t diff = jto - jfrom;
        assert(diff < (1ull << 31));
        return diff;
    } else {
        size_t diff = jfrom - jto;
        assert(diff - 1 < (1ull << 31));
        uint32_t diff_unsigned = (uint32_t)(diff);
        return ~diff_unsigned + 1;
    }
}
