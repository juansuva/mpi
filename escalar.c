/* C Example */
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define NUM_FILAS_A 5 //Filas de A
#define NUM_COLUMNAS_A 5 //Columnas de A

//#define NUM_FILAS_B 5 //Filas de B
//#define NUM_COLUMNAS_B 5 //Columnas de B

double mat_a[NUM_FILAS_A][NUM_COLUMNAS_A];  
//double mat_b[NUM_FILAS_B][NUM_COLUMNAS_B]; 

 
int main(int argc, char **argv)
{
	int rank, size;
	MPI_Status status;

	MPI_Init (&argc, &argv);	/* starts MPI */
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);	/* get current process id (hilos=0 proceso maestro,)*/
	MPI_Comm_size (MPI_COMM_WORLD, &size);	/* get number of processes */
	//formato send((&)lo que se envia(variable, arreglo, matriz ,etc), numero de elementos que se manda, tipo del elemento, hilo de destino(1,2,3....n), tag, MPI common world)
	int x = 0,y, i, j, k, aux, parte_i,parte_j;
	double inicio, fin;
	
	parte_i = 10/size;
	inicio = MPI_Wtime();
	if(rank == 0)
	{
		//llenar matriz A
		for(i = 0; i < NUM_FILAS_A; i++)
				for(j = 0;j < NUM_COLUMNAS_A;j++)
					mat_a[i][j] = x;
					
		//llenar matriz B
		/*for(i = 0; i < NUM_FILAS_B; i++)
				for(j = 0;j < NUM_COLUMNAS_B;j++)
					mat_b[i][j] = x;*/
					
					
					
		for(i = 1; i < size; i++)
		{
			porcion
			for(j = 1; j < parte_i ; j++)
			{
				v = j;
				MPI_Send(&matriz_A[k][v], 10, MPI_INT, 1, 100, MPI_COMM_WORLD);
				
				aux = k;
				//le mandamos el indice de inicio de las siguientes casillas
				MPI_Send(&aux, 1, MPI_INT, 1, 500, MPI_COMM_WORLD);
			}
		}
		
	}
	
	//bloque ejecucion hilos esclavos
	else
	{

			

	}
	////////////////////////////////
	
	if(rank==0)
	{
		
		

	}
	
	
	MPI_Finalize();
	return 0;
}
