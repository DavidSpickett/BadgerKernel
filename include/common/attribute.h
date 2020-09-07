#ifndef COMMON_ATTRIBUTE_H
#define COMMON_ATTRIBUTE_H

#ifdef __aarch64__
// No naked attribute for AArch64
#define ATTR_NAKED
#else
#define ATTR_NAKED __attribute__((naked))
#endif

#endif /* ifdef COMMON_ATTRIBUTE_H */
