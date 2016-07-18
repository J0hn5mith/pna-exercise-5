#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>
#include <math.h>
#include "matrix.h"
#include "utils.h"

#define DEBUG 0

/*static MATRIX_SIZES[7] = [200, 400, 600, 800, 1000, 1200, 1400]*/
const static int MATRIX_SIZES[3] = {200, 400, 600};
const static int THREAD_SIZES[8] = {2, 4, 6, 8, 10, 12, 14, 16};


struct Timer timer_global = {0};
struct Timer timer_computation = {0};
struct Timer timer_communication = {0};

struct Node{
	int rank;
	int world_size;
};

struct Measurement{
	float global;
	float computation;
	float communication;
};

struct Node init_mpi(){
	MPI_Init(NULL, NULL);
	int world_size;
	int rank;

	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	struct Node node;
	node.rank = rank;
	node.world_size = world_size;

	return node;
}

void broadcast_matrix(float* matrix, int dimension){
	MPI_Bcast(
			(void*) matrix,
			dimension*dimension,
			MPI_FLOAT,
			0,
			MPI_COMM_WORLD
			);
}

float* init_matrix(struct Node node, int dimension){
	float* matrix = generate_matrix(dimension);
	broadcast_matrix(matrix, dimension);
	return matrix;
}

void process_row(int step, int row, float* l, float* u, int dimension){
	float l_value  = get(u, row, step, dimension)/get(u, step, step, dimension);
	set(l, row, step, l_value, dimension);
	for (int column = step; column < dimension; column++) {
		float u_value = get(u, row, column, dimension) - l_value\
						* get(u, step, column, dimension);
		set(u,row, column, u_value, dimension);
	}
}

void broadcast_rows(float* matrix, int sender, int start_row, int num_rows, int dimension){
	matrix += start_row * dimension;
	MPI_Bcast(
			matrix,
			num_rows * dimension,
			MPI_FLOAT,
			sender,
			MPI_COMM_WORLD
			);
}

void init(struct Node node, float** m, float** l, float** u, int dimension){
	*m = init_matrix(node, dimension);
	*u = generate_unit_matrix(dimension);
	copy_matrix(*m, *u, dimension);
	*l = generate_unit_matrix(dimension);
}

struct Measurement lu_decomposition(struct Node node, int dimension, int threads){
	float*  matrix, *u, *l = NULL;
	init(node, &matrix, &l, &u, dimension);

	reset_timer(&timer_global);
	reset_timer(&timer_computation);
	reset_timer(&timer_communication);

	omp_set_num_threads(threads);
	MPI_Barrier(MPI_COMM_WORLD);
	start_timer(&timer_global);
	for (int step = 0; step < dimension; ++step) {
		int remaining_rows = dimension - step - 1;

		// Calculate Own values
		start_timer(&timer_computation);
		if(node.rank < remaining_rows ){
			int block_size = floor((float)remaining_rows/(float)node.world_size);
			int block_start = remaining_rows - block_size * (node.world_size - node.rank);
			if(block_size == 0 || node.rank < remaining_rows%block_size){
				block_size++;
				block_start = step + 1 + block_size * node.rank;
			}
			int end = block_start +  block_size;
#pragma omp parallel for
			for (int row = block_start; row < end; ++row) {
				process_row(step, row, l, u, dimension);
			}
#pragma omp barrier
		}
		stop_timer(&timer_computation);

		start_timer(&timer_communication);
		for (int worker = 0; worker < node.world_size; ++worker) {
			if(worker < remaining_rows ){
				int block_size = floor((float)remaining_rows/(float)node.world_size);
				int block_start = remaining_rows - block_size * (node.world_size - worker);
				if(block_size == 0 || worker < remaining_rows%block_size){
					block_size++;
					block_start = step + 1 + block_size * worker;
				}
				broadcast_rows(u, worker, block_start, block_size, dimension);
				broadcast_rows(l, worker, block_start, block_size, dimension);
			}
		}
		stop_timer(&timer_communication);
	}

	MPI_Barrier(MPI_COMM_WORLD);
	stop_timer(&timer_global);

	float* check = mul_matrix(l, u, dimension);

	if(node.rank == 0){
		if(DEBUG){
			if(!compare_matrix(matrix, check, dimension)){
				printf("Error!!\n");
			} else {
				printf("Successfully finished job\n");
			}
		}
	}

	free(matrix);
	free(l);
	free(u);
	struct Measurement  measurment = {
		timer_global.duration, 
		timer_computation.duration, 
		timer_communication.duration
	};
	return measurment;
}



int main(int argc, char *argv[]) {
	struct Node node = init_mpi();
	if(node.rank == 0){
		printf("time_total, time_computation, time_communication,  dimension, threads, ranks\n");
	}
	for (int i = 0; i < sizeof(MATRIX_SIZES)/sizeof(int); ++i) {
		for (int ii = 0; ii < sizeof(THREAD_SIZES)/sizeof(int); ++ii) {
			int dimension = MATRIX_SIZES[i];
			int threads = THREAD_SIZES[ii];
			struct Measurement m = lu_decomposition(node, dimension, threads);
			if(node.rank == 0){
				printf("%f, ", m.global);
				printf("%f, ", m.computation);
				printf("%f, ", m.communication);
				printf("%i, ", dimension);
				printf("%i, ", 1);
				printf("%i, ", node.world_size);
				printf("\n");
			}
		}
	}

	MPI_Finalize();
	return 0;
}
