#include "debug_utils.h"

#if !defined(NDEBUG)

void global_debug_handle_func(void) {}
debug_handle global_debug_handle = global_debug_handle_func;

#endif
