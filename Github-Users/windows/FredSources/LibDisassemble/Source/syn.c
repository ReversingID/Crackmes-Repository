/* udis86 - libudis86/syn.c
 *
 * Copyright (c) 2002-2013 Vivek Thampi
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright notice, 
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, 
 *       this list of conditions and the following disclaimer in the documentation 
 *       and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "types.h"
#include "decode.h"
#include "syn.h"
#include "udint.h"

/* 
 * Register Table - Order Matters (types.h)!
 *
 */
#ifndef REMOVE_REG_NAME
	#define REMOVE_REG_NAME(string) (string)
#endif
const char* ud_reg_tab[] = 
{
  REMOVE_REG_NAME("al"),   REMOVE_REG_NAME("cl"),   REMOVE_REG_NAME("dl"),   REMOVE_REG_NAME("bl"),
  REMOVE_REG_NAME("ah"),   REMOVE_REG_NAME("ch"),   REMOVE_REG_NAME("dh"),   REMOVE_REG_NAME("bh"),
  REMOVE_REG_NAME("spl"),  REMOVE_REG_NAME("bpl"),  REMOVE_REG_NAME("sil"),  REMOVE_REG_NAME("dil"),
  REMOVE_REG_NAME("r8b"),  REMOVE_REG_NAME("r9b"),  REMOVE_REG_NAME("r10b"), REMOVE_REG_NAME("r11b"),
  REMOVE_REG_NAME("r12b"), REMOVE_REG_NAME("r13b"), REMOVE_REG_NAME("r14b"), REMOVE_REG_NAME("r15b"),
 
  REMOVE_REG_NAME("ax"),   REMOVE_REG_NAME("cx"),   REMOVE_REG_NAME("dx"),   REMOVE_REG_NAME("bx"),
  REMOVE_REG_NAME("sp"),   REMOVE_REG_NAME("bp"),   REMOVE_REG_NAME("si"),   REMOVE_REG_NAME("di"),
  REMOVE_REG_NAME("r8w"),  REMOVE_REG_NAME("r9w"),  REMOVE_REG_NAME("r10w"), REMOVE_REG_NAME("r11w"),
  REMOVE_REG_NAME("r12w"), REMOVE_REG_NAME("r13w"), REMOVE_REG_NAME("r14w"), REMOVE_REG_NAME("r15w"),

  REMOVE_REG_NAME("eax"),  REMOVE_REG_NAME("ecx"),  REMOVE_REG_NAME("edx"),  REMOVE_REG_NAME("ebx"),
  REMOVE_REG_NAME("esp"),  REMOVE_REG_NAME("ebp"),  REMOVE_REG_NAME("esi"),  REMOVE_REG_NAME("edi"),
  REMOVE_REG_NAME("r8d"),  REMOVE_REG_NAME("r9d"),  REMOVE_REG_NAME("r10d"), REMOVE_REG_NAME("r11d"),
  REMOVE_REG_NAME("r12d"), REMOVE_REG_NAME("r13d"), REMOVE_REG_NAME("r14d"), REMOVE_REG_NAME("r15d"),

  REMOVE_REG_NAME("rax"),  REMOVE_REG_NAME("rcx"),  REMOVE_REG_NAME("rdx"),  REMOVE_REG_NAME("rbx"),
  REMOVE_REG_NAME("rsp"),  REMOVE_REG_NAME("rbp"),  REMOVE_REG_NAME("rsi"),  REMOVE_REG_NAME("rdi"),
  REMOVE_REG_NAME("r8"),   REMOVE_REG_NAME("r9"),   REMOVE_REG_NAME("r10"),  REMOVE_REG_NAME("r11"),
  REMOVE_REG_NAME("r12"),  REMOVE_REG_NAME("r13"),  REMOVE_REG_NAME("r14"),  REMOVE_REG_NAME("r15"),

  REMOVE_REG_NAME("es"),   REMOVE_REG_NAME("cs"),   REMOVE_REG_NAME("ss"),  REMOVE_REG_NAME("ds"),
  REMOVE_REG_NAME("fs"),   REMOVE_REG_NAME("gs"), 

  REMOVE_REG_NAME("cr0"),  REMOVE_REG_NAME("cr1"),  REMOVE_REG_NAME("cr2"),  REMOVE_REG_NAME("cr3"),
  REMOVE_REG_NAME("cr4"),  REMOVE_REG_NAME("cr5"),  REMOVE_REG_NAME("cr6"),  REMOVE_REG_NAME("cr7"),
  REMOVE_REG_NAME("cr8"),  REMOVE_REG_NAME("cr9"),  REMOVE_REG_NAME("cr10"), REMOVE_REG_NAME("cr11"),
  REMOVE_REG_NAME("cr12"), REMOVE_REG_NAME("cr13"), REMOVE_REG_NAME("cr14"), REMOVE_REG_NAME("cr15"),

  REMOVE_REG_NAME("dr0"),  REMOVE_REG_NAME("dr1"),  REMOVE_REG_NAME("dr2"),  REMOVE_REG_NAME("dr3"),
  REMOVE_REG_NAME("dr4"),  REMOVE_REG_NAME("dr5"),  REMOVE_REG_NAME("dr6"),  REMOVE_REG_NAME("dr7"),
  REMOVE_REG_NAME("dr8"),  REMOVE_REG_NAME("dr9"),  REMOVE_REG_NAME("dr10"), REMOVE_REG_NAME("dr11"),
  REMOVE_REG_NAME("dr12"), REMOVE_REG_NAME("dr13"), REMOVE_REG_NAME("dr14"), REMOVE_REG_NAME("dr15"),

  REMOVE_REG_NAME("mm0"),  REMOVE_REG_NAME("mm1"),  REMOVE_REG_NAME("mm2"),  REMOVE_REG_NAME("mm3"),
  REMOVE_REG_NAME("mm4"),  REMOVE_REG_NAME("mm5"),  REMOVE_REG_NAME("mm6"),  REMOVE_REG_NAME("mm7"),

  REMOVE_REG_NAME("st0"),  REMOVE_REG_NAME("st1"),  REMOVE_REG_NAME("st2"),  REMOVE_REG_NAME("st3"),
  REMOVE_REG_NAME("st4"),  REMOVE_REG_NAME("st5"),  REMOVE_REG_NAME("st6"),  REMOVE_REG_NAME("st7"), 

  REMOVE_REG_NAME("xmm0"), REMOVE_REG_NAME("xmm1"), REMOVE_REG_NAME("xmm2"), REMOVE_REG_NAME("xmm3"),
  REMOVE_REG_NAME("xmm4"), REMOVE_REG_NAME("xmm5"), REMOVE_REG_NAME("xmm6"), REMOVE_REG_NAME("xmm7"),
  REMOVE_REG_NAME("xmm8"), REMOVE_REG_NAME("xmm9"), REMOVE_REG_NAME("xmm10"), REMOVE_REG_NAME("xmm11"),
  REMOVE_REG_NAME("xmm12"), REMOVE_REG_NAME("xmm13"), REMOVE_REG_NAME("xmm14"), REMOVE_REG_NAME("xmm15"),

  REMOVE_REG_NAME("ymm0"),  REMOVE_REG_NAME("ymm1"),  REMOVE_REG_NAME("ymm2"),   REMOVE_REG_NAME("ymm3"),
  REMOVE_REG_NAME("ymm4"),  REMOVE_REG_NAME("ymm5"),  REMOVE_REG_NAME("ymm6"),   REMOVE_REG_NAME("ymm7"),
  REMOVE_REG_NAME("ymm8"),  REMOVE_REG_NAME("ymm9"),  REMOVE_REG_NAME("ymm10"),  REMOVE_REG_NAME("ymm11"),
  REMOVE_REG_NAME("ymm12"), REMOVE_REG_NAME("ymm13"), REMOVE_REG_NAME("ymm14"),  REMOVE_REG_NAME("ymm15"),

  REMOVE_REG_NAME("rip")
};


