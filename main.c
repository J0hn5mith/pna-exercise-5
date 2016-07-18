#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "utils.h"
#include "matrix.h"

#define DEBUG 0


const int DIMENSIONS_LIST[5] = {100, 200, 400, 800, 1600};
const int THREADS_LIST[4] = {1, 2, 4, 8};

void process_remaining_rows(int step, float* l, float* u, int dimension){

#pragma omp parallel for
	for (int row = step + 1; row < dimension; ++row) {
		float l_value  = get(u, row, step, dimension)/get(u, step, step, dimension);
		set(l, row, step, l_value, dimension);
		for (int column = step; column < dimension; column++) {
			float u_value = get(u, row, column, dimension) - l_value\
							* get(u, step, column, dimension);
			set(u,row, column, u_value, dimension);
		}
	}
}

float lu_decomposition(int dimension, int threads){
	float* m = generate_matrix(dimension);
	float* u = generate_unit_matrix(dimension);
	float* l = generate_unit_matrix(dimension);
	copy_matrix(m, u, dimension);

	struct Timer timer = {0};

	omp_set_num_threads(threads);
	start_timer(&timer);
	for (int step = 0; step < dimension; ++step) {
		process_remaining_rows(step, l, u, dimension);
	}
#pragma omp barrier
	stop_timer(&timer);

	float* check = mul_matrix(l, u, dimension);
	if(!compare_matrix(m, check, dimension)){
		printf("Error!!\n");
		return 1;
	}
	if(DEBUG){
		printf("Success\n");
	}
	free(m);
	free(l);
	free(u);
	return timer.duration;
}

int main(void) {

	printf("time, threads, dimension\n");
	for (int i = 0; i < sizeof(DIMENSIONS_LIST)/sizeof(int); ++i) {
		for (int ii = 0; ii < sizeof(THREADS_LIST)/sizeof(int); ++ii) {
			int dimension = DIMENSIONS_LIST[i];
			int threads = THREADS_LIST[ii];
			printf("%f, %i, %i\n", lu_decomposition(dimension, threads), threads, dimension);
		}
	}
	return 0;
}
