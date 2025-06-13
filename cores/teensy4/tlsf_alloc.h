#pragma once
#include <stddef.h>
#include <stdbool.h>
#include "tlsf.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
  tlsf_t tlsf;
  void *pool_start;
  size_t pool_size;
  size_t min_ever_free;
  size_t alloc_failures;
} tlsf_region_t;

typedef struct
{
  size_t total_free;
  size_t largest_block;
  size_t free_blocks;
} tlsf_walk_stats;

// Global init
bool tlsf_heap_init(void);
void tlsf_heap_debug(void);

tlsf_region_t* get_ocram_region(void);
tlsf_region_t* get_dtcm_region(void);

// DTCM
void *dtcm_malloc(size_t size);
void dtcm_free(void *ptr);
void *dtcm_realloc(void *ptr, size_t size);

void dtcm_heap_stats(tlsf_walk_stats *out);
size_t dtcm_min_ever_free(void);
size_t dtcm_alloc_failures(void);
float dtcm_fragmentation(void);

// OCRAM
void *ocram_malloc(size_t size);
void ocram_free(void *ptr);
void *ocram_realloc(void *ptr, size_t size);

void ocram_heap_stats(tlsf_walk_stats *out);
size_t ocram_min_ever_free(void);
size_t ocram_alloc_failures(void);
float ocram_fragmentation(void);

void update_stats(tlsf_region_t *reg, tlsf_walk_stats *out);

#ifdef __cplusplus
}
#endif
