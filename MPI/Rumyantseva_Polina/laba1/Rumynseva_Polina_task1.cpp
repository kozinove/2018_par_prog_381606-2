#include"mpi.h"

#include <iostream>
#include <random>


int** new_int_matrix(int rows, int cols) //��������� ������ ��� �������
{
	int *data = new int[rows*cols];
	int **array = new int*[rows];
	for (int i = 0; i < rows; i++)
		array[i] = &(data[cols*i]);

	return array;
}


int main(int argc, char *argv[])
{

	int** matrix = nullptr;
	int* vect = nullptr;
	int* tmp = nullptr;

	int N; // ������ �������
	int id, num;
	double start, end, parallelTime;

	MPI_Init(&argc, &argv); // ������������� ����� ���������� MPI-���������
	MPI_Comm_size(MPI_COMM_WORLD, &num); // ����������� ���������
	MPI_Comm_rank(MPI_COMM_WORLD, &id); // ����������� ����� ��������

	if (id == 0)
	{
		std::cout << "Enter size of matrix: ";
		std::cin >> N;
		std::cout << std::endl;
	}

	MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD); // �������� ������ ���������

	matrix = new_int_matrix(N, N);
	vect = new int[N];
	tmp = new int[N];

	if (id == 0)
	{
		std::default_random_engine gen;
		std::uniform_int_distribution<int> dist(0, 100);	// ������������� ������� ���������� �������
		std::cout << std::endl;
		for (size_t i = 0; i < N; ++i)
		{
			for (int j = 0; j < N; ++j)
			{
				matrix[i][j] = dist(gen);
				std::cout << matrix[i][j] << "\t";
			}
			std::cout << std::endl;
		}

	}

	MPI_Bcast(*matrix, N*N, MPI_INT, 0, MPI_COMM_WORLD);

	start = MPI_Wtime();
	
	for (int i = (id); i < N; i += (num)) // ����� ������������� �������� (�����������)
	{
		int max = -1;
		for (int j = 0; j < N; j++)
		{
			if (matrix[i][j] > max)
				max = matrix[i][j];
		}
		tmp[i] = max;
	}

	MPI_Reduce(tmp, vect, N, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD); // "��������" ���������� ����������

	end = MPI_Wtime();
	parallelTime = end - start;
		
	if (id == 0) // ����� ������������� �������� (�������)
	{
		
		double startl = MPI_Wtime();
		std::cout << std::endl;
		for (int i = 0; i < N; ++i)
		{
			int max = -1;
			for (int j = 0; j < N; ++j)
			{
				if (matrix[i][j] > max)
					max = matrix[i][j];
			}
			std::cout << max << "\t";
		}
		double endl = MPI_Wtime();
		double linearTime = endl - startl;
		std::cout << std::endl;
		std::cout << "Linear Time:" << linearTime << std::endl;


		if (id == 0)
		{
			std::cout << std::endl;
			for (int i = 0; i < N; ++i)
				std::cout << vect[i] << "\t";
		}
		std::cout << std::endl;
		std::cout << "Parallel Time:" << parallelTime << std::endl;

		
	}
	
	MPI_Finalize(); // ���������� ���������� MPI-���������

	return 0;
}

