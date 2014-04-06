/* C Example */
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


int main(int argc, char **argv)
{
	int rank, size;
	MPI_Status status;

	MPI_Init (&argc, &argv);	/* starts MPI */
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);	/* get current process id (hilos=0 proceso maestro,)*/
	MPI_Comm_size (MPI_COMM_WORLD, &size);	/* get number of processes */
	//formato send((&)lo que se envia(variable, arreglo, matriz ,etc), numero de elementos que se manda, tipo del elemento, hilo de destino(1,2,3....n), tag, MPI common world)
	int x,y, i, j, k, aux, parte;
	int vector[100];
	
	double inicio, fin;
	
	parte = 100 / size ; 
  
	inicio = MPI_Wtime();
	if(rank == 0)
	{
		//llenamos vector
		for(j=0;j<10;j++)
		{
			vector[j] = j;
		}
		
		for(i=1; i < size ; i++)
		{
			//k = indice de la casilla del vector desde el cual enviaremos la informacion con cada iteracion
			k = (i-1)*10;
			MPI_Send(&vector[k], parte, MPI_INT, 1, 100, MPI_COMM_WORLD);
			aux = k;
			//le mandamos el indice de inicio de las siguientes casillas
			MPI_Send(&aux, 1, MPI_INT, 1, 500, MPI_COMM_WORLD);
		}
	}
	
	//bloque ejecucion hilos esclavos
	else
	{

			MPI_Recv(&vector[0], parte, MPI_INT, 0, 100, MPI_COMM_WORLD, &status);
			MPI_Recv(&aux, 1, MPI_INT, 0, 500, MPI_COMM_WORLD,&status);
			
			for(j=0;j<parte;j++)
			{
				vector[j] = vector[j]*2;
				//printf("valor: %d\n", vector[j]);
			}
			
			MPI_Send(&aux, 1, MPI_INT, 0, 700, MPI_COMM_WORLD);
			MPI_Send(&vector[0], parte, MPI_INT, 0, 200, MPI_COMM_WORLD);

	}
	////////////////////////////////
	
	if(rank==0)
	{
		for(i=1; i < size ; i++)
		{		
			MPI_Recv(&aux, 1, MPI_INT, MPI_ANY_SOURCE, 700, MPI_COMM_WORLD,&status);
			printf("aux: %d\n", aux);
			
			k = (i-1)*10;
			MPI_Recv(&vector[aux], parte, MPI_INT, MPI_ANY_SOURCE, 200, MPI_COMM_WORLD, &status);
		}
		
		fin = MPI_Wtime();
		printf("tiempo de ejecucion: %f\n", fin - inicio);
		
		
		/*for(j=0;j<10;j++)
		{
			printf("casilla %d con valor: %d\n", j, vector[j]);
		}*/
		

	}
	
	
	MPI_Finalize();
	return 0;
}
