#ifndef P0_FINGERPRINT_H
#define P0_FINGERPRINT_H

#include <stdint.h>
#include <stddef.h>

#define P0_FINGERPRINT_WORDS 8

static const uint16_t p0_fingerprint_offsets[P0_FINGERPRINT_WORDS] = {
  0x000, 0x200, 0x400, 0x600, 0x800, 0xa00, 0xc00, 0xe00,
};

struct p0_fingerprint {
  uintptr_t slide;
  uint64_t words[P0_FINGERPRINT_WORDS];
};

static const struct p0_fingerprint p0_fingerprints[] = {
  { 0x000000ULL, { 0x0000000000000000ULL, 0x0000000000000000ULL,
    0x0000000000000000ULL, 0x0000000000000000ULL,
    0x0000000000000000ULL, 0x0000000000000000ULL,
    0x0000000000000000ULL, 0x0000000000000000ULL } },
};

#endif
