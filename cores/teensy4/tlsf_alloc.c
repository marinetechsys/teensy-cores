#include "tlsf_alloc.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int printf_debug(const char *format, ...);

// ---- linker symbols ----

extern unsigned long _stack_reclaim_start;
extern unsigned long _stack_reclaim_end;

extern unsigned long _heap_start;
extern unsigned long _heap_end;

// ---- allocator structs ----

tlsf_region_t dtcm_region = {0};
tlsf_region_t ocram_region = {0};

// ---- internal stats ----
bool _tlsf_init = false;

static void walk_cb(void *ptr, size_t size, int used, void *user) {
  tlsf_walk_stats *stats = (tlsf_walk_stats *)user;
  if (!used) {
    stats->total_free += size;
    stats->free_blocks++;
    if (size > stats->largest_block)
      stats->largest_block = size;
  }
}

void update_stats(tlsf_region_t *reg, tlsf_walk_stats *out) {
  if (!reg->tlsf)
    return;
  *out = (tlsf_walk_stats){0};
  tlsf_walk_pool(tlsf_get_pool(reg->tlsf), walk_cb, out);
  if (out->total_free < reg->min_ever_free)
    reg->min_ever_free = out->total_free;
}

// ---- init ----

bool tlsf_heap_init(void) {
  memset(&ocram_region, 0, sizeof(ocram_region));
  memset(&dtcm_region, 0, sizeof(dtcm_region));

  ocram_region.pool_start = (void *)&_heap_start;
  ocram_region.pool_size = (uintptr_t)&_heap_end - (uintptr_t)&_heap_start;
  ocram_region.min_ever_free = SIZE_MAX;
  ocram_region.tlsf =
      tlsf_create_with_pool(ocram_region.pool_start, ocram_region.pool_size);

  return (ocram_region.tlsf != NULL);
}

tlsf_region_t *get_ocram_region(void) { return &ocram_region; }

tlsf_region_t *get_dtcm_region(void) { return &dtcm_region; }

// ---- DTCM API ----
void reclaim_boot_stack_for_tlsf() {
  dtcm_region.pool_start = (void *)&_stack_reclaim_start;
  dtcm_region.pool_size =
      (uintptr_t)&_stack_reclaim_end - (uintptr_t)&_stack_reclaim_start;
  dtcm_region.min_ever_free = SIZE_MAX;
  dtcm_region.tlsf =
      tlsf_create_with_pool(dtcm_region.pool_start, dtcm_region.pool_size);
}

void *dtcm_malloc(size_t sz) {
  void *ptr = tlsf_malloc(dtcm_region.tlsf, sz);
  if (!ptr)
    dtcm_region.alloc_failures++;
  return ptr;
}

void dtcm_free(void *ptr) {
  if (ptr)
    tlsf_free(dtcm_region.tlsf, ptr);
}

void *dtcm_realloc(void *ptr, size_t sz) {
  void *newp = tlsf_realloc(dtcm_region.tlsf, ptr, sz);
  if (!newp)
    dtcm_region.alloc_failures++;
  return newp;
}

void dtcm_heap_stats(tlsf_walk_stats *out) { update_stats(&dtcm_region, out); }

size_t dtcm_min_ever_free(void) { return dtcm_region.min_ever_free; }

size_t dtcm_alloc_failures(void) { return dtcm_region.alloc_failures; }

float dtcm_fragmentation(void) {
  tlsf_walk_stats s;
  dtcm_heap_stats(&s);
  return s.total_free == 0 ? 0 : 1.0f - ((float)s.largest_block / s.total_free);
}

// ---- OCRAM API ----

void *ocram_malloc(size_t sz) {
  void *ptr = tlsf_malloc(ocram_region.tlsf, sz);
  if (!ptr)
    ocram_region.alloc_failures++;
  return ptr;
}

void ocram_free(void *ptr) {
  if (ptr)
    tlsf_free(ocram_region.tlsf, ptr);
}

void *ocram_realloc(void *ptr, size_t sz) {
  void *newp = tlsf_realloc(ocram_region.tlsf, ptr, sz);
  if (!newp)
    ocram_region.alloc_failures++;
  return newp;
}

void ocram_heap_stats(tlsf_walk_stats *out) {
  update_stats(&ocram_region, out);
}

size_t ocram_min_ever_free(void) { return ocram_region.min_ever_free; }

size_t ocram_alloc_failures(void) { return ocram_region.alloc_failures; }

float ocram_fragmentation(void) {
  tlsf_walk_stats s;
  ocram_heap_stats(&s);
  return s.total_free == 0 ? 0 : 1.0f - ((float)s.largest_block / s.total_free);
}

// ---- Debug Print ----

static void print_stats(const char *name, tlsf_region_t *reg) {
  tlsf_walk_stats s;
  update_stats(reg, &s);

  printf_debug("[%s HEAP]\n", name);
  printf_debug("  Pool size     : %u bytes\n", (unsigned)reg->pool_size);
  printf_debug("  Total free    : %u bytes in %u blocks\n", (unsigned)s.total_free,
         (unsigned)s.free_blocks);
  printf_debug("  Largest block : %u bytes\n", (unsigned)s.largest_block);
  printf_debug("  Fragmentation : %.2f%%\n",
         (1.0f - ((float)s.largest_block / s.total_free)) * 100.0f);
  printf_debug("  Min ever free : %u bytes\n", (unsigned)reg->min_ever_free);
  printf_debug("  Failed allocs : %u\n\n", (unsigned)reg->alloc_failures);
}

void tlsf_heap_debug(void) {
  print_stats("DTCM", &dtcm_region);
  print_stats("OCRAM", &ocram_region);
}

// ---- Wrappers ----
void *__wrap_malloc(size_t size)
{
  if (!_tlsf_init)
  {
    _tlsf_init = true;
    if (!tlsf_heap_init())
    {
      printf_debug("TLSF init failed\n");
      return NULL;
    }
  }
  return ocram_malloc(size);
}

void __wrap_free(void *ptr) { ocram_free(ptr); }

void *__wrap_realloc(void *ptr, size_t s) { return ocram_realloc(ptr, s); }

void *__wrap_calloc(size_t n, size_t s) {
  void *p = ocram_malloc(n * s);
  if (p)
    memset(p, 0, n * s);
  return p;
}

#ifdef __cplusplus
}
#endif
