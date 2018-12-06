#include "mpi.h"
#include<iostream>
#include<string>
#include<algorithm>
typedef unsigned int uint;

void reset(uint *counter) {

	for (int i = 0; i < 256; ++i) {
		counter[i] = 0;
	}
}

void createMas(uint* mas, int size) {
	for (uint i = 0; i < size; ++i) {
		mas[i] = rand() * rand();
	}
}
void createMas(uint* mas, uint* tmas, int size) {
	for (uint i = 0; i < size; ++i) {
		tmas[i] = mas[i] = rand() * rand();

	}
}

void printMas(uint* mas, int size) {
	for (uint i = 0; i < size; ++i) {
		std::cout << mas[i] << " ";
	}
	std::cout << std::endl;
	std::cout << std::endl;
}

void radix(uint* mas, uint* tempMas, int byte, int length) {

	unsigned char *c = (unsigned char*)mas;
	uint *counter = new uint[256];
	uint *offset = new uint[256];
	offset[0] = 0;
	reset(counter);

	for (int i = 0; i < length; ++i) {
		counter[c[i * 4 + byte]]++;
	}

	for (int i = 1; i < 256; ++i) {
		offset[i] = counter[i - 1] + offset[i - 1];
	}

	for (int i = 0; i < length; ++i) {
		tempMas[offset[c[i * 4 + byte]]++] = mas[i];
	}
}

void sort(uint* mas, uint* tempMas, int length) {
	radix(mas, tempMas, 0, length);
	radix(tempMas, mas, 1, length);
	radix(mas, tempMas, 2, length);
	radix(tempMas, mas, 3, length);
}

bool sravn(uint* mas, uint* tmas, int size) {
	for (int i = 0; i < size; i++) {
		if (mas[i] != tmas[i]) {
			return false;
		}
	}
	return true;
}

void main() {

	int procSize, procRank;

	MPI_Init(nullptr, nullptr);
	MPI_Comm_size(MPI_COMM_WORLD, &procSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

	int ROOT = 0;
	const int size = 100000;
	uint* mas = nullptr;

	if (procRank == ROOT) {
		 mas = new uint[size];
		 createMas(mas, size);

		/* проверка поразрядной сортировки

		
		uint* mas = new uint[size];
		uint* tempMas = new uint[size];
		createMas(mas, size);
		
		uint* tmas = new uint[size];
		createMas(mas, tmas, size);
		double time = MPI_Wtime();
		sort(mas, tempMas, size);
		time = MPI_Wtime() - time;
		std::cout << time << std::endl;

	    time = MPI_Wtime();
		std::sort(tmas, tmas + size);
		time = MPI_Wtime() - time;
		std::cout << time << std::endl;

		std::cout << sravn(mas, tmas, size)<<std::endl;*/
		
		 
	}
	int* dis = new int[procSize];
	int* scounts = new int[procSize];
	

	scounts[0] = size / procSize + size % procSize;
	dis[0] = 0;

	for (int i = 1; i < procSize; i++) {
		scounts[i] = size / procSize;
		dis[i] = procRank * size / procSize + size % procSize;
	}

	uint* tempMas1 = new uint[scounts[procRank]];
	uint* tempMas2 =  new uint[scounts[procRank]];
	
	MPI_Scatterv(mas, scounts,dis, MPI_UNSIGNED, tempMas1, scounts[procRank], MPI_UNSIGNED, ROOT, MPI_COMM_WORLD);

	double time = MPI_Wtime();
	sort(tempMas1, tempMas2, scounts[procRank]);
	time = MPI_Wtime() - time;
	std::cout << time << std::endl;

	if (procRank % 2 != 0) {
		MPI_Send(tempMas1, scounts[procRank], MPI_UNSIGNED, procRank - 1,0,MPI_COMM_WORLD);
	}
	else {
		
	}
	
	int n = procSize;
	/*while (n > 1)

	{

		n = n / 2 + n % 2;

		if ((procID - m) % (2 * m) == 0)

		{

			MPI_Send(&k, 1, MPI_INT, procID - m, 0, MPI_COMM_WORLD);

			MPI_Send(x, k, MPI_DOUBLE, procID - m, 0, MPI_COMM_WORLD);

		}

		if ((procID % (2 * m) == 0) && (ProcSize - procID > m))

		{

			MPI_Status status;

			int k1;

			MPI_Recv(&k1, 1, MPI_INT, procID + m, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

			double *y = new double[k + k1];

			MPI_Recv(y, k1, MPI_DOUBLE, procID + m, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

			for (int i = 0; i < k; i++)

				y[i + k1] = x[i];

			bond(y, 0, k1 - 1, k + k1 - 1);

			x = new double[k1 + k];

			for (int i = 0; i < k + k1; i++)

				x[i] = y[i];

			k = k + k1;

		}

		m = 2 * m;

	}*/

	MPI_Finalize();
}