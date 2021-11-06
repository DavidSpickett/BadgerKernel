/*
 * C library functions added as needed.
 * (we don't link with gcc provided libc)
 *
 * One reason to do this is that the toolchain
 * provided implementations contain
 * some instructions that require specific alignments.
 *
 * Since we don't set up MMU, it is possible
 * to trigger data abort by those instruction.
 * (like DC ZVA, etc.)
 * Not an issue on QEMU but happens on real hardware.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
// For EOF
#include <stdio.h>

int isdigit(int c) {
  if (c == EOF) {
    return 0;
  }

  char chr = c;
  return chr >= '0' && chr <= '9';
}

int abs(int x) {
  return x > 0 ? x : x * -1;
}

size_t strlen(const char* s) {
  size_t len = 0;
  while (*s != '\0') {
    len++;
    s++;
  }
  return len;
}

int strcmp(const char* s1, const char* s2) {
  while (*s1 == *s2 && *s1 != '\0' && *s2 != '\0') {
    s1++;
    s2++;
  }

  return *s1 - *s2;
}

int strncmp(const char* s1, const char* s2, size_t len) {
  while (len > 0) {
    if (*s1 != *s2) {
      return *s1 - *s2;
    }

    if (*s1 == '\0') {
      break;
    }

    s1++;
    s2++;
    len--;
  }

  return 0;
}

char* strncpy(char* dst, const char* src, size_t num) {
  size_t i = 0;
  while (i < num && src[i] != '\0') {
    dst[i] = src[i];
    i++;
  }

  if (i < num)
    dst[i] = '\0';
  return dst;
}

void bzero(void* dst, size_t len) {
  memset(dst, 0, len);
}

// TODO: LTO decides memset/memcpy are unused then presumably the compiler
// generates a call to it and realises it's gone.
__attribute__((used)) void* memset(void* dst, int ch, size_t len) {
  uint8_t* d = dst;
  while (len-- > 0) {
    *d++ = ch;
  }
  return dst;
}

__attribute__((used)) void* memcpy(void* dst, const void* src, size_t len) {
  uint8_t* d = dst;
  const uint8_t* s = src;
  while (len-- > 0) {
    *d++ = *s++;
  }
  return dst;
}

void* memmove(void* dst, const void* src, size_t len) {
  if (len == 0 || dst == src) {
    // len == 0 or copy to the same place, return immediately
    return dst;
  } else if (src < dst) {
    // copy backward
    uint8_t* d = dst + len;
    const uint8_t* s = src + len;
    while (len-- > 0) {
      *--d = *--s;
    }
  } else if (src > dst) {
    // copy forward
    uint8_t* d = dst;
    const uint8_t* s = src;
    while (len-- > 0) {
      *d++ = *s++;
    }
  }
  return dst;
}
