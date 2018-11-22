#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include <mpi.h>
#include <iostream>
#include <random>
#include <ctime>
using namespace std;

void DataInitialization(double*& matrix, double*& vec_b, int& Size)
{
	srand(time(0));
	for (int i = 0; i < Size; i++)
	{
		for (int j = 0; j < Size; j++)
		{
			matrix[Size * i + j] = rand() % 100;
			if (i == j)
			{
				matrix[Size * i + j] += 10000;
			}

		}
		vec_b[i] = rand() % 100;
	}
}

void ProcessInitialization(double* &matrix, double* &vector_b, double* &x_parallel, double* &procRows, double* &procResult,
	int &Size, int &rowNum, int procRank, int procNum)
{
	int restRows; // количество строк которые не были распределены
	restRows = Size;
	for (int i = 0; i<procRank; i++)
		restRows = restRows - restRows / (procNum - i);

	rowNum = restRows / (procNum - procRank);

	vector_b = new double[Size];
	x_parallel = new double[Size];
	procRows = new double[rowNum*Size];
	procResult = new double[rowNum];
	if (procRank == 0)
	{
		matrix = new double[Size*Size];
		DataInitialization(matrix, vector_b, Size);
	}
}

void ReceiveInfoCalculation(int*& receiveNum, int*& receiveInd, int n, int ProcNum)
{
	int restRows = n; 
	receiveNum = new int[ProcNum];
	receiveInd = new int[ProcNum];
	receiveInd[0] = 0;
	receiveNum[0] = n / ProcNum;
	for (int i = 1; i < ProcNum; i++)
	{
		restRows -= receiveNum[i-1];
		receiveNum[i] = restRows / (ProcNum - i);
		receiveInd[i] = receiveInd[i - 1] + receiveNum[i - 1];
	}
}
bool converge(double *xk, double *xkp, int n, double eps)
{
	double norm = 0;
	for (int i = 0; i < n; i++)
	{
		norm += (xk[i] - xkp[i])*(xk[i] - xkp[i]);
	}
	if (sqrt(norm) >= eps)
		return false;
	return true;
}

double *Zeidel(double *arr, double *b, int n, int rowSize, double eps)
{
	double *xPrev = new double[rowSize];
	double *x = new double[rowSize];
	for (int i = 0; i < rowSize; i++)
	{
		x[i] = 0;
	}
	do
	{
		for (int i = 0; i < rowSize; i++)
			xPrev[i] = x[i];

		for (int i = 0; i < rowSize; i++)
		{
			double var = 0;
			for (int j = 0; j < i; j++)
				var += (arr[i*n + j] * x[j]);
			for (int j = i + 1; j < n; j++)
				var += (arr[i*n + j] * xPrev[j]);
			x[i] = (b[i] - var) / arr[i*n + i];
		}
	} while (!converge(x, xPrev, n, eps));
	return x;
}

void DataDistribution(double*& matrix, double*& procRows, double*& vector_b, int*& sendNum, int*& sendInd, int Size, int RowNum, int ProcRank, int ProcNum)
{
	int RestRows = Size;
	MPI_Bcast(vector_b, Size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	sendInd = new int[ProcNum];
	sendNum = new int[ProcNum];
	RowNum = (Size / ProcNum); //количество строк отдаваемых каждому процессу
	sendNum[0] = RowNum*Size; //количество элементов отдаваемых процессу с рангом 0
	sendInd[0] = 0; //индекс строки массива с которой начинаются строки в процессе 0
	for (int i = 1; i<ProcNum; i++)
	{
		RestRows -= RowNum;  //отсавшиеся строки
		RowNum = RestRows / (ProcNum - i); //изменяем количество строк передаваемых следующему процессу
		sendNum[i] = RowNum*Size; //количество элементов отдаваемых процессу с рангом i
		sendInd[i] = sendInd[i - 1] + sendNum[i - 1]; //индекс строки массива с которой начинаются строки в процессе i
	}
	// Рассылаем строки
	MPI_Scatterv(matrix, sendNum, sendInd, MPI_DOUBLE, procRows, sendNum[ProcRank], MPI_DOUBLE, 0, MPI_COMM_WORLD);

}
bool isRight(double*matrix, double*vec_b, double *x, int size)
{
	double result;
	for (int i = 0; i < size; i++)
	{
		result = 0;
		for (int j = 0; j < size; j++)
			result += matrix[i*size + j] * x[j];
		if (fabs(result - vec_b[i]) > 0.1)
			return false;
	}
	return true;
}

bool AreEqual(double *x_seq, double *x_paral, int size)
{
	for (int i = 0; i < size; i++)
	{
		if (fabs(x_seq[i] - x_paral[i]) > 0.1)
		{
			return false;
		}
	}
	return true;
}

int main(int argc, char* argv[])
{
	int ProcRank, ProcSize;
	double *matrix = NULL, *b = NULL, *x_seq = NULL, *x_paral = NULL;
	double *procRows = NULL, *procB = NULL, *procX = NULL;
	int n, rowNum;
	int *sendNum; // Количество отправленных процессу элементов
	int *sendInd; // Индекс первого среди них
	int *receiveNum; //Количество элементов которое будет отправлять данный процесс
	int *receiveInd; //Индекс первого среди них
	double time_s, time_f, time_seq, time_paral;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &ProcSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
	n = atoi(argv[1]);
	ProcessInitialization(matrix, b, x_paral, procRows, procB, n, rowNum, ProcRank, ProcSize);
	DataDistribution(matrix, procRows, b, sendNum, sendInd, n, rowNum, ProcRank, ProcSize);
	ReceiveInfoCalculation(receiveNum, receiveInd, n, ProcSize);
	procX = new double[rowNum];
	time_s = MPI_Wtime();
	procX = Zeidel(procRows, procB, n, rowNum, 0.001);
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Gatherv(procX, rowNum, MPI_DOUBLE, x_paral, receiveNum, receiveInd, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	time_f = MPI_Wtime();
	time_paral = time_f - time_s;
	if (ProcRank == 0)
	{
		time_s = MPI_Wtime();
		x_seq = new double[n];
		x_seq = Zeidel(matrix, b, n, n, 0.001);
		time_f = MPI_Wtime();
		time_seq = time_f - time_s;
		cout << "running time of the sequential algorithm: " << time_seq << endl;
		cout << "running time of the parallel algorithm: " << time_paral << endl;
		if (AreEqual(x_seq, x_paral, n))
			cout<< "results are equal" <<endl;
		else
			cout << "results are not equal" << endl;
		if (isRight(matrix, b, x_seq, n))
			cout << "Seq result is right" << endl;
		else
			cout << "Seq result is not right" << endl;
		if (isRight(matrix, b, x_paral, n))
			cout << "Parallel result is right" << endl;
		else
			cout << "Parallel result is not right" << endl;

	}
	
	
	delete[] procX;
	if (ProcRank == 0)
	{
		delete[] matrix;
		delete[] b;
		delete[] x_paral;
	}
	MPI_Finalize();
	return 0;
}