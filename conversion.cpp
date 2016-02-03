#include<iostream>
#include "mpi.h"
#include<stdlib.h>
#include<stdio.h>
#include <string.h>

int main (int argc, char **argv){

	double * data;
	FILE *fp;
	int buf[2];

	MPI_File fh;
	MPI_Init(&argc, &argv);
	fp = fopen (argv[2], "w");
	MPI_File_open(MPI_COMM_WORLD, argv[2], MPI_MODE_RDWR, MPI_INFO_NULL, &fh);

	if ((fp = fopen (argv[1], "r")) != NULL){
		fscanf(fp, "%d %d", buf, buf+1);
		data = new double [(int)(buf[0] * buf[1])];
		for (int i=0; i < (int)(buf[0] * buf[1]); i++)
			fscanf(fp, "%lf ", data+i);
	} else {
		fprintf(stderr, "error opening file for matrix A (%s)\n", argv[1]);
		exit (EXIT_FAILURE);
	}


	MPI_File_write(fh, buf,  2,                      MPI_INT,    MPI_STATUS_IGNORE);
	MPI_File_write(fh, data, (int)(buf[0] * buf[1]), MPI_DOUBLE, MPI_STATUS_IGNORE);
	

	MPI_File_read(fh, buf, 2, MPI_INT, MPI_STATUS_IGNORE);
//	printf ("%d\n%d\n%lf\n%lf\n" ,buf[0], buf[1],buf[2],buf[3]);
	printf ("%d\n%d\n%lf\n%lf\n" ,buf[0], buf[1],data[0], data[1]);


	MPI_File_close(&fh);
	MPI_Finalize();

return 0;

}

