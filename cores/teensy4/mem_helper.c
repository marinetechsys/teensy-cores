#include "mem_helper.h"
#include "arm_math.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

size_t get_heap_free()
{
    tlsf_walk_stats stats;
    ocram_heap_stats(&stats);
    return stats.total_free;
}

size_t get_heap_min_free()
{
    return ocram_min_ever_free();
}

size_t get_heap_max_alloc()
{
    tlsf_walk_stats stats;
    ocram_heap_stats(&stats);
    return stats.largest_block;
}

size_t get_stack_free()
{
    extern char _stack_end[];
    uint32_t sp;
    __asm volatile ("MRS %0, msp" : "=r" (sp));
    return (size_t)(sp - (uintptr_t)_stack_end);
}

#ifdef __cplusplus
}
#endif
