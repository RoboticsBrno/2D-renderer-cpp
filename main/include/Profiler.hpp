#ifndef PROFILING_H
#define PROFILING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_timer.h"
#include <stdint.h>

#ifndef MAX_PROFILED_FUNCTIONS
#define MAX_PROFILED_FUNCTIONS 50
#endif

typedef struct {
    const char *function_name;
    uint64_t total_time_us;
    uint32_t call_count;
    uint64_t max_time_us;
    uint64_t min_time_us;
} function_profile_t;

extern function_profile_t profiles[MAX_PROFILED_FUNCTIONS];
extern int profile_count;

void profile_add_data(const char *func_name, uint64_t duration_us);

void profile_print_results(void);

void profile_reset(void);

void profile_print_results_sorted(void);

#define PROFILE_FUNC_RET(func_call)                                            \
    ({                                                                         \
        uint64_t _start = esp_timer_get_time();                                \
        typeof(func_call) _result = func_call;                                 \
        uint64_t _end = esp_timer_get_time();                                  \
        profile_add_data(#func_call, _end - _start);                           \
        _result;                                                               \
    })

#define PROFILE_FUNC(func_call)                                                \
    do {                                                                       \
        uint64_t _start = esp_timer_get_time();                                \
        func_call;                                                             \
        uint64_t _end = esp_timer_get_time();                                  \
        profile_add_data(#func_call, _end - _start);                           \
    } while (0)

#define PROFILE_START() uint64_t _profile_start = esp_timer_get_time()

#define PROFILE_END(name)                                                      \
    do {                                                                       \
        uint64_t _profile_end = esp_timer_get_time();                          \
        profile_add_data(name, _profile_end - _profile_start);                 \
    } while (0)

#ifdef __cplusplus
}
#endif
#endif // PROFILING_H
