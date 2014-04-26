#include<stdio.h>
#include<mpi.h>
#include<math.h>
#include<time.h>
MPI_Status status;
#define MASTER_TO_SLAVE_TAG 1 //tag for messages sent from master to slaves
#define SLAVE_TO_MASTER_TAG 10 //tag for messages sent from slaves to master

/********************************************************************
eleva d a la s, solo para s entero positivo
********************************************************************/
float eleva(float d, int s)
{
	if(s==0)
		return(1.0);
	return(d*eleva(d,s-1));
}
/*****************************************************************************+
busca <b_i,b_j> con la base b={1,x, x^2,x^3...} y con los ptos en a
***************************************************************************+**/
float prod_scalar(float a[],int n,int i, int j)
{
	int k;
	float sum;
	for(k=0,sum=0.0;k<n;k++)
		sum=sum+(eleva(a[k],i)*eleva(a[k],j));
	return(sum);
}
int main(int argc, char **argv)
{
	int i,npts=500000,grado=50,j,k,rank,nproc,l,tam_a,tam_fa,parte_a,parte_fa;
	double inicio,inicio_2 ,fin,fin_2,comm_time=0.0,dt,aux=0.0;
	MPI_Init(&argc,&argv);
	
	MPI_Comm_size(MPI_COMM_WORLD,&nproc);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	float a[npts+1],fa[npts],zx[grado][grado],zxt[grado][grado],zxta[grado][grado],y[grado],x[grado],sum,fzx[grado],b[2*npts/nproc+2];
	
	srand(time(NULL));
	//-----------------------------------------maestro----------------------------------------------//
	
	if(rank==0)
	{
		
		//npts = 100;
		//float a[npts/nproc+1],b[2*npts/nproc+2],fa[npts/nproc+1],sum;
		grado=grado+1;
		//float a[npts+1],fa[npts],zx[grado][grado],zxt[grado][grado],zxta[grado][grado],y[grado],x[grado],sum,fzx[grado],b[2*npts/nproc+2];
		inicio = MPI_Wtime();
		for(i=0;i<npts;i++)
		{
		   //printf("x%d:\n",i+1);
			//scanf("%f",&a[i]);
			a[i] = rand()%10 -5;
			//printf("Valor de x[%d]: %f\n",i,a[i]);
			//printf("y%d:\n",i+1);
			//scanf("%f",&fa[i]);
			fa[i] = rand()%10 -5;
			//printf("Valor de y[%d]: %f\n",i+1,fa[i]);
			

		}
		/*
		a[0] = 0.75;
		a[1] = 2;
		a[2] = 3;
		a[3] = 4;
		a[4] = 6;
		a[5] = 8;
		a[6] = 8.5;
		
		fa[0] = 1.2;
		fa[1] = 1.95;
		fa[2] = 2;
		fa[3] = 2.4;
		fa[4] = 2.4;
		fa[5] = 2.7;
		fa[6] = 2.6;
		*/
		///////////////enviando datos a los demas
		a[npts]=grado; //pa ahorrarme un mensaje
		for(i=1;i<nproc;i++)
		{
			
			
			
			MPI_Send(&npts,1,MPI_INT,i,MASTER_TO_SLAVE_TAG,MPI_COMM_WORLD);   //se envia el numero de puntos
			
			MPI_Send(a,npts+1,MPI_FLOAT,i,MASTER_TO_SLAVE_TAG+1,MPI_COMM_WORLD);//se envia vector a[] con valores "x", con el numero de puntos + 1
			
		
			MPI_Send(fa,npts,MPI_FLOAT,i,MASTER_TO_SLAVE_TAG+2,MPI_COMM_WORLD);//se envia vector fa[] con valores "y", con el numero de puntos
			
		}
		////////////////////Mi parte
		for(i=0,j=0;i<grado*grado;i=i+nproc,j++)
		{
			zx[i%grado][i/grado]=prod_scalar(a,npts,i%grado,i/grado);
			zx[i/grado][i%grado]=zx[i%grado][i/grado];
			
		}
		for(i=0;i<grado;i++)
		{
			for(sum=0,j=0;j<npts;j++)
				sum=sum+(fa[j]*eleva(a[j],i));
			fzx[i]=sum;
		}
		/////////////////////////////Recibiendo la parte de lops demas
		for(i=1;i<nproc;i++)
		{
			MPI_Recv(&j,1,MPI_INT,i,SLAVE_TO_MASTER_TAG,MPI_COMM_WORLD,&status);
			MPI_Recv(b,j,MPI_FLOAT,i,SLAVE_TO_MASTER_TAG+1,MPI_COMM_WORLD,&status);
			//almacenando los datos <bj,bi>
			for(l=i,k=0;l<grado*grado;l=l+nproc)
			{
				zx[l%grado][l/grado]=b[k];
				zx[l/grado][l%grado]=b[k];
				k++;
			}
			//almacenando los datos <f,bj>
			for(l=i;l<grado;l=l+nproc,k++)
				fzx[l]=b[k];
		}
		///////resolviendo el sistema de ecuaciones
		///mandando a todos los porcesos la matriz a
		for(i=1;i<nproc;i++)
		
		MPI_Send(zx,grado*grado,MPI_FLOAT,i,MASTER_TO_SLAVE_TAG+3,MPI_COMM_WORLD);
		
		zxt[0][0]=sqrt(zx[0][0]);
		for(i=0;i<grado;i++)
		{
			//mandando a los procesos la parte que se a
			//calculado de la matriz triangular T
			for(j=1;j<nproc;j++)
				MPI_Send(zxt,grado*grado,MPI_FLOAT,j,MASTER_TO_SLAVE_TAG+4,MPI_COMM_WORLD);
				
			for(j=i+1;j<grado;j=j+nproc)
			{
				for(k=0,sum=0;k<i;k++)
					sum=sum+zxt[i][k]*zxt[j][k];
				zxt[j][i]=(1/zxt[i][i])*(zx[i][j]-sum);
				zxt[i][j]=zxt[j][i];
			}
			///recibir los datos de los procesos
			for(j=1;j<nproc;j++)
			{
				
				MPI_Recv(zxta,grado*grado,MPI_INT,j,SLAVE_TO_MASTER_TAG+2,MPI_COMM_WORLD,&status);
				
				//almacenado datos
				for(k=i+1+j;k<grado;k=k+nproc)
				{
					zxt[k][i]=zxta[k][i];
					zxt[i][k]=zxt[k][i];
				}
			}
			//calculando los elementos diagonal de la matriz triangular T con cholesky
			if(i+1<grado)
			{
				for(sum=0,k=0;k<i+1;k++)
					sum=sum+zxt[i+1][k]*zxt[i+1][k];
				zxt[i+1][i+1]=sqrt(zx[i+1][i+1]-sum);
			}
		}
		y[0]=fzx[0]/zxt[0][0];
		for(i=1;i<grado;i++)
		{
			for(k=0,sum=0;k<i;k++)
				sum=sum+zxt[i][k]*y[k];
			y[i]=(1/zxt[i][i])*(fzx[i]-sum);
			//printf("y=%f\n",y[i]);
		}
		x[grado-1]=y[grado-1]/zxt[grado-1][grado-1];
		//printf("x[%d]=%f\n",grado-1,x[grado-1]);
		for(i=grado-2;i>=0;i--)
		{
			for(k=i+1,sum=0;k<grado;k++)
				sum=sum+zxt[i][k]*x[k];
			x[i]=(1/zxt[i][i])*(y[i]-sum);
			//printf("x[%d]=%f\n",i,x[i]);
		}
		/*printf("\nEl polinomio es:");
		for(i=0;i<grado;i++)
			if(x[i]>0)
				printf(" + %fx^%d",x[i],i);
			else if(x[i]<0)
				printf("%fx^%d",x[i],i);
		printf("\n");*/
		for(i=1;i<nproc;i++)
		{
			MPI_Recv(&comm_time,1,MPI_DOUBLE,i,SLAVE_TO_MASTER_TAG+3,MPI_COMM_WORLD,&status);
			aux = aux + comm_time;
		}
		fin = MPI_Wtime();
		printf("\nTiempo procesamiento = %f\n",(fin-inicio)-comm_time);
		printf("\nTiempo de comunicacion = %f\n",comm_time);
		printf("\nTiempo ejecucion total = %f\n\n",fin-inicio);

	}
	
	//----------------------------workers------------------------------------//
	
	else
	{
		inicio_2 = MPI_Wtime();
		MPI_Recv(&npts,1,MPI_INT,0,MASTER_TO_SLAVE_TAG,MPI_COMM_WORLD,&status);
		
		//float a[npts/nproc+1],b[2*npts/nproc+2],fa[npts/nproc+1],sum;

		MPI_Recv(a,npts+1,MPI_FLOAT,0,MASTER_TO_SLAVE_TAG+1,MPI_COMM_WORLD,&status);
		MPI_Recv(fa,npts,MPI_FLOAT,0,MASTER_TO_SLAVE_TAG+2,MPI_COMM_WORLD,&status);
		fin_2 = MPI_Wtime();
		dt = fin_2 - inicio_2;
		comm_time += dt;
		
		grado=a[npts];
		for(i=rank,j=0;i<grado*grado;i=i+nproc)
		{
			b[j]=prod_scalar(a,npts,i%grado,i/grado);
			j++;
		}
		for(i=rank;i<grado;i=i+nproc)
		{
			for(sum=0,k=0;k<npts;k++)
				sum=sum+(fa[k]*eleva(a[k],i));
			b[j]=sum;
			j++;
		}
		inicio_2 = MPI_Wtime();
		
		MPI_Send(&j,1,MPI_INT,0,SLAVE_TO_MASTER_TAG,MPI_COMM_WORLD);
		MPI_Send(b,j,MPI_FLOAT,0,SLAVE_TO_MASTER_TAG+1,MPI_COMM_WORLD);
		
		fin_2 = MPI_Wtime();
		dt = fin_2 - inicio_2;
		comm_time += dt;
		
		float zx[grado][grado],zxt[grado][grado];
		
		inicio_2 = MPI_Wtime();
		MPI_Recv(zx,grado*grado,MPI_FLOAT,0,MASTER_TO_SLAVE_TAG+3,MPI_COMM_WORLD,&status);
		fin_2 = MPI_Wtime();
		dt = fin_2 - inicio_2;
		comm_time += dt;
		
		for(i=0;i<grado;i++)
		{	
			inicio_2 = MPI_Wtime();
			MPI_Recv(zxt,grado*grado,MPI_FLOAT,0,MASTER_TO_SLAVE_TAG+4,MPI_COMM_WORLD,&status);
			fin_2 = MPI_Wtime();
			dt = fin_2 - inicio_2;
			comm_time += dt;
		
			for(j=i+1+rank;j<grado;j=j+nproc)
			{
				for(k=0,sum=0;k<i;k++)
					sum=sum+zxt[i][k]*zxt[j][k];
				zxt[j][i]=(1/zxt[i][i])*(zx[i][j]-sum);
				zxt[i][j]=zxt[j][i];
			}
			
			inicio_2 = MPI_Wtime();
			MPI_Send(zxt,grado*grado,MPI_FLOAT,0,SLAVE_TO_MASTER_TAG+2,MPI_COMM_WORLD);
			fin_2 = MPI_Wtime();
			dt = fin_2 - inicio_2;
			comm_time += dt;
		}
		MPI_Send(&comm_time,1,MPI_DOUBLE,0,SLAVE_TO_MASTER_TAG+3,MPI_COMM_WORLD);

	}
	MPI_Finalize();
	return 0;
}





