#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#include "mpi.h"

#define N 500000
#define TSTEPS 50000 // MEDIUM_DATASET



int main(int argc, char** argv)
{
  int n = N;
  int tsteps = TSTEPS;
  MPI_Request req[4];
  MPI_Status status[4];

  int t, i, rank, ranks;
  int ll, shift;
  int start_elem, last_elem, count_elem;
  ll = 4;
  shift = 0;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &ranks);

  start_elem = ((rank) * n) / ranks;
  last_elem = ((rank + 1) * n) / ranks;
  count_elem = last_elem - start_elem;

  float (*A) = (float*) malloc((count_elem + 2) * sizeof(float));
  float (*B) = (float*) malloc((count_elem + 2) * sizeof(float));

  for(i = 1; i <= count_elem; ++i){
    A[i] = ((float)(start_elem + i - 1) + 2) / n;
    B[i] = ((float)(start_elem + i - 1) + 3) / n;
  }

  MPI_Barrier(MPI_COMM_WORLD);
  double tm1 = MPI_Wtime();

  for (t = 0; t < tsteps; t++)
  {

    if(ranks != 1){
      if (rank != 0){
        MPI_Isend(&A[1], 1, MPI_FLOAT, rank - 1, 1111, MPI_COMM_WORLD, &req[1]);
        MPI_Irecv(&A[0], 1, MPI_FLOAT, rank - 1, 2222, MPI_COMM_WORLD, &req[0]);
      }
      if (rank != ranks - 1){
        MPI_Irecv(&A[count_elem + 1], 1, MPI_FLOAT, rank + 1, 1111, MPI_COMM_WORLD, &req[3]);
        MPI_Isend(&A[count_elem], 1, MPI_FLOAT, rank + 1, 2222, MPI_COMM_WORLD, &req[2]);
      }

      if ((rank == 0) || (rank == ranks - 1)){
        ll = 2;
        if (rank == 0){
          shift = 2;
        }
      }
      MPI_Waitall(ll, &req[shift], &status[0]);
    }

    for (i = 1; i <= count_elem; i++){
      if (((rank == 0) && (i == 1)) || ((rank == ranks - 1) && (i == count_elem))){
        continue;
      }
      B[i] = 0.33333 * (A[i-1] + A[i] + A[i + 1]);
    }

    if(ranks != 1){
      if (rank != 0){
        MPI_Isend(&B[1], 1, MPI_FLOAT, rank - 1, 1111, MPI_COMM_WORLD, &req[1]);
        MPI_Irecv(&B[0], 1, MPI_FLOAT, rank - 1, 2222, MPI_COMM_WORLD, &req[0]);
      }
      if (rank != ranks - 1){
        MPI_Irecv(&B[count_elem + 1], 1, MPI_FLOAT, rank + 1, 1111, MPI_COMM_WORLD, &req[3]);
        MPI_Isend(&B[count_elem], 1, MPI_FLOAT, rank + 1, 2222, MPI_COMM_WORLD, &req[2]);
      }

      if ((rank == 0) || (rank == ranks - 1)){
        ll = 2;
        if (rank == 0){
          shift = 2;
        }
      }

      MPI_Waitall(ll, &req[shift], &status[0]);
    }


    for (i = 1; i <= count_elem; i++)
    {
      if (((rank == 0) && (i == 1)) || ((rank == ranks - 1) && (i == count_elem))){
        continue;
      }
      A[i] = 0.33333 * (B[i-1] + B[i] + B[i + 1]);
    }
  }

  printf("Process: %d Time: %lf\n", rank, MPI_Wtime() - tm1);

  free((void*)A);
  free((void*)B);

  MPI_Finalize();

  return 0;
}
