#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

// Run with
// gcc para-mat-mult.c -g -Wall -I/usr/local/opt/libomp/include -L/usr/local/opt/libomp/lib

void
print_mat (int **M,
					 int n)
{
	for (int i=0; i<n; i++) {
		for (int j=0; j<n; j++)	{
			printf("%2i ", M[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

int **
mat_mult (int **A,
					int **B,
					int n)
{
	int **C = calloc(n, sizeof(int *));
	#pragma omp for
	for (int i=0; i<n; i++)
		C[i] = calloc(n, sizeof(int));

	#pragma omp for
	for (int i=0; i<n; i++)
		for (int j=0; j<n; j++)
			for (int k=0; k<n; k++)
				C[i][j] += A[i][k] * B[k][j];

	return C;
}

int
main (int argc,
			char *argv[])
{
	assert(argc > 1);
	int n;
	sscanf(argv[1], "%i", &n);

	printf("I am a non-parallel region.\n\n");
	int **A = calloc(n, sizeof(int *));
	int **B = calloc(n, sizeof(int *));
	int **C;
	for (int i=0; i<n; i++) {
		A[i] = calloc(n, sizeof(int));
		B[i] = calloc(n, sizeof(int));
	}
	for (int i=0; i<n; i++) {
		for (int j=0; j<n; j++) {
			A[i][j] = 1;
			B[i][j] = 2;
		}
	}
	print_mat(A, n);
	print_mat(B, n);
	/* sequential code */
#pragma omp parallel num_threads(n)
	{
		printf("I am a parallel region.\n");
		C = mat_mult(A, B, n);
		
	}
	/* sequential code */
	print_mat(C, n);
	return 0;
}
