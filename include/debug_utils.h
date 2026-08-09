#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(NDEBUG)

// Release code

#define PUSH_STACK_TRACE(func) ((void) 0)
#define POP_STACK_TRACE() ((void) 0)

#define EXIT(x) ((void) 0)
#define EXIT_HANDLE(x, handle) ((void) 0)
#define ASSERT(x) ((void) 0)
#define ASSERT_HANDLE(x, handle) ((void) 0)
#define ASSERT_MSG(x, msg) ((void) 0)
#define ASSERT_MSG_HANDLE(x, msg, handle) ((void) 0)

#else

// Debug code

#define DEBUG_MODE

#include <stdlib.h>
#include <stdio.h>

#include "types.h"

#define GLOBAL_DEBUG_STACK_TRACE_SIZE 1024
#define GLOBAL_DEBUG_STACK_TRACE_STRING_SIZE 1024

#define ASSERTION_FORMAT "Assertion failed!\nExpression      : %s\nFile            : %s\nLine            : %s\n"
#define ASSERTION_MSG_FORMAT "Assertion failed!\nMessage         : %s\nExpression      : %s\nFile            : %s\nLine            : %d\n"
#define STACK_TRACE_FORMAT "Method          : %s\nFile            : %s\nLine            : %d\n"

typedef void (*debug_handle)(void);
extern debug_handle global_debug_handle;

extern char global_debug_stack_trace[GLOBAL_DEBUG_STACK_TRACE_SIZE][GLOBAL_DEBUG_STACK_TRACE_STRING_SIZE];
extern u64 global_debug_stack_trace_index;

void add_to_global_debug_stack_trace(const char* str);
void remove_from_global_debug_stack_trace(void);
void global_debug_handle_func(void);

#define PUSH_STACK_TRACE(func) { char buf[1024]; snprintf(buf, GLOBAL_DEBUG_STACK_TRACE_STRING_SIZE, STACK_TRACE_FORMAT, func, __FILE__, __LINE__); add_to_global_debug_stack_trace(buf); }
#define POP_STACK_TRACE() { remove_from_global_debug_stack_trace(); }

#define EXIT(x) { global_debug_handle(); exit(1); }
#define EXIT_HANDLE(x, handle) { #handle(); global_debug_handle(); exit(1); }
#define ASSERT(x) if (!(x)) { fprintf(stderr, ASSERTION_FORMAT, #x, __FILE__, __LINE__); global_debug_handle(); exit(1); }
#define ASSERT_HANDLE(x, handle) if (!(x)) { fprintf(stderr, ASSERTION_FORMAT, #x, __FILE__, __LINE__); #handle(); global_debug_handle(); exit(1); }
#define ASSERT_MSG(x, msg) if (!(x)) { fprintf(stderr, ASSERTION_MSG_FORMAT, msg, #x, __FILE__, __LINE__); global_debug_handle(); exit(1); }
#define ASSERT_MSG_HANDLE(x, msg, handle) if (!(x)) { fprintf(stderr, ASSERTION_MSG_FORMAT, msg, #x, __FILE__, __LINE__); #handle(); global_debug_handle(); exit(1); }

#endif

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // DEBUG_UTILS_H
