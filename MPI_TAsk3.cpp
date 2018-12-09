#include "mpi.h"
#include <iostream>
#include <ctime>
#include "Windows.h"
#include <algorithm>

const int MAX_SIZE = 10000000;

//Hibbard step
int HibbardStep(long inc[], unsigned int size) {

	int i;
	for (i = 1; pow(2, i) - 1 <= size; ++i)
	{
		inc[i] = pow(2, i) - 1;
	}

	return i > 0 ? ++i : 0;
}
//Sedjvik sorted
int SedjvikStep(long inc[], unsigned int size) {
	int p1, p2, p3, s;

	p1 = p2 = p3 = 1;
	s = -1;
	do {
		if (++s % 2) 
		{
			inc[s] = 8 * p1 - 6 * p2 + 1;
		}
		else 
		{
			inc[s] = 9 * p1 - 9 * p3 + 1;
			p2 *= 2;
			p3 *= 2;
		}
		p1 *= 2;
	} while (3 * inc[s] < size);

	return s > 0 ? s : 0;
}

long int* merge(long int *A, int asize, long int *B, int bsize)
{
	int ai, bi, ci, i;
	long int *C;
	int csize = asize + bsize;

	ai = 0;
	bi = 0;
	ci = 0;

	C = new long int[csize];
	while ((ai < asize) && (bi < bsize))
	{
		if (A[ai] <= B[bi])
		{
			C[ci] = A[ai];
			ci++; ai++;
		}
		else
		{
			C[ci] = B[bi];
			ci++; bi++;
		}
	}

	if (ai >= asize)
		for (i = ci; i < csize; i++, bi++)
			C[i] = B[bi];
	else if (bi >= bsize)
		for (i = ci; i < csize; i++, ai++)
			C[i] = A[ai];

	for (i = 0; i < asize; i++)
		A[i] = C[i];
	for (i = 0; i < bsize; i++)
		B[i] = C[asize + i];

	return C;
}

bool Equals(long int *arr1, long int *arr2, double &time)
{
	std::cout << "START EQUALS  " << std::endl;
	 double start_time = clock();
	std::qsort(arr1, MAX_SIZE, sizeof *arr1, [](const void *a, const void* b)
	{
		int arg1 = *static_cast<const int*>(a);
		int arg2 = *static_cast<const int*>(b);

		if (arg1 < arg2) return -1;
		if (arg1 > arg2) return 1;
		return 0;
	}
	);

	double end_time = clock();
	time = (end_time - start_time) / 1000;
	std::cout << "QSort time: " << time << std::endl;

	if (MAX_SIZE <= 30)
	{
		std::cout << "qsort is:  ";
		for (long int i = 0; i < MAX_SIZE; ++i)
		{
			std::cout << arr1[i] << " ";
		}
		std::cout << std::endl;
	}
	for (long int i = 0; i < MAX_SIZE; ++i)
	{
		if (arr1[i] != arr2[i])
			return false;
	}
	std::cout << "END EQUALS  " << std::endl;

	return true;
}

void ShellSort(long int *a, long size) {
	long inc, i, j, seq[400];
	int s;

	s = SedjvikStep(seq, size);
	while (s >= 0) 
	{
		inc = seq[s--];

		for (i = inc; i < size; ++i) 
		{
			long int temp = a[i];
			for (j = i - inc; (j >= 0) && (a[j] > temp); j -= inc)
				a[j + inc] = a[j];
			a[j + inc] = temp;
			//for (unsigned long long int i = 1; i < send_counts[procID]; ++i)
			//for (unsigned long long int j = i; j >= 0 && a[j - inc] > a[j]; j -= inc)
			//	std::swap(a[j - inc], a[j]);
		}
	}
}

double& OneProcTime(long int* arr1, double& time)
{
	double start_time = clock();
	ShellSort(arr1, MAX_SIZE);
	double end_time = clock();
	time = (end_time - start_time) / 1000;
	//std::cout << "time: " << time << std::endl;
	return time;
}

