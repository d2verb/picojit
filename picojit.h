#pragma once

#include <stdint.h>

typedef struct {
    uint8_t *code;
    size_t len;
    size_t cap;
} CodeSeq;

CodeSeq *newCodeSeq(void);
void delCodeSeq(CodeSeq *cs);

void emitU8(CodeSeq *cs, uint8_t v);
void emitU8s(CodeSeq *cs, int n, ...);
void emitU32(CodeSeq *cs, uint32_t v);
void emitU64(CodeSeq *cs, uint64_t v);

void replU8At(CodeSeq *cs, size_t off, uint8_t v);
void replU32At(CodeSeq *cs, size_t off, uint32_t v);

void execCode(CodeSeq *cs);

uint32_t calcRel32Off(size_t jfrom, size_t jto);