uint64_t
ud_syn_rel_target(struct ud *u, struct ud_operand *opr)
{
  const uint64_t trunc_mask = 0xffffffffffffffffull >> (64 - u->opr_mode);
  switch (opr->size) {
  case 8 : return (u->pc + opr->lval.sbyte)  & trunc_mask;
  case 16: return (u->pc + opr->lval.sword)  & trunc_mask;
  case 32: return (u->pc + opr->lval.sdword) & trunc_mask;
  default: UD_ASSERT(!"invalid relative offset size.");
    return 0ull;
  }
}


/*
 * asmprintf
 *    Printf style function for printing translated assembly
 *    output. Returns the number of characters written and
 *    moves the buffer pointer forward. On an overflow,
 *    returns a negative number and truncates the output.
 */
int
ud_asmprintf(struct ud *u, const char *fmt, ...)
{
  int ret;
  size_t avail;
  va_list ap;
  va_start(ap, fmt);
  avail = u->asm_buf_size - u->asm_buf_fill - 1 /* nullchar */;

#if (defined(WIN32) || defined(WIN64))
  #pragma warning( disable  : 4996 )
#endif
  ret = vsnprintf((char*) u->asm_buf + u->asm_buf_fill, avail, fmt, ap);
#if (defined(WIN32) || defined(WIN64))
  #pragma warning( default : 4996 )
#endif

  if (ret < 0 || ((unsigned int)ret) > avail) {
      u->asm_buf_fill = u->asm_buf_size - 1;
  } else {
      u->asm_buf_fill += ret;
  }
  va_end(ap);
  return ret;
}


