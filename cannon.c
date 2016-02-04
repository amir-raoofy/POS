#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

int main (int argc, char **argv) {
	FILE *fp;
	MPI_File fh;
	MPI_Offset filesize;
	int bufsize;
	double **A = NULL, **B = NULL, **C = NULL, *A_array = NULL, *B_array = NULL, *C_array = NULL;
	double *A_local_block = NULL, *B_local_block = NULL, *C_local_block = NULL;
	int A_rows, A_columns, A_local_block_rows, A_local_block_columns, A_local_block_size;
	int B_rows, B_columns, B_local_block_rows, B_local_block_columns, B_local_block_size;
	int C_local_block_size;
	int rank, size, sqrt_size, matrices_a_b_dimensions[4], matrix_c_dimensions[2], matrices_a_b_dimensions_recv[4];
	int A_local_block_dimentions[2], B_local_block_dimentions[2], C_local_block_dimentions[2], starts[6]; 
	MPI_Comm cartesian_grid_communicator, row_communicator, column_communicator;
	MPI_Status status; 
	MPI_Request request1, request2;
	double compute_time = 0, mpi_time = 0 ,init_time = 0, global_init_time = 0, start;
	MPI_Datatype local_block_A , local_block_B, local_block_C;


	// used to manage the cartesian grid
	int dimensions[2], periods[2], coordinates[2], remain_dims[2];

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	/* For square mesh */
	sqrt_size = (int)sqrt((double) size);             
	if(sqrt_size * sqrt_size != size){
		if( rank == 0 ) perror("need to run mpiexec with a perfect square number of processes\n");
		MPI_Abort(MPI_COMM_WORLD, -1);
	}

	// create a 2D cartesian grid 
	dimensions[0] = dimensions[1] = sqrt_size;
	periods[0] = periods[1] = 1;    
	MPI_Cart_create(MPI_COMM_WORLD, 2, dimensions, periods, 1, &cartesian_grid_communicator);
	MPI_Cart_coords(cartesian_grid_communicator, rank, 2, coordinates);

	// create a row communicator
	remain_dims[0] = 0; 
	remain_dims[1] = 1; 
	MPI_Cart_sub(cartesian_grid_communicator, remain_dims, &row_communicator);

	// create a column communicator
	remain_dims[0] = 1;
	remain_dims[1] = 0;
	MPI_Cart_sub(cartesian_grid_communicator, remain_dims, &column_communicator);

	// getting matrices from files at rank 0 only
	// example: mpiexec -n 64 ./cannon matrix1 matrix2 [test]

	start = MPI_Wtime();
	


	MPI_File_open(MPI_COMM_WORLD, argv[1] , MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
	MPI_File_read(fh, matrices_a_b_dimensions, 2, MPI_INT, MPI_STATUS_IGNORE);
 	MPI_File_close(&fh);
	A_rows = matrices_a_b_dimensions[0];
	A_columns = matrices_a_b_dimensions[1];
	A_local_block_rows = A_rows / sqrt_size;
	A_local_block_columns = A_columns / sqrt_size;
	A_local_block_dimentions[0] = A_local_block_rows;
	A_local_block_dimentions[1] = A_local_block_columns;
	starts[0]=coordinates[0] * A_local_block_dimentions[0];
	starts[1]=coordinates[1] * A_local_block_dimentions[1];

	A_local_block_size = A_local_block_rows * A_local_block_columns;
	A_local_block = (double *) malloc (A_local_block_size * sizeof(double));
	MPI_Type_create_subarray(2, matrices_a_b_dimensions, A_local_block_dimentions, starts, MPI_ORDER_C, MPI_DOUBLE, &local_block_A);
	MPI_Type_commit(&local_block_A);
	MPI_File_open(cartesian_grid_communicator, argv[1] , MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
	MPI_File_set_view(fh, 2*sizeof(int), MPI_DOUBLE, local_block_A,"native", MPI_INFO_NULL);     //TODO what is native?
	MPI_File_read(fh, A_local_block, A_local_block_size, MPI_DOUBLE, &status);
 	MPI_File_close(&fh);
	
	MPI_File_open(MPI_COMM_WORLD,  argv[2] , MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
	MPI_File_read(fh, matrices_a_b_dimensions+2, 2, MPI_INT, MPI_STATUS_IGNORE);
	MPI_File_close(&fh);
	B_rows = matrices_a_b_dimensions[2];
	B_columns = matrices_a_b_dimensions[3];
	B_local_block_rows = B_rows / sqrt_size;
	B_local_block_columns = B_columns / sqrt_size;
	B_local_block_dimentions[0] = B_local_block_rows;
        B_local_block_dimentions[1] = B_local_block_columns;
	starts[2]=coordinates[0] * B_local_block_dimentions[0];
        starts[3]=coordinates[1] * B_local_block_dimentions[1];

	B_local_block_size = B_local_block_rows * B_local_block_columns;
	B_local_block = (double *) malloc (B_local_block_size * sizeof(double));
	MPI_Type_create_subarray(2, matrices_a_b_dimensions+2, B_local_block_dimentions, starts+2, MPI_ORDER_C, MPI_DOUBLE, &local_block_B);
        MPI_Type_commit(&local_block_B);
        MPI_File_open(cartesian_grid_communicator, argv[2] , MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
	MPI_File_set_view(fh, 2*sizeof(int), MPI_DOUBLE, local_block_B,"native", MPI_INFO_NULL);     //TODO what is native?
	MPI_File_read(fh, B_local_block, B_local_block_size, MPI_DOUBLE, &status);
 	MPI_File_close(&fh);

	start = MPI_Wtime();

	// send dimensions to all peers
	C_local_block = (double *) malloc (A_local_block_rows * B_local_block_columns * sizeof(double));
	// C needs to be initialized at 0 (accumulates partial dot-products)
	int i;
	for(i=0; i < A_local_block_rows * B_local_block_columns; i++){
		C_local_block[i] = 0;
	}

	// full arrays only needed at root
	if(rank == 0){
		C_array = (double *) malloc(sizeof(double) * A_rows * B_columns);
		// allocate output matrix C
		C = (double **) malloc(A_rows * sizeof(double *));
		for(i=0; i<A_rows ;i++){
			C[i] = (double *) malloc(B_columns * sizeof(double));
		}
	} 

// fix initial arrangements before the core algorithm starts
	if(coordinates[0] != 0){
		MPI_Sendrecv_replace(A_local_block, A_local_block_size, MPI_DOUBLE, 
				(coordinates[1] + sqrt_size - coordinates[0]) % sqrt_size, 0, 
				(coordinates[1] + coordinates[0]) % sqrt_size, 0, row_communicator, &status);
	}
	if(coordinates[1] != 0){
		MPI_Sendrecv_replace(B_local_block, B_local_block_size, MPI_DOUBLE, 
				(coordinates[0] + sqrt_size - coordinates[1]) % sqrt_size, 0, 
				(coordinates[0] + coordinates[1]) % sqrt_size, 0, column_communicator, &status);
	}
	init_time += MPI_Wtime() - start;
	MPI_Reduce(&init_time ,&global_init_time ,1 ,MPI_DOUBLE, MPI_MAX, 0, cartesian_grid_communicator);
	// cannon's algorithm
	int cannon_block_cycle;
	int C_index, A_row, A_column, B_column;
	for(cannon_block_cycle = 0; cannon_block_cycle < sqrt_size; cannon_block_cycle++){
		// compute partial result for this block cycle
		start = MPI_Wtime();
		for(C_index = 0, A_row = 0; A_row < A_local_block_rows; A_row++){
			for(B_column = 0; B_column < B_local_block_columns; B_column++, C_index++){
				for(A_column = 0; A_column < A_local_block_columns; A_column++){
					C_local_block[C_index] += A_local_block[A_row * A_local_block_columns + A_column] *
						B_local_block[A_column * B_local_block_columns + B_column];
				}
			}
		}
		compute_time += MPI_Wtime() - start;
		start = MPI_Wtime();
		// rotate blocks horizontally
		MPI_Sendrecv_replace(A_local_block, A_local_block_size, MPI_DOUBLE, 
				(coordinates[1] + sqrt_size - 1) % sqrt_size, 0, 
				(coordinates[1] + 1) % sqrt_size, 0, row_communicator, &status);
		// rotate blocks vertically
		MPI_Sendrecv_replace(B_local_block, B_local_block_size, MPI_DOUBLE, 
				(coordinates[0] + sqrt_size - 1) % sqrt_size, 0, 
				(coordinates[0] + 1) % sqrt_size, 0, column_communicator, &status);
		mpi_time += MPI_Wtime() - start;
	}

	// get C parts from other processes at rank 0
	matrix_c_dimensions[0]= matrices_a_b_dimensions[0];
	matrix_c_dimensions[1]= matrices_a_b_dimensions[3];
	C_local_block_size = A_local_block_rows * B_local_block_columns;
	C_local_block_dimentions[0] = A_local_block_rows;
        C_local_block_dimentions[1] = B_local_block_columns;
        starts[4]=coordinates[0] * C_local_block_dimentions[0];
        starts[5]=coordinates[1] * C_local_block_dimentions[1];


	MPI_Type_create_subarray(2, matrix_c_dimensions, C_local_block_dimentions, starts+4, MPI_ORDER_C, MPI_DOUBLE, &local_block_C);
        MPI_Type_commit(&local_block_C);
        MPI_File_open(cartesian_grid_communicator, argv[3] , MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
        MPI_File_set_view(fh, 0, MPI_DOUBLE, local_block_C,"native", MPI_INFO_NULL);     //TODO what is native?
        MPI_File_write(fh, C_local_block, C_local_block_size, MPI_DOUBLE, &status);
        MPI_File_close(&fh);


	if(rank == 0) {
		for(i = 0; i < A_local_block_rows * B_local_block_columns; i++){
			C_array[i] = C_local_block[i];
		}
		int i;
		for(i = 1; i < size; i++){
			MPI_Recv(C_array + (i * A_local_block_rows * B_local_block_columns), A_local_block_rows * B_local_block_columns, 
				MPI_DOUBLE, i, 0, cartesian_grid_communicator, &status);
		}
	} else {
		MPI_Send(C_local_block, A_local_block_rows * B_local_block_columns, MPI_DOUBLE, 0, 0, cartesian_grid_communicator);
	}

	
	if(rank == 0) {
		for(i = 0; i < A_local_block_rows * B_local_block_columns; i++)
			C_array[i] = C_local_block[i];
	}

	double * buf = (double *) malloc (sizeof(double) * A_rows * B_columns);
	double **buff;
 	buff = (double **) malloc(A_rows * sizeof(double *));
        for(i=0; i<A_rows ;i++){
       	        buff[i] = (double *) malloc(B_columns * sizeof(double));
        }
	MPI_File fh2;
	MPI_File_open(MPI_COMM_WORLD, argv[3], MPI_MODE_RDONLY, MPI_INFO_NULL, &fh2);
	MPI_File_read(fh2, buf, A_rows * B_columns, MPI_DOUBLE, MPI_STATUS_IGNORE);
	MPI_File_close(&fh2);

	// generating output at rank 0
	if (rank == 0) {
		// convert the ID array into the actual C matrix 
		int i, j, k, row, column;
		for (i = 0; i < sqrt_size; i++){  // block row index
			for (j = 0; j < sqrt_size; j++){ // block column index
				for (row = 0; row < A_local_block_rows; row++){
					for (column = 0; column < B_local_block_columns; column++){
						C[i * A_local_block_rows + row] [j * B_local_block_columns + column] = 
							C_array[((i * sqrt_size + j) * A_local_block_rows * B_local_block_columns) 
							+ (row * B_local_block_columns) + column];
					}
				}
			}
		}


	for (i=0;i< A_rows; i++){
		for (j=0; j< B_columns; j++){
			buff[j][i]=buf[i+A_rows*j];
		}
	}
		for (i=0;i< A_rows; i++){
			for (j=0; j< B_columns; j++){
				C[i][j]=buff[i][j];
				}
			}

		printf("(%d,%d)x(%d,%d)=(%d,%d)\n", A_rows, A_columns, B_rows, B_columns, A_rows, B_columns);
		printf("Computation time:	%lf\n", compute_time);
		printf("MPI time:         	%lf\n", mpi_time);
		printf("Initialization time:    %lf\n", init_time);

		if (argc == 7){

			if ((fp = fopen (argv[4], "r")) != NULL){
				fscanf(fp, "%d %d\n", &matrices_a_b_dimensions[0], &matrices_a_b_dimensions[1]);
				A = (double **) malloc (matrices_a_b_dimensions[0] * sizeof(double *));
				for (row = 0; row < matrices_a_b_dimensions[0]; row++){
					A[row] = (double *) malloc(matrices_a_b_dimensions[1] * sizeof(double));
					for (column = 0; column < matrices_a_b_dimensions[1]; column++)
						fscanf(fp, "%lf", &A[row][column]);
				}
				fclose(fp);
			} else {
				if(rank == 0) fprintf(stderr, "error opening file for matrix A (%s)\n", argv[4]);
				MPI_Abort(MPI_COMM_WORLD, -1);
			}
			if((fp = fopen (argv[5], "r")) != NULL){
				fscanf(fp, "%d %d\n", &matrices_a_b_dimensions[2], &matrices_a_b_dimensions[3]);
				B = (double **) malloc (matrices_a_b_dimensions[2] * sizeof(double *));
				for(row = 0; row < matrices_a_b_dimensions[2]; row++){
					B[row] = (double *) malloc(matrices_a_b_dimensions[3] * sizeof(double *));
					for(column = 0; column < matrices_a_b_dimensions[3]; column++)
						fscanf(fp, "%lf", &B[row][column]);
				}
				fclose(fp);
			} else {
				if(rank == 0) fprintf(stderr, "error opening file for matrix B (%s)\n", argv[5]);
				MPI_Abort(MPI_COMM_WORLD, -1);
			}

		// need to check that the multiplication is possible given dimensions 
		// matrices_a_b_dimensions[0] = row size of A
		// matrices_a_b_dimensions[1] = column size of A
		// matrices_a_b_dimensions[2] = row size of B
		// matrices_a_b_dimensions[3] = column size of B
			if(matrices_a_b_dimensions[1] != matrices_a_b_dimensions[2]){
				if(rank == 0) fprintf(stderr, "A's column size (%d) must match B's row size (%d)\n", 
						matrices_a_b_dimensions[1], matrices_a_b_dimensions[2]);
				MPI_Abort(MPI_COMM_WORLD, -1);
			}

			// this implementation is limited to cases where thematrices can be partitioned perfectly
			if( matrices_a_b_dimensions[0] % sqrt_size != 0 
					|| matrices_a_b_dimensions[1] % sqrt_size != 0 
					|| matrices_a_b_dimensions[2] % sqrt_size != 0 
					|| matrices_a_b_dimensions[3] % sqrt_size != 0 ){
				if(rank == 0) fprintf(stderr, "cannot distribute work evenly among processe\n"
						"all dimensions (A: r:%d c:%d; B: r:%d c:%d) need to be divisible by %d\n",
						matrices_a_b_dimensions[0],matrices_a_b_dimensions[1],
						matrices_a_b_dimensions[2],matrices_a_b_dimensions[3], sqrt_size );
				MPI_Abort(MPI_COMM_WORLD, -1);
			}


			// present results on the screen
			printf("\nA( %d x %d ):\n", A_rows, A_columns);
			for(row = 0; row < A_rows; row++) {
				for(column = 0; column < A_columns; column++)
					printf ("%7.3f ", A[row][column]);
				printf ("\n");
			}
			printf("\nB( %d x %d ):\n", B_rows, B_columns);
			for(row = 0; row < B_rows; row++){
				for(column = 0; column < B_columns; column++)
					printf("%7.3f ", B[row][column]);
				printf("\n");
			}
			printf("\nC( %d x %d ) = AxB:\n", A_rows, B_columns);
			for(row = 0; row < A_rows; row++){
				for(column = 0; column < B_columns; column++)
					printf("%7.3f ",C[row][column]);
				printf("\n");
			}


			printf("\nPerforming serial consistency check. Be patient...\n");
			fflush(stdout);
			int pass = 1;
			double temp;
			for(i=0; i<A_rows; i++){
				for(j=0; j<B_columns; j++){
					temp = 0;
					for(k=0; k<B_rows; k++){
						temp += A[i][k] * B[k][j];
					}
					printf("%7.3f ", temp);
					if(temp != C[i][j]){
						pass = 0;
					}
				}
				printf("\n");
			}
			if (pass) printf("Consistency check: PASS\n");
			else printf("Consistency check: FAIL\n");

			for(i = 0; i < A_rows; i++){
				free(A[i]);
			}
			for(i = 0; i < B_rows; i++){
				free(B[i]);
			}
			free(A);
			free(B);
		}	
	}

	// free all memory
	if(rank == 0){
		int i;
		for(i = 0; i < A_rows; i++){
			free(C[i]);
		}
		free(C);
		free(C_array);
	}
	free(A_local_block);
	free(B_local_block);
	free(C_local_block);

	// finalize MPI
	MPI_Finalize();
}
