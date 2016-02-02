#include<iostream>
#include "mpi.h"
#include<stdlib.h>
#include<stdio.h>
#include <string.h>

int main (int argc, char **argv){

	double * data;
	FILE *fp;
	int matrices_a_b_dimensions[2];

	MPI_File fh;
	MPI_Init(&argc, &argv);
	fp = fopen (argv[2], "w");
	MPI_File_open(MPI_COMM_WORLD, argv[2], MPI_MODE_RDWR, MPI_INFO_NULL, &fh);

	if ((fp = fopen (argv[1], "r")) != NULL){
		fscanf(fp, "%d %d\n", matrices_a_b_dimensions, matrices_a_b_dimensions+1);
		data = new double [matrices_a_b_dimensions[0] * matrices_a_b_dimensions[1]];
		for (int i=0; i < matrices_a_b_dimensions[0] * matrices_a_b_dimensions[1]; i++)
			fscanf(fp, "%lf", data+i);
	} else {
		fprintf(stderr, "error opening file for matrix A (%s)\n", argv[1]);
		exit (EXIT_FAILURE);
	}

	MPI_File_write(fh, matrices_a_b_dimensions, 2, MPI_INT, MPI_STATUS_IGNORE);
	MPI_File_write(fh, data, matrices_a_b_dimensions[0] * matrices_a_b_dimensions[1], MPI_DOUBLE, MPI_STATUS_IGNORE);
	MPI_File_close(&fh);

	MPI_Finalize();

return 0;

}