int main(int argc, char *argv[])
{
	srand(63);

	double MySortTimeParallel;
	double MySortOneProcTimeHalf1, MySortOneProcTimeHalf2;
	double QSortTIme;
	int ProcSize, procID;
	int *send_counts, *displacements;
	long int *RecieveBuffer;
	int localBuff = 0, reminder;
	double resultTime = 0;

	long int *TempArray = 0;
	long int *Array = 0;

	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &ProcSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &procID);

	if (!procID)
	{
		Array = new long int[MAX_SIZE];
		TempArray = new long int[MAX_SIZE];
		for (int i = 0; i < MAX_SIZE; i++)
		{
			TempArray[i] = Array[i] = rand();
			if (MAX_SIZE <= 30)
				std::cout << Array[i] << " ";
		}
		std::cout << std::endl;
	}

	MySortTimeParallel = MPI_Wtime();
	MySortOneProcTimeHalf1 = MPI_Wtime();
	send_counts = new int[ProcSize];
	displacements = new int[ProcSize];
	//localBuff = MAX_SIZE / ProcSize;
	reminder = MAX_SIZE % ProcSize; 

	///brain dead 
	//send_counts[0] = localBuff + reminder;
	//displacements[0] = 0;
	//for (int i = 1; i < ProcSize; ++i)
	//{
	//	send_counts[i] = localBuff;
	//	displacements[i] = reminder + i * localBuff;
	//}

	int temp = 0;
	for (int i = 0; i < ProcSize; ++i) {
		send_counts[i] = MAX_SIZE / ProcSize;
		if (reminder > 0)
		{
			send_counts[i]++;
			reminder--;
		}

		displacements[i] = temp;
		temp += send_counts[i];
	}


	RecieveBuffer = new long int[send_counts[procID]];

	MySortOneProcTimeHalf1 = MPI_Wtime() - MySortOneProcTimeHalf1;

	MPI_Scatterv(Array, send_counts, displacements, MPI_INT, RecieveBuffer, send_counts[procID], MPI_INT, 0, MPI_COMM_WORLD);

	if (MAX_SIZE <= 30)
	{
		std::cout << procID << "   not";
		for (int i = 0; i < send_counts[procID]; i++)
		{
			std::cout << " " << RecieveBuffer[i];
		}
		std::cout << std::endl;
	}

	ShellSort(RecieveBuffer, send_counts[procID]);

	if (MAX_SIZE <= 30)
	{
		std::cout << procID << "   sorted ";
		for (int i = 0; i < send_counts[procID]; i++)
		{
			std::cout << " " << RecieveBuffer[i];
		}
		std::cout << std::endl;
	}

	int s = ProcSize, step = 1;

	while (s > 1)
	{
		s = s / 2 + s % 2;
		if ((procID % (2 * step) == 0) && (ProcSize - procID > step))
		{
			MPI_Status status;

			int tempSize = 0;
			MPI_Recv(&tempSize, 1, MPI_INT, procID + step, 0, MPI_COMM_WORLD, &status);
			long int *tempHalfArray = new long int[tempSize];
			MPI_Recv(tempHalfArray, tempSize, MPI_INT, procID + step, 0, MPI_COMM_WORLD, &status);
			RecieveBuffer = merge(RecieveBuffer, send_counts[procID], tempHalfArray, tempSize);
			send_counts[procID] = send_counts[procID] + tempSize;
			delete[] tempHalfArray;
			if (MAX_SIZE == send_counts[procID])
			{
				double time2 = MPI_Wtime();
				for (int i = 0; i < MAX_SIZE; ++i)
					Array[i] = RecieveBuffer[i];
				time2 = MPI_Wtime() - time2;
				//std::cout << "Copy time is: " << time2 << std::endl;

			}
		}
		if ((procID - step) % (2 * step) == 0)
		{
			MPI_Send(&send_counts[procID], 1, MPI_INT, procID - step, 0, MPI_COMM_WORLD);
			MPI_Send(RecieveBuffer, send_counts[procID], MPI_INT, procID - step, 0, MPI_COMM_WORLD);
			break;
		}
		step = step * 2;
	}

	MySortTimeParallel = MPI_Wtime() - MySortTimeParallel;
	MPI_Reduce(&MySortTimeParallel, &resultTime, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

	if (ProcSize == 1)
	{
		for (long int i = 0; i < MAX_SIZE; ++i)
		{
			Array[i] = RecieveBuffer[i];
		}
	}

	if (!procID)
	{
		for (int i = 0; i < MAX_SIZE; i++)
		{
			if (MAX_SIZE <= 30)
				std::cout << Array[i] << " ";
		}
		std::cout << std::endl;
	}

	///check on right
	//if (!procID)
	//{
	//	if (Equals(TempArray, Array, QSortTIme) == true)
	//		std::cout << "All is fine" << std::endl;
	//	else
	//		std::cout << "ERROR SORTING IS WRONG" << std::endl;
	//}

	if (!procID)
	{
		resultTime /= ProcSize;
		std::cout << "Average time is: " << resultTime << std::endl;
		OneProcTime(TempArray, MySortOneProcTimeHalf2);
		std::cout << "One process time is: " << MySortOneProcTimeHalf1 + MySortOneProcTimeHalf2 << std::endl;
		std::cout << "BOOST IS: " << (MySortOneProcTimeHalf1 + MySortOneProcTimeHalf2) / resultTime << std::endl;
	}

	if (!procID)
	{
		delete Array;
		delete TempArray;
	}

	MPI_Finalize();
	delete[] RecieveBuffer;
	delete[] send_counts;
	delete[] displacements;

	return 0;
}


