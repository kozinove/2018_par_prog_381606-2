#include "mpi.h"
#include <iostream>
#include "time.h"

using namespace std;

int main(int argc, char* argv[])
{

	int n, size, rank;
	int count = 101;
	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &size); // num of processes
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); // ¹ of process


	MPI_Status status;
	n = count / size; // mas-->blocks
	if (rank == 0)
	{
		
		int *mas;
		mas = new int[count];
		for (int i = 0; i < count; i++)							//mas generating
		{
			for (i = 0; i < count; i++)
			{
		
				mas[i] = rand() % 100 -50; // mas [-50;50]
			}

			if(count<20)
				cout<<"mas["<<i<<"]=" << mas[i] << endl;
		}
		//use Scatter(...)
		for (int i = 1; i < size; i++)
			MPI_Send((mas + n * (i - 1)), n, MPI_INT, i, 0, MPI_COMM_WORLD); // send blocks to other processes statring with mas[n*(i-1)]

		int res;

		res = mas[n*(size - 1)];
		for (int i = n * (size - 1) - 1; i < count; i++)			//calculate for head process(RANK = 0)  
		{
			if (mas[i] < res)
				res = mas[i];
		}

		int take; 
		for (int i = 1; i < size; i++)
		{
			MPI_Recv(&take, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status); //receiving info from other procs
			if (take < res)
				res = take;
		}
		cout << "result = " << res << endl;
		//int min = mas[0];
		//{
		//	for (int i = 0; i < count; i++)  // as usual
		//		if (mas[i] < min)
		//			min = mas[i];
		//}
		//cout << "the same res=" << min << endl;

		
	}
	else
	{
		int * tmp;
		tmp = new int[n];

		MPI_Recv(tmp, n, MPI_INT, 0, 0, MPI_COMM_WORLD, &status); //receiving info form 1st proc

		int res = tmp[0];
		for (int i = 0; i < n; i++)
		{
			if (tmp[i] < res)
				res = tmp[i];
		}

		MPI_Send(&res, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);  // sending info to 1st proc
	}
	MPI_Finalize();
	return 0;
	
} 