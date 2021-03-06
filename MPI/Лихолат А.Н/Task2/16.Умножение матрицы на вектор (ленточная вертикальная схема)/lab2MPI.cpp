#include "pch.h"
#include <iostream>
#include <mpi.h>
#include <ctime>

//умножение матрицы на вектор: ленточная вертикальная схема
//Матрица делится по столбцам, вектор в этом случае также делится между процессами.

double* genVec(const int n) {
	double* vec = new double[n];
	for (int i = 0; i < n; i++)
		vec[i] = rand() % 100 - 50;
	return vec;
}

double** genMatr(const int m, const int n)
{
	double** matr = 0;
	matr = new double*[m];
	for (int i = 0; i < m; i++)
		matr[i] = new double[n];

	for (int i = 0; i < m; i++)
		for (int j = 0; j < n; j++)
			matr[i][j] = rand() % 100 - 50;
	return matr;
}

double* multiply(int sendNum, int sendNumV, double* matr, double* vec, int size) //умножение
{
	double *intrmRes = new double[sendNum];
	double *res = new double[size];
	for (int i = 0; i < size; i++)
		res[i] = 0;

	int tmp = 0;
	for (int i = 0; i < sendNumV; i++)		//умножение столбцов на соответствующие элементы вектора
		for (int j = 0; j < size; j++){
			intrmRes[tmp] = matr[tmp] * vec[i];
			tmp++;
		}

	if (sendNum > size){					//складываем элементы в соответствующих строках, если процесс обрабатывает несколько столбцов
		for (int i = 0; i < size; i++)
			for (int j = 0; j < sendNumV; j++)
				res[i] += intrmRes[i + j * size];
		return res;
	}
	return intrmRes;
}

bool isEqual(double *v1, double *v2, int size) {
	for (int i = 0; i < size; i++)
		if (v1[i] != v2[i])
			return false;
	return true;
}

int main(int argc, char* argv[]) {

	int errCode;

	errCode = MPI_Init(&argc, &argv);

	if (errCode != 0) {
		return errCode;
	}

	double *vec = NULL;
	double **matr = NULL;
	double *matr_as_vec = NULL;
	int vecSize = 10;
	int mSize1 = vecSize; // число строк
	int mSize2 = vecSize; // число столбцов = vecSize
	double *intrmRes = NULL;
	double *res = NULL;
	double *lineRes = NULL;

	int procNum, procRank;
	int *sendNum; //кол-во элементов в блоке
	int *displ;   //смещение блока относительно начала
	int *sendNumV;
	int *displV;
	int step;
	int balance;
	int tmp, tmpV;
	double *recBuf;
	double *recBufV;
	double *gatBuf = NULL;
	double lineSt = 0;
	double lineEt = 0;
	double paralSt = 0;
	double paralEt = 0;

	MPI_Comm_size(MPI_COMM_WORLD, &procNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

	if (procRank == 0) {

		vec = genVec(vecSize);
		
		if (vecSize <= 10) {
			for (int i = 0; i < vecSize; i++)
				std::cout << vec[i] << "\n";
			std::cout << '\n';
		}
			
		matr = genMatr(mSize1, mSize2);

		if (mSize2 <= 10) {
			for (int i = 0; i < mSize1; i++) {
				for (int j = 0; j < mSize2; j++)
					std::cout << matr[i][j] << " ";
				std::cout << '\n';
			}
			std::cout << '\n';
		}

	//считаем без распараллеливания:
		lineRes = new double[mSize1];
		for (int i = 0; i < mSize1; i++)
			lineRes[i] = 0;

		lineSt = MPI_Wtime();

		for (int i = 0; i < mSize1; i++)
			for (int j = 0; j < mSize2; j++)
				lineRes[i] += matr[i][j] * vec[j];
		
		if (vecSize <= 10) {
			for (int i = 0; i < mSize1; i++)
				std::cout << "linear_res[ " << i << " ] " << "= " << lineRes[i] << '\n';
			std::cout << '\n';
		}

		lineEt = MPI_Wtime();
	//______________________________________________________________________________________________________________

		paralSt = MPI_Wtime();

		//представим матрицу как один вектор (ленточное разбиение по столбцам)
		matr_as_vec = new double[(mSize1*mSize2)]; 
		for (int i = 0; i < (mSize1*mSize2); i++)
			matr_as_vec[i] = 0;

		for (int i = 0; i < mSize2; i++) //по столбцам
			for (int j = 0; j < mSize1; j++)  //по строкам
				matr_as_vec[i*mSize1 + j] = matr[j][i];

		gatBuf = new double[mSize1*procNum];
		for (int i = 0; i < mSize1*procNum; i++)
			gatBuf[i] = 0;

		res = new double[mSize1];
		for (int i = 0; i < mSize1; i++)
			res[i] = 0;

	}

	sendNum = new int[procNum];		 //кол-во элементов в блоке
	displ = new int[procNum];		 //смещение блока относительно начала
	sendNumV = new int[procNum];	 //кол-во элементов в блоке - для вектора
	displV = new int[procNum];		 //смещение блока относительно начала - для вектора
	step = mSize2 / procNum;
	balance = mSize2 % procNum;
	tmp = 0;
	tmpV = 0;

	for (int i = 0; i < procNum; i++) {
		displ[i] = tmp;
		sendNum[i] = step*mSize1;
		displV[i] = tmpV;
		sendNumV[i] = step;
		if (balance != 0) {
			sendNum[i] = (step + 1) * mSize1;
			sendNumV[i]++;
			balance--;
		}
		tmp += sendNum[i];
		tmpV += sendNumV[i];
	}

	recBuf = new double[sendNum[procRank]];
	recBufV = new double[sendNumV[procRank]];

	MPI_Scatterv(matr_as_vec , sendNum, displ, MPI_DOUBLE, recBuf, sendNum[procRank], MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Scatterv(vec, sendNumV, displV, MPI_DOUBLE, recBufV, sendNumV[procRank], MPI_DOUBLE, 0, MPI_COMM_WORLD);

	intrmRes = new double[mSize1];
	for (int i = 0; i < mSize1; i++)
		intrmRes[i] = 0;
	intrmRes = multiply(sendNum[procRank], sendNumV[procRank], recBuf, recBufV, mSize1);

	/*gatBuf = new double[mSize1*procNum];
	for (int i = 0; i < mSize1*procNum; i++)
		gatBuf[i] = 0;*/
	MPI_Gather(intrmRes, mSize1, MPI_DOUBLE, gatBuf, mSize1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	if (procRank == 0) {
		for (int i = 0; i < mSize1; i++)
			for (int j = 0; j < procNum; j++)
				res[i] += gatBuf[i + j * mSize1];
	
		if (vecSize <= 10) {
			for (int i = 0; i < mSize1; i++)
				std::cout << "parallel_res[ " << i << " ] " << "= " << res[i] << '\n';
			std::cout << '\n';
			
		}

		paralEt =  MPI_Wtime();

		if (isEqual(lineRes, res, vecSize)) {
			std::cout << "linear and parallel results are equal" << '\n';
			std::cout << "linear time = " << lineEt - lineSt << '\n';
			std::cout << "parallel time = " << paralEt - paralSt << '\n';
		}
		else
			std::cout << "something is wrong" << '\n';
	}

	MPI_Finalize();
	return 0;
}


//cd/d C:\Users\User\source\repos\lab2MPI\Debug
//"C:\Program Files\Microsoft MPI\Bin\mpiexec.exe" -n 4 lab2MPI.exe