void
ud_syn_print_addr(struct ud *u, uint64_t addr)
{
  const char *name = NULL;
  if (u->sym_resolver) {
    int64_t offset = 0;
    name = u->sym_resolver(u, addr, &offset);
    if (name) {
      if (offset) {
        ud_asmprintf(u, "%s%+" FMT64 "d", name, offset);
      } else {
        ud_asmprintf(u, "%s", name);
      }
      return;
    }
  }
  ud_asmprintf(u, "0x%" FMT64 "x", addr);
}


void
ud_syn_print_imm(struct ud* u, const struct ud_operand *op)
{
  uint64_t v;
  if (op->_oprcode == OP_sI && op->size != u->opr_mode) {
    if (op->size == 8) {
      v = (int64_t)op->lval.sbyte;
    } else {
      UD_ASSERT(op->size == 32);
      v = (int64_t)op->lval.sdword;
    }
    if (u->opr_mode < 64) {
      v = v & ((1ull << u->opr_mode) - 1ull);
    }
  } else {
    switch (op->size) {
    case 8 : v = op->lval.ubyte;  break;
    case 16: v = op->lval.uword;  break;
    case 32: v = op->lval.udword; break;
    case 64: v = op->lval.uqword; break;
    default: UD_ASSERT(!"invalid offset"); v = 0; /* keep cc happy */
    }
  }
  ud_asmprintf(u, "0x%" FMT64 "x", v);
}


void
ud_syn_print_mem_disp(struct ud* u, const struct ud_operand *op, int sign)
{
  UD_ASSERT(op->offset != 0);
 if (op->base == UD_NONE && op->index == UD_NONE) {
    uint64_t v;
    UD_ASSERT(op->scale == UD_NONE && op->offset != 8);
    /* unsigned mem-offset */
    switch (op->offset) {
    case 16: v = op->lval.uword;  break;
    case 32: v = op->lval.udword; break;
    case 64: v = op->lval.uqword; break;
    default: UD_ASSERT(!"invalid offset"); v = 0; /* keep cc happy */
    }
    ud_asmprintf(u, "0x%" FMT64 "x", v);
  } else {
    int64_t v;
    UD_ASSERT(op->offset != 64);
    switch (op->offset) {
    case 8 : v = op->lval.sbyte;  break;
    case 16: v = op->lval.sword;  break;
    case 32: v = op->lval.sdword; break;
    default: UD_ASSERT(!"invalid offset"); v = 0; /* keep cc happy */
    }
    if (v < 0) {
      ud_asmprintf(u, "-0x%" FMT64 "x", -v);
    } else if (v > 0) {
      ud_asmprintf(u, "%s0x%" FMT64 "x", sign? "+" : "", v);
    }
  }
}

/*
vim: set ts=2 sw=2 expandtab
*/
