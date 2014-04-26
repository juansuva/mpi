#include<math.h>
#include<stdio.h>
#include<time.h>

/********************************************************************
eleva d a la s, solo para s entero positivo
********************************************************************/

float eleva(float d, int s)
{
if(s==0)
return(1.0);
return(d*eleva(d,s-1));
}

/******************************************************************************
busca <b_i,b_j> con la base b={1,x, x^2,x^3...} y con los ptos en a
******************************************************************************/

float prod_scalar(float a[],int n,int i, int j)
{
int k;
float sum;
for(k=0,sum=0.0;k<n;k++)
sum=sum+(eleva(a[k],i)*eleva(a[k],j));
return(sum);
}

/*--------------------------------------------------------------------------------*/

int main(int argc, char **argv)
{

	clock_t start = clock();
	int i,npts,grado,j,k;
	npts=500000;
	grado=30;
	grado=grado+1;
	float a[npts+1],fa[npts],zx[grado][grado],zxt[grado][grado],y[grado],x[grado],sum,fzx[grado],b[2*npts/3];

	for(i=0;i<npts;i++)
	{
		a[i] = rand()%10-5;
		fa[i] = rand()%10-5;
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
		fa[6] = 2.6;*/
	
	a[npts]=grado;

	for(i=0,j=0;i<grado*grado;i++,j++)
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

	zxt[0][0]=sqrt(zx[0][0]);
	
	for(i=0;i<grado;i++)
	{
		for(j=i+1;j<grado;j++)
		{
			for(k=0,sum=0;k<i;k++)
				sum=sum+zxt[i][k]*zxt[j][k];
			zxt[j][i]=(1/zxt[i][i])*(zx[i][j]-sum);
			zxt[i][j]=zxt[j][i];
		}



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
			printf("%fx^%d",x[i],i);*/

	printf("\n");
	printf("Tiempo: %f\n",((double)clock() - start)/ CLOCKS_PER_SEC);

	return 0;
}




