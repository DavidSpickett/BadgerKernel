#include <stdint.h>
#include "print.h"
#include "thread.h"

typedef struct {
  uint32_t r0;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t r4;
  uint32_t r5;
  uint32_t r6;
  uint32_t r7;
  uint32_t r8;
  uint32_t r9;
  uint32_t r10;
  uint32_t r11;
  uint32_t r12;
  /* stack pointer in Thread struct */
  uint32_t lr;
  /* aka the exception mode lr */
  uint32_t pc;
  uint32_t cpsr;
} __attribute__((packed)) RegisterContext;

void print_register_context(const Thread* thread) {
  const RegisterContext* ctx =
    (const RegisterContext*)thread->stack_ptr;

  printf(
  "  r0: 0x%08x r1: 0x%08x  r2: 0x%08x  r3: 0x%08x\n"
  "  r4: 0x%08x r5: 0x%08x  r6: 0x%08x  r7: 0x%08x\n"
  "  r8: 0x%08x r9: 0x%08x r10: 0x%08x r11: 0x%08x\n"
  " r12: 0x%08x sp: 0x%08x  lr: 0x%08x  pc: 0x%08x\n"
  "cpsr: 0x%08x\n",
  ctx->r0, ctx->r1, ctx->r2,  ctx->r3,
  ctx->r4, ctx->r5, ctx->r6,  ctx->r7,
  ctx->r8, ctx->r9, ctx->r10, ctx->r11,
  ctx->r12, thread->stack_ptr, ctx->lr, ctx->pc,
  ctx->cpsr);
}
