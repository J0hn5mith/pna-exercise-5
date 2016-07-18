#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "matrix.h"

void process_remaining_rows(int step, float* l, float* u, int dimension){
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

int main(void) {
	int dimension = 4;
	float* m = generate_matrix(dimension);
	float* u = generate_unit_matrix(dimension);
	float* l = generate_unit_matrix(dimension);
	copy_matrix(m, u, dimension);

	omp_set_num_threads(8);

	for (int step = 0; step < dimension; ++step) {
		process_remaining_rows(step, l, u, dimension);

	}

    float* check = mul_matrix(l, u, dimension);
	if(!compare_matrix(m, check, dimension)){
		printf("Error!!\n");
		return 1;
	}
	printf("Success\n");
	print_matrix(u, dimension);
	print_matrix(l, dimension);
	free(m);
	free(l);
	free(u);

	return 0;
}
