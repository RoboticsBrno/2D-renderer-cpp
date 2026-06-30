#ifndef PROFILING_H
#define PROFILING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifdef ESP_PLATFORM
#include "esp_timer.h"
#define RENDERER_GET_TIME_US() RENDERER_GET_TIME_US()
#else
#include <time.h>
static inline uint64_t _renderer_get_time_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000;
}
#define RENDERER_GET_TIME_US() _renderer_get_time_us()
#endif

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
        uint64_t _start = RENDERER_GET_TIME_US();                                \
        typeof(func_call) _result = func_call;                                 \
        uint64_t _end = RENDERER_GET_TIME_US();                                  \
        profile_add_data(#func_call, _end - _start);                           \
        _result;                                                               \
    })

#define PROFILE_FUNC(func_call)                                                \
    do {                                                                       \
        uint64_t _start = RENDERER_GET_TIME_US();                                \
        func_call;                                                             \
        uint64_t _end = RENDERER_GET_TIME_US();                                  \
        profile_add_data(#func_call, _end - _start);                           \
    } while (0)

#define PROFILE_START() uint64_t _profile_start = RENDERER_GET_TIME_US()

#define PROFILE_END(name)                                                      \
    do {                                                                       \
        uint64_t _profile_end = RENDERER_GET_TIME_US();                          \
        profile_add_data(name, _profile_end - _profile_start);                 \
    } while (0)

#ifdef __cplusplus
}
#endif
#endif // PROFILING_H
