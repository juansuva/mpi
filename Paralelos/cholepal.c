#include<stdio.h>
#include<mpi.h>
#include<math.h>
MPI_Status status;
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
	int i,npts,grado,j,k,rank,nproc,tag=99,l;
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&nproc);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	if(rank==0)
	{
		printf("De que grado el el polinomio que se desea encontrar?\n");
		scanf("%d",&grado);
		printf("Cuantos numeros son:\n");
		scanf("%d",&npts);
		grado=grado+1;
		float a[npts+1],fa[npts],zx[grado][grado],zxt[grado][grado],zxta[grado][grado],y[grado],x[grado],sum,fzx[grado],b[2*npts/nproc+2];
		for(i=0;i<npts;i++)
		{
			printf("Dame el punto %d-esimo:\n",i+1);
			scanf("%f",&a[i]);
			printf("Dame f(%f)=\n",a[i]);
			scanf("%f",&fa[i]);
		}
		///////////////enviando datos a los demas
		a[npts]=grado; //pa ahorrarme un mensaje
		for(i=1;i<nproc;i++)
		{
			MPI_Send(&npts,1,MPI_INT,i,tag,MPI_COMM_WORLD);
			MPI_Send(a,npts+1,MPI_FLOAT,i,tag,MPI_COMM_WORLD);
			MPI_Send(fa,npts,MPI_FLOAT,i,tag,MPI_COMM_WORLD);
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
			MPI_Recv(&j,1,MPI_INT,i,tag,MPI_COMM_WORLD,&status);
			MPI_Recv(b,j,MPI_FLOAT,i,tag,MPI_COMM_WORLD,&status);
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
			MPI_Send(zx,grado*grado,MPI_FLOAT,i,tag,MPI_COMM_WORLD);
		zxt[0][0]=sqrt(zx[0][0]);
		for(i=0;i<grado;i++)
		{
			//mandando a los procesos la parte que se a
			//calculado de la matriz triangular T
			for(j=1;j<nproc;j++)
				MPI_Send(zxt,grado*grado,MPI_FLOAT,j,tag,MPI_COMM_WORLD);
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
				MPI_Recv(zxta,grado*grado,MPI_INT,j,tag,MPI_COMM_WORLD,&status);
				//almacenado datos
				for(k=i+1+j;k<grado;k=k+nproc)
				{
					zxt[k][i]=zxta[k][i];
					zxt[i][k]=zxt[k][i];
				}
			}
			//calculando los elementos diagonal de la matriz triangular T
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
			printf("y=%f\n",y[i]);
		}
		x[grado-1]=y[grado-1]/zxt[grado-1][grado-1];
		printf("x[%d]=%f\n",grado-1,x[grado-1]);
		for(i=grado-2;i>=0;i--)
		{
			for(k=i+1,sum=0;k<grado;k++)
				sum=sum+zxt[i][k]*x[k];
			x[i]=(1/zxt[i][i])*(y[i]-sum);
			printf("x[%d]=%f\n",i,x[i]);
		}
		printf("\nEl polinomio es:");
		for(i=0;i<grado;i++)
			if(x[i]>0)
				printf(" + %fx^%d",x[i],i);
			else if(x[i]<0)
				printf("%fx^%d",x[i],i);
			printf("\n");
	}
	else
	{
		MPI_Recv(&npts,1,MPI_INT,0,tag,MPI_COMM_WORLD,&status);
		float a[npts/nproc+1],b[2*npts/nproc+2],fa[npts/nproc+1],sum;
		MPI_Recv(a,npts+1,MPI_FLOAT,0,tag,MPI_COMM_WORLD,&status);
		MPI_Recv(fa,npts,MPI_FLOAT,0,tag,MPI_COMM_WORLD,&status);
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
		MPI_Send(&j,1,MPI_INT,0,tag,MPI_COMM_WORLD);
		MPI_Send(b,j,MPI_FLOAT,0,tag,MPI_COMM_WORLD);
		float zx[grado][grado],zxt[grado][grado];
		MPI_Recv(zx,grado*grado,MPI_FLOAT,0,tag,MPI_COMM_WORLD,&status);
		for(i=0;i<grado;i++)
		{
			MPI_Recv(zxt,grado*grado,MPI_FLOAT,0,tag,MPI_COMM_WORLD,&status);
			for(j=i+1+rank;j<grado;j=j+nproc)
			{
				for(k=0,sum=0;k<i;k++)
					sum=sum+zxt[i][k]*zxt[j][k];
				zxt[j][i]=(1/zxt[i][i])*(zx[i][j]-sum);
				zxt[i][j]=zxt[j][i];
			}
			MPI_Send(zxt,grado*grado,MPI_FLOAT,0,tag,MPI_COMM_WORLD);
		}
	}
	MPI_Finalize(); 
}
