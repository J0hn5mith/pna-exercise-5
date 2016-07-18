#include <omp.h>
#include <stdio.h>

#include "utils.h"


void start_timer(struct Timer* timer){
    timer->start_time = omp_get_wtime();
}

void stop_timer(struct Timer* timer){
    timer->end_time = omp_get_wtime();
    timer->duration += timer->end_time - timer->start_time;
}

void report_timer(struct Timer* timer){
        printf("%f\n", timer->duration);
}
