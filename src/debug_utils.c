#include "debug_utils.h"

#if !defined(NDEBUG)

#include <string.h>

char global_debug_stack_trace[GLOBAL_DEBUG_STACK_TRACE_SIZE][GLOBAL_DEBUG_STACK_TRACE_STRING_SIZE];
u64 global_debug_stack_trace_index = 0;

void add_to_global_debug_stack_trace(const char* str) {
    u64 len = strlen(str) + 1;
    if (len >= GLOBAL_DEBUG_STACK_TRACE_STRING_SIZE) {
        fprintf(stderr, "add_to_global_debug_stack_trace() failed: %llu string size too large\n", len);
        exit(1);
    }
    for (u64 i = 0; i < len - 1; i++) {
        global_debug_stack_trace[global_debug_stack_trace_index][i] = str[i];
    }
    global_debug_stack_trace[global_debug_stack_trace_index][len - 1] = 0;
    global_debug_stack_trace_index++;
}

void remove_from_global_debug_stack_trace(void) {
    if (global_debug_stack_trace_index > 0) global_debug_stack_trace_index--;
}

void global_debug_handle_func(void) {
    fprintf(stderr, "\n\nPrinting global_debug stack trace:\n\n");
    for (u64 i = 0; i < global_debug_stack_trace_index; i++) {
        fprintf(stderr, "Index %llu:\n%s\n", i, global_debug_stack_trace[i]);
    }
    fprintf(stderr, "\n");
}

debug_handle global_debug_handle = global_debug_handle_func;

#endif
