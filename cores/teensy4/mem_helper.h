#pragma once
#include "tlsf_alloc.h"

#ifdef __cplusplus
extern "C"
{
#endif

  size_t get_heap_free();
  size_t get_heap_min_free();
  size_t get_heap_max_alloc();

  size_t get_stack_free();

#ifdef __cplusplus
}
#endif
