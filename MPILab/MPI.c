#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])

{
	int *a;
	int dest;
	float factor;
	int global;
	int i;
	int master = 0;
	int my_id;
	int n;
	int npart;
	int num_procs;
	int source;
	int start;
	MPI_Status status;
	int tag;
	int tag_target = 1;
	int tag_size = 2;
	int tag_data = 3;
	int tag_found = 4;
	int tag_done = 5;
	int target;
	int workers_done;
	int x;
	
	MPI_Init(&argc, &argv);
	
	MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
	
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

	if (my_id == master)
	{
		printf("\n");
		printf("  The number of processes is %d,\n", num_procs);
	}

	printf("\n");
	printf("Process %d is active.\n", my_id);

	if (my_id == master)
	{
		factor = (float)rand() / (float)RAND_MAX;
		npart = 50 + (int)(factor * 100.0E+00);
		n = npart * num_procs;

		printf("\n");
		printf("SEARCH - Master process:\n");
		printf("  The number of data items per process is %d,\n", npart);
		printf("  The total number of data items is       %d.\n", n);
		a = (int *)malloc(n * sizeof(int));

		factor = (float)n / 10.0E+00;

		for (i = 0; i < n; i++)
		{
			a[i] = (int)(factor * (float)rand() / (float)RAND_MAX);
		}
		target = a[n / 2];

		printf("  The target value is %d.\n", target);
		for (i = 1; i <= num_procs - 1; i++)
		{
			dest = i;
			tag = tag_target;

			MPI_Send(&target, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);

			tag = tag_size;

			MPI_Send(&npart, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);

			start = (i - 1) * npart;
			tag = tag_data;

			MPI_Send(a + start, npart, MPI_INT, dest, tag,
				MPI_COMM_WORLD);
		}
		workers_done = 0;

		while (workers_done < num_procs - 1)
		{
			MPI_Recv(&x, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG,
				MPI_COMM_WORLD, &status);

			source = status.MPI_SOURCE;
			tag = status.MPI_TAG;

			if (tag == tag_done)
			{
				workers_done = workers_done + 1;
			}
			else if (tag == tag_found)
			{
				printf("P%d %d %d\n", source, x, a[x]);
			}
			else
			{
				printf("  Master process received message with unknown tag = %d.\n", tag);
			}

		}
		free(a);
	}

	else
	{
		source = master;
		tag = tag_target;

		MPI_Recv(&target, 1, MPI_INT, source, tag, MPI_COMM_WORLD,
			&status);

		source = master;
		tag = tag_size;

		MPI_Recv(&npart, 1, MPI_INT, source, tag, MPI_COMM_WORLD,
			&status);

		a = (int *)malloc(npart * sizeof(int));

		source = master;
		tag = tag_data;

		MPI_Recv(a, npart, MPI_INT, source, tag, MPI_COMM_WORLD,
			&status);
		for (i = 0; i < npart; i++)
		{
			if (a[i] == target)
			{
				global = (my_id - 1) * npart + i;
				dest = master;
				tag = tag_found;

				MPI_Send(&global, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
			}

		}
		dest = master;
		tag = tag_done;

		MPI_Send(&target, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);

		free(a);

	}
	
	MPI_Finalize();
	
	if (my_id == master)
	{
		printf("\n");
		printf("SEARCH - Master process:\n");
		printf("  Normal end of execution.\n");
		printf("\n");
	}
	return 0;
}