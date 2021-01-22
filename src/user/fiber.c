#include "user/fiber.h"
#include "port/port.h"
#include <string.h>

static void init_context(FiberContext* ctx) {
  memset(ctx, 0, sizeof(FiberContext));
}

void make_context(FiberContext* ctx, void (*function)(FiberContext*),
                  uint8_t* stack_ptr) {
  init_context(ctx);

  // Align to 16 bytes for AArch64
  ctx->sp = ALIGN_STACK_PTR((size_t)stack_ptr);
  ctx->pc = (size_t)function;

  // In case the function returns normally, we'll catch it
  ctx->lr = (size_t)set_context_from_stack_address;
  // Putting the address of the return context on the new fiber's stack
  ctx->sp -= sizeof(FiberContext*);
  // Note that we align, then write to it
  ctx->sp = ALIGN_STACK_PTR(ctx->sp);

  FiberContext** ctx_addr_on_stack = (FiberContext**)ctx->sp;
  *ctx_addr_on_stack = ctx;
}
