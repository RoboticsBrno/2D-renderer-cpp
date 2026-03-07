#include "Profiler.hpp"
#include <stdio.h>
#include <string.h>

function_profile_t profiles[MAX_PROFILED_FUNCTIONS] = {0};
int profile_count = 0;

void profile_add_data(const char *func_name, uint64_t duration_us) {
    int i;
    for (i = 0; i < profile_count; i++) {
        if (strcmp(profiles[i].function_name, func_name) == 0) {
            break;
        }
    }

    if (i == profile_count && profile_count < MAX_PROFILED_FUNCTIONS) {
        profiles[i].function_name = func_name;
        profiles[i].call_count = 0;
        profiles[i].total_time_us = 0;
        profiles[i].max_time_us = 0;
        profiles[i].min_time_us = UINT64_MAX;
        profile_count++;
    }

    if (i < MAX_PROFILED_FUNCTIONS) {
        profiles[i].total_time_us += duration_us;
        profiles[i].call_count++;
        if (duration_us > profiles[i].max_time_us) {
            profiles[i].max_time_us = duration_us;
        }
        if (duration_us < profiles[i].min_time_us) {
            profiles[i].min_time_us = duration_us;
        }
    }
}

void profile_print_results(void) {
    printf("\n=== FUNCTION PROFILING RESULTS ===\n");

    int max_name_length = 0;
    for (int i = 0; i < profile_count; i++) {
        int name_length = strlen(profiles[i].function_name);
        if (name_length > max_name_length) {
            max_name_length = name_length;
        }
    }

    printf("%-*s %8s %12s %10s %10s %10s\n", max_name_length, "Function",
           "Calls", "Total(us)", "Avg(us)", "Max(us)", "Min(us)");

    for (int i = 0; i < max_name_length + 60; i++)
        printf("-");
    printf("\n");

    for (int i = 0; i < profile_count; i++) {
        uint64_t avg_time =
            profiles[i].call_count > 0
                ? profiles[i].total_time_us / profiles[i].call_count
                : 0;

        printf("%-*s %8lu %12llu %10llu %10llu %10llu\n", max_name_length,
               profiles[i].function_name, profiles[i].call_count,
               profiles[i].total_time_us, avg_time, profiles[i].max_time_us,
               profiles[i].min_time_us);
    }

    for (int i = 0; i < max_name_length + 60; i++)
        printf("=");
    printf("\n");
}
void profile_reset(void) {
    profile_count = 0;
    memset(profiles, 0, sizeof(profiles));
}
