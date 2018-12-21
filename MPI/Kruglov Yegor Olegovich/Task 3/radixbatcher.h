#pragma once
#include <iostream>
#include <time.h>
#include "mpi.h"

using namespace std;

void Radix(int byte, int size, int *sourceArr, int *destArr)
{
	int count[256];
	int offset[256];
	memset(count, 0, sizeof(count));

	for (int i = 0; i < size; i++)
	{
		if (byte == 3)
			count[((sourceArr[i] >> (byte * 8)) + 128) & 0xff]++;
		else
			count[((sourceArr[i]) >> (byte * 8)) & 0xff]++;
	}

	offset[0] = 0;
	for (int i = 1; i < 256; ++i)
		offset[i] = offset[i - 1] + count[i - 1];


	for (int i = 0; i < size; ++i)
	{
		if (byte == 3)
			destArr[offset[((sourceArr[i] >> (byte * 8)) + 128) & 0xff]++] = sourceArr[i];
		else
			destArr[offset[((sourceArr[i]) >> (byte * 8)) & 0xff]++] = sourceArr[i];
	}
}

void RadixSort(int *sourceArr, int size)
{
	int *temp = new int[size];
	Radix(0, size, sourceArr, temp);
	Radix(1, size, temp, sourceArr);
	Radix(2, size, sourceArr, temp);
	Radix(3, size, temp, sourceArr);
	delete[] temp;
}

void ArrayFill(int *arr, int size)
{
	//srand(time(NULL));
	srand(6);
	for (int i = 0; i < size; i++)
		arr[i] = rand();
}

void EvenSplit(int* arr, int* tmp, int size1, int size2)
{
	//for (int i = 0; i < size1; i++)
		//tmp[i] = arr[i];

	int *arr2 = arr + size1;

	int a = 0;
	int b = 0;
	int i = 0;

	while ((a < size1) && (b < size2))
	{
		if (tmp[a] < arr2[b])
		{
			arr[i] = tmp[a];
			a += 2;
		}

		else
		{
			arr[i] = arr2[b];
			b += 2;
		}

		i += 2;
	}

	if (a == size1)
		for (int j = b; j < size2; j += 2, i += 2)
			arr[i] = arr2[j];
	else
		for (int j = a; j<size1; j += 2, i += 2)
			arr[i] = tmp[j];
}

void OddSplit(int* arr, int* tmp, int size1, int size2)
{
	//for (int i = 0; i < size1; i++)
	//	tmp[i] = arr[i];

	int *arr2 = arr + size1;

	int a = 1;
	int b = 1;
	int i = 1;

	while ((a < size1) && (b < size2))
	{
		if (tmp[a] < arr2[b])
		{
			arr[i] = tmp[a];
			a += 2;
		}

		else
		{
			arr[i] = arr2[b];
			b += 2;
		}

		i += 2;
	}

	if (a == size1)
		for (int j = b; j < size2; j += 2, i += 2)
			arr[i] = arr2[j];
	else
		for (int j = a; j<size1; j += 2, i += 2)
			arr[i] = tmp[j];
}

void Comparator(int *arr, int size)
{
	for (int i = 1; i < size; i++)
		if (arr[i] < arr[i - 1])
			std::swap(arr[i], arr[i - 1]);
}

double TimeOfLinearSort(int *arr, int size, int *linearmas)
{
	int *tmp = new int[size];
	double startTime;
	double finishTime;
	for (int i = 0; i < size; i++)
		tmp[i] = arr[i];
	startTime = MPI_Wtime();
	RadixSort(tmp, size);
	finishTime = MPI_Wtime();
	for (int i = 0; i < size; i++)
		linearmas[i] = tmp[i];
	delete[] tmp;
	return (finishTime - startTime);
}

int* BatcherMerge(int *arr1, int *arr2, int size1, int size2)
{
	int *res = new int[size1 + size2];
	//for (int i = 0; i < size1; i++)
		//res[i] = arr1[i];
	//for (int j = 0; j < size2; j++)
		//res[j + size1] = arr2[j];

	memcpy(res, arr1, size1 * sizeof(int));
	memcpy(res + size1, arr2, size2 * sizeof(int));

	//int *tmp = new int[size1];
	EvenSplit(res, arr1 /*tmp*/, size1, size2);
	OddSplit(res, arr1 /*tmp*/, size1, size2);
	Comparator(res, size1 + size2);

	//delete[] tmp;
	return res;
}

void reallocate(int* &arr, int oldsize, int newsize)
{
	int *newArr = new int[newsize]{ 0 };
	memcpy(newArr, arr, oldsize * sizeof(int));
	delete[] arr;
	arr = newArr;
}

//void Batcher_tree(int *rcvBuf, int *res, int *sendCounts, int size,
//	MPI_Datatype type, int root, MPI_Comm comm)
//{
//	MPI_Status status;
//	int procRank, procNum;
//	MPI_Comm_size(MPI_COMM_WORLD, &procNum);
//	MPI_Comm_rank(MPI_COMM_WORLD, &procRank);
//
//	int rankRecv, rankSend;
//	int *resBuf = new int[size];
//	int recvArrSize = 0;
//	int sendArrSize = sendCounts[procRank];
//	int newProcRank = (procRank + procNum) % procNum;
//
//	int mask = 1;
//
//	while (mask < procNum)
//	{
//		if ((newProcRank & mask) == 0)
//		{
//			rankSend = newProcRank | mask;
//			if (rankSend < procNum)
//			{
//				rankSend = (rankSend + root) % procNum;
//				MPI_Recv(&recvArrSize, 1, MPI_INT, rankSend, 0, comm, &status);
//				MPI_Recv(res, recvArrSize, type, rankSend, 0, comm, &status);
//
//				BatcherMerge(rcvBuf, res, resBuf, sendArrSize, recvArrSize);
//				reallocate(rcvBuf, sendArrSize, sendArrSize + recvArrSize);
//
//				sendArrSize += recvArrSize;
//				memcpy(rcvBuf, resBuf, sendArrSize * sizeof(int));
//			}
//		}
//
//		else
//		{
//			rankRecv = newProcRank & (~mask);
//			rankRecv = (rankRecv + root) % procNum;
//			MPI_Send(&sendArrSize, 1, MPI_INT, rankRecv, 0, comm);
//			MPI_Send(rcvBuf, sendArrSize, type, rankRecv, 0, comm);
//
//			break;
//		}
//		mask = mask << 1;
//	}
//
//	if (procRank != root)
//	{
//		delete[] rcvBuf;
//		delete[] resBuf;
//	}
//
//	if (procRank == root)
//	{
//		memcpy(res, resBuf, size * sizeof(int));
//		delete[] rcvBuf; 
//		delete[] resBuf;
//	}
//}


