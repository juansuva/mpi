/* C Example */
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


int main (argc, argv)
     int argc;
     char *argv[];
{
  int rank, size;
  MPI_Status status;
  
  int x,y;

	MPI_Init (&argc, &argv);	/* starts MPI */
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);	/* get current process id (hilos=0 proceso maestro,)*/
	MPI_Comm_size (MPI_COMM_WORLD, &size);	/* get number of processes */
	
	//send(
	if(rank == 0)
	{
		x = 5;
		MPI_Send(&x, 1, MPI_INT, 1, 100, MPI_COMM_WORLD);
	}
	
	else
	{
			MPI_Recv(&x, 1, MPI_INT, 0, 100, MPI_COMM_WORLD, &status);
			x = x * 1000;
			//printf("%d",x);
			MPI_Send(&x, 1, MPI_INT, 0, 200, MPI_COMM_WORLD);

	}
	
	if(rank==0)
	{
		MPI_Recv(&x, 1, MPI_INT, 1, 200, MPI_COMM_WORLD, &status);
		
		printf("valor de la variable x: %d", x);
	}
	
	
	MPI_Finalize();
	return 0;
}
