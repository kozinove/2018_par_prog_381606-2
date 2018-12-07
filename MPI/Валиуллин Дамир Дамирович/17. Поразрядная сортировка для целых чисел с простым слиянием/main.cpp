#include "mpi.h"
#include<iostream>
#include<string>
#include<algorithm>
#include<time.h>
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

void sort(uint* mas, int length) {
	uint* tempMas = new uint[length];
	radix(mas, tempMas, 0, length);
	radix(tempMas, mas, 1, length);
	radix(mas, tempMas, 2, length);
	radix(tempMas, mas, 3, length);
	delete[] tempMas;
}

bool sravn(uint* mas, uint* tmas, int size) {
	for (int i = 0; i < size; i++) {

		if (mas[i] != tmas[i]) {
			return false;
		}
	}
	return true;
}

void bond(uint* mas1, int size1, uint* mas2, int size2, uint* result) {
	int i = 0, j = 0, k = 0;
	while (i < size1 && j < size2) {

		if (mas1[i] < mas2[j]) {
			result[k] = mas1[i]; i++;
		}
		else {
			result[k] = mas2[j];
			j++;
		}
		k++;
	}

	while (i < size1) {
		result[k] = mas1[i];
		k++; i++;
	}

	while (j < size2) {
		result[k] = mas2[j];
		k++;
		j++;
	}
}

int main() {
	srand(time(NULL));
	int procSize, procRank;

	MPI_Init(nullptr, nullptr);
	MPI_Comm_size(MPI_COMM_WORLD, &procSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

	int ROOT = 0;
	const int size = 10000000;
	uint* mas = nullptr;
	uint* tmas = nullptr;
	double gltime;
	if (procRank == ROOT) {
		mas = new uint[size];
		tmas = new uint[size];
		createMas(mas, tmas, size);

		gltime = MPI_Wtime();
	}

	int* dis = new int[procSize];
	int* scounts = new int[procSize];


	dis[0] = 0;
	scounts[0] = size / procSize + size % procSize;

	for (int i = 1; i < procSize; i++) {
		dis[i] = i * size / procSize + size % procSize;
		scounts[i] = size / procSize ;
	}

	uint* result = new uint[scounts[procRank]];

	MPI_Scatterv(mas, scounts, dis, MPI_UNSIGNED, result, scounts[procRank], MPI_UNSIGNED, ROOT, MPI_COMM_WORLD);

	double time = MPI_Wtime();
	sort(result, scounts[procRank]);
	time = MPI_Wtime() - time;
	
	int n = procSize;
	int m = 1;

	while (n > 1) {
		n = n / 2 + n % 2;

		if ((procRank - m) % (2 * m) == 0) {

			MPI_Send(&scounts[procRank], 1, MPI_UNSIGNED, procRank - m, 0, MPI_COMM_WORLD);
			MPI_Send(result, scounts[procRank], MPI_UNSIGNED, procRank - m, 0, MPI_COMM_WORLD);

			delete[] result;
			MPI_Finalize();
			return 1;
		}
		if ((procRank % (2 * m) == 0) && (procSize - procRank > m)) {
			MPI_Status status;
			int length;

			MPI_Recv(&length, 1, MPI_UNSIGNED, procRank + m, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

			uint *y = new uint[length];
			std::cout << "rank:" << procRank <<" get:" << scounts[procRank + m] << " elements, from:" <<status.MPI_SOURCE<<  std::endl;
			MPI_Recv(y, length, MPI_UNSIGNED, procRank + m, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

			uint* tempMas3 = new uint[length + scounts[procRank]];
			bond(result, scounts[procRank], y, length, tempMas3);

			delete[] result;
			delete[] y;
	
			result = new uint[length + scounts[procRank]];
			for (int i = 0; i < length + scounts[procRank]; i++) {
				result[i] = tempMas3[i];
			}
			delete[] tempMas3;
			scounts[procRank] += length;
		}
		m *= 2;
	}


	if (procRank == ROOT) {
		gltime = MPI_Wtime() - gltime;

		std::cout << "Starting std::sort..." << std::endl;
		time = MPI_Wtime();
		sort(tmas, size);
		std::sort(tmas, tmas + size);
		time = MPI_Wtime() - time;
		std::cout << "Sort on the " <<1<<" process: "<< time << " sec" << std::endl;
		std::cout << "Sort on the "<< procSize << " process: " << gltime << " sec" << std::endl;

		std::cout << "Acceleration: " << time / gltime << std::endl;
		if (sravn(result, tmas, size)) {
			std::cout << "ALL RIGHT" << std::endl;
		}
		else {
			std::cout << "ERROR" << std::endl;
		}
		std::cout << "ROOT size: " << scounts[0] << std::endl;
		
	}

	MPI_Finalize();
	return 1;
}