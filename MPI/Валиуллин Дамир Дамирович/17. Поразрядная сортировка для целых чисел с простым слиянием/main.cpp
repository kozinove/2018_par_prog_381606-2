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

void sort(uint* mas,uint* tempMas, int length) {
	
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

bool PowerOfTwo(int Value)
{
	int InitValue = 1;
	while (InitValue < Value)
		InitValue *= 2;
	if (InitValue == Value)
		return true;
	return false;
}

int main() {
	srand(time(NULL));
	int procSize, procRank;

	MPI_Init(nullptr, nullptr);
	MPI_Comm_size(MPI_COMM_WORLD, &procSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &procRank);
	int ROOT = 0;

	if (!PowerOfTwo(procSize)) {
		if (procRank == ROOT) {
			std::cout << "Size of process is not power of two!!!" << std::endl;
		}
		MPI_Finalize();
		return 0;
	}
	
	const int size = 10000000;
	uint* mas = nullptr;
	uint* tmas = nullptr;

	double gltime;
	double time;
	int length;

	int* dis = new int[procSize];
	int* scounts = new int[procSize];

	dis[0] = 0;
	scounts[0] = size / procSize + size % procSize;

	for (int i = 1; i < procSize; i++) {
		dis[i] = i * size / procSize + size % procSize;
		scounts[i] = size / procSize;
	}

	uint* result = new uint[size];
	uint* tempMas = new uint[size];
	uint* tempMas2 = new uint[size];
	if (procRank == ROOT) {
		mas = new uint[size];
		tmas = new uint[size];

		createMas(mas, tmas, size);
	}

	gltime = MPI_Wtime();

	MPI_Scatterv(mas, scounts, dis, MPI_UNSIGNED, result, scounts[procRank], MPI_UNSIGNED, ROOT, MPI_COMM_WORLD);

	time = MPI_Wtime();
	sort(result, tempMas, scounts[procRank]);
	time = MPI_Wtime() - time;
	std::cout << "rank:" << procRank << " sorted for: " <<time << " sec" << std::endl;

	int n = procSize;
	int m = 1;
	
	time = MPI_Wtime();
	while (n > 1) {
		n = n / 2 + n % 2;

		if ((procRank - m) % (2 * m) == 0) {

			MPI_Send(&scounts[procRank], 1, MPI_UNSIGNED, procRank - m, 0, MPI_COMM_WORLD);
			MPI_Send(result, scounts[procRank], MPI_UNSIGNED, procRank - m, 0, MPI_COMM_WORLD);

			delete[] result;
			delete[] tempMas;
			delete[] tempMas2;
			MPI_Finalize();
			return 1;
		}
		 if ((procRank % (2 * m) == 0) && (procSize - procRank > m)) {
			MPI_Status status;

			MPI_Recv(&length, 1, MPI_UNSIGNED, procRank + m, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			
			std::cout << "rank:" << procRank <<" get:" << scounts[procRank + m] << " elements, from:" <<status.MPI_SOURCE<<  std::endl;
			MPI_Recv(tempMas, length, MPI_UNSIGNED, procRank + m, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

			bond(result, scounts[procRank], tempMas, length, tempMas2);
	
			
			
				for (int i = 0; i < length + scounts[procRank]; i++) {
					result[i] = tempMas2[i];
				}
		


			scounts[procRank] += length;
		}
		m *= 2;
	}
	time = MPI_Wtime() - time;

	if (procRank == ROOT) {
		for (int i = 0; i < size; ++i) {
			mas[i] = result[i];
		}

		gltime = MPI_Wtime() - gltime;
		delete[] result;

		std::cout << "Time of merge on main process: "<<time << std::endl;

		std::cout << std::endl;
		time = MPI_Wtime();

		sort(tmas, tempMas, size);
		//std::sort(tmas,tmas+size);
		time = MPI_Wtime() - time;
		std::cout << "Sort on the " <<1<<" process: "<< time << " sec" << std::endl;
		std::cout << "Sort on the "<< procSize << " process: " << gltime << " sec" << std::endl;

		std::cout << "Acceleration: " << time / gltime << std::endl;
		if (sravn(mas, tmas, size)) {
			std::cout << "ALL RIGHT" << std::endl;
		}
		else {
			std::cout << "ERROR" << std::endl;
		}
		std::cout << "ROOT size: " << scounts[0] << std::endl;
		delete[] mas;
		delete[] tempMas;
		delete[] tempMas2;
	}

	MPI_Finalize();
	return 1;
}