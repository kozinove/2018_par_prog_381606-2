#include "radixbatcher.h"

int main(int argv, char* argc[])
{
	int procNum, procRank;
	int size = 10000000;
	int *arr = new int[size];
	int *linearmas = new int[size];
	int root = 0;
	double stime, ftime;
	double linTime, parTime;
	

	MPI_Init(&argv, &argc);
	MPI_Comm_size(MPI_COMM_WORLD, &procNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

	if (procRank == 0)
	{
		ArrayFill(arr, size);

		linTime = TimeOfLinearSort(arr, size, linearmas);
		cout << "Time of linear Radix Sort = " << linTime << endl;
	}

	int *sendCounts = new int[procNum];
	int *displs = new int[procNum];

	if (procRank == 0)
	{
		int localBuf = size / procNum;
		int reminder = size % procNum;
		sendCounts[0] = localBuf + reminder;
		displs[0] = 0;
		for (int i = 1; i < procNum; i++)
		{
			sendCounts[i] = localBuf;
			displs[i] = reminder + i*localBuf;
		}

	}

	MPI_Bcast(sendCounts, procNum, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(displs, procNum, MPI_INT, 0, MPI_COMM_WORLD);

	//int *resultBuffer = new int[sendCounts[procRank]];
	int *resultBuffer = new int[size];

	MPI_Scatterv(arr, sendCounts, displs, MPI_INT, resultBuffer, 
		sendCounts[procRank],MPI_INT, 0, MPI_COMM_WORLD);

	if (procRank == 0)
		stime = MPI_Wtime();

	RadixSort(resultBuffer, sendCounts[procRank]);

	int n = procNum;
	int m = 1;
	int length;
	//int *tempMas = new int[size];
	//int *resMas = new int[size];

	while (n > 1)
	{
		n = n / 2 + n % 2;

		if ((procRank - m) % (2 * m) == 0)
		{
			MPI_Send(&sendCounts[procRank], 1, MPI_INT, procRank - m, 0, MPI_COMM_WORLD);
			MPI_Send(resultBuffer, sendCounts[procRank], MPI_INT, procRank - m, 0, MPI_COMM_WORLD);
			break;

		}

		if ((procRank % (2 * m) == 0) && (procNum - procRank > m))
		{
			MPI_Status status;
			MPI_Recv(&length, 1, MPI_INT, procRank + m, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			int *tempMas = new int[length];
			MPI_Recv(tempMas, length, MPI_INT, procRank + m, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			resultBuffer = BatcherMerge(resultBuffer, tempMas, sendCounts[procRank], length);
			sendCounts[procRank] += length;
		/*	if (size == sendCounts[procRank])
			{
				for (int i = 0; i < length + sendCounts[procRank]; i++)
				{
					resultBuffer[i] = resMas[i];
				}
			}*/

			
		}

		m *= 2;
	}


	if (procRank == 0)
	{
		ftime = MPI_Wtime();
		parTime = ftime - stime;
	}


	if (procRank == 0)
	{	
		//cout << "ARRAY = ";
		//for (int i = 0; i < size; i++)
		//	cout << arr[i]<<" ";
		//cout << endl;
		for (int i = 0; i < size; i++)
			arr[i] = resultBuffer[i];
		delete[] resultBuffer;
		bool flag = true;
		for(int i = 0; i < size; i++)
			if (arr[i] != linearmas[i])
			{
				flag = false;
				break;
			}
		
		//cout << "paralel result = ";
		//for (int i = 0; i < size; i++)
		//	cout << arr[i] << " ";
		//cout << endl;

		//cout << "linear result = ";
		//for (int i = 0; i < size; i++)
		//	cout << linearmas[i] << " ";
		//cout << endl;

		cout << "time of parallel sort = " << parTime << endl;
		double uskor = linTime / parTime;
		cout << "acceleration = " << uskor << endl;

		if (flag)
			cout << "The same results" << endl;
		else
			cout << "Error" << endl;
	}

	delete[] sendCounts;
	delete[] displs;
	//delete[] tempMas;
	//delete[] resMas;

	MPI_Finalize();

	delete[] arr;

}