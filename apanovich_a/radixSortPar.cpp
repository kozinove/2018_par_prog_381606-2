#include <iostream> 
#include <ctime>  
#include <omp.h>

#define MAX_SHOW_SIZE 100

using namespace::std;

double* Create_and_init_arr(int size_arr)
{
	if (size_arr < 1)
		return NULL;

	double* arr;

	arr = new double[size_arr];
	srand((unsigned)time(NULL));

	for (int i = 0; i < size_arr; i++)
		arr[i] = rand() % 100 - 50 + (double)(rand()) / RAND_MAX + 0.00000001; // [-50, 51] - �������������� �����

	return arr;
}

// ���������� ������
void Show_arr(double* arr, int size_arr)
{
	if (arr == NULL || size_arr < 1)
		return;
	
	cout.precision(9);
		for (int i = 0; i < size_arr; i++)
			cout << arr[i] << " ";
		cout << endl;
	
}

// �������� � ��������
void Swap(int& arr_el_1, int& arr_el_2)
{
	int tmp = arr_el_1;
	arr_el_1 = arr_el_2;
	arr_el_2 = tmp;
}




// ���������� ��������� ��� ���� double �� i-�� ����� ��� ������������� �����
void CountingSortPlus(double* arr_inp, double* arr_out, int size_arr, int byte_num)
{
	// ���� ���������:
	// ������ ���� ����� ����� 256 ��������� ��������� (�������� ����� �� 0 �� 255)
	// ����� ���� ����� � � � � �������� ��������� s_A � s_B ��������������
	// ���� A ������ ����� B, ���� s_A > s_B
	unsigned char* mas = (unsigned char*)arr_inp; // ��� double �������� ������������� � char - � 256 ��������� �����

	int counter[256];// �������� 256 ��������� ��������� ������ �����
	int offset;

	memset(counter, 0, sizeof(int) * 256); // ��������� ������ 256 ���� ������ (�.�. int �������� 4 �����, �� ����� 256 * 4 ���� ���������)

	// ����� � mas ���������� �������� ������������ (8 * i) ���� �� byte_num ���� (�������������� �������� ������ ��� ����)
	// 8 * i - �������� i-�� ������� ������� mas (�� ���� ��� ������� ����), � ����� � ������� ���������� byte_num �� ��������� ������� �����, �� �������� ������������� �� ������ ������
	// mas ���������� ������ � ����� �� 0 �� 255 (�� ���� ����� ��������� �����)
	for (int i = 0; i < size_arr; i++)
		counter[mas[8 * i + byte_num]]++; // counter ����������, ������� ����� ���� double �������� ������������ ������

	// ������ ���� ����� ��������� ����� byte_num, ������� ������������ � �����-���� ��������� double (���������, ������� �� ���� ����������, �� ������ ������ ��� �� ���������)
	int j = 0;
	for (j; j < 256; j++)
		if (counter[j] != 0)
			break;

	offset = counter[j];// ������ offset ����������, ������� ������� ��������� � ������������ ������ (����� ����������, ������� ����� ������� arr_out ����� ��� �����,
	// ���������� ���� � ������� ��������� j)
	counter[j] = 0;// ��� ������������� �������� ���������, ���������� ���� � ������� ��������� j. ������ ����� �������� ����� ����� "���������� ����" � ����� �������� � ������ ������� arr_out
	j++;

	// ����� ������� �������� � ���������� �� � counter
	for (j; j < 256; j++)
	{
		int tmp = counter[j];
		counter[j] = offset;
		offset += tmp;
	}

	for (int i = 0; i < size_arr; i++)
	{
		arr_out[counter[mas[8 * i + byte_num]]] = arr_inp[i];// counter �������� ��� ����������� ���������� �� ����������� ������������ ���������
		counter[mas[8 * i + byte_num]]++;// ����������� �������� �� 1 ��� ��������, ����������� ���� � ������� ��������� mas[8 * i + byte_num] (����� ��������� ��� �������� � ������ ������� arr_out)
	}


}

// ���������� ��������� ��� ���� double �� i-�� ����� ��� ������������� �����
void CountingSortMinus(double* arr_inp, double* arr_out, int size_arr, int byte_num)
{
	// ���� ���������:
	// ������ ���� ����� ����� 256 ��������� ��������� (�������� ����� �� 0 �� 255)
	// ����� ���� ����� � � � � �������� ��������� s_A � s_B ��������������
	// ���� A ������ ����� B, ���� s_A < s_B
	unsigned char* mas = (unsigned char*)arr_inp; // ��� double �������� ������������� � char - � 256 ��������� �����

	int counter[256];// �������� 256 ��������� ��������� ������ �����
	int offset;

	memset(counter, 0, sizeof(int) * 256); // ��������� ������ 256 ���� ������ (�.�. int �������� 4 �����, �� ����� 256 * 4 ���� ���������)

	// ����� � mas ���������� �������� ������������ (8 * i) ���� �� byte_num ���� (�������������� �������� ������ ��� ����)
	// 8 * i - �������� i-�� ������� ������� mas (�� ���� ��� ������� ����), � ����� � ������� ���������� byte_num �� ��������� ������� �����, �� �������� ������������� �� ������ ������
	// mas ���������� ������ � ����� �� 0 �� 255 (�� ���� ����� ��������� �����)
	for (int i = 0; i < size_arr; i++)
		counter[mas[8 * i + byte_num]]++; // counter ����������, ������� ����� ���� double �������� ������������ ������

	// ������ ���� ����� ��������� ����� byte_num, ������� ������������ � �����-���� ��������� double (���������, ������� �� ���� ����������, �� ������ ������ ��� �� ���������)
	int j = 255;
	for (j; j >= 0; j--)
		if (counter[j] != 0)
			break;

	offset = counter[j];// ������ offset ����������, ������� ������� ��������� � ������������ ������ (����� ����������, ������� ����� ������� arr_out ����� ��� �����,
	// ���������� ���� � ������� ��������� j)
	counter[j] = 0;// ��� ������������� �������� ���������, ���������� ���� � ������� ��������� j. ������ ����� �������� ����� ����� "���������� ����" � ����� �������� � ������ ������� arr_out
	j--;

	// ����� ������� �������� � ���������� �� � counter
	for (j; j >= 0; j--)
	{
		int tmp = counter[j];
		counter[j] = offset;
		offset += tmp;
	}

	for (int i = 0; i < size_arr; i++)
	{
		arr_out[counter[mas[8 * i + byte_num]]] = arr_inp[i];// counter �������� ��� ����������� ���������� �� ����������� ������������ ���������
		counter[mas[8 * i + byte_num]]++;// ����������� �������� �� 1 ��� ��������, ����������� ���� � ������� ��������� mas[8 * i + byte_num] (����� ��������� ��� �������� � ������ ������� arr_out)
	}

}

void LSDSortDouble(double* arr_inp, int size_arr)
{
	double* arr_inp_plus;
	double* arr_inp_minus;
	double* arr_out_plus;
	double* arr_out_minus;
	int size_arr_plus = 0,
		size_arr_minus = 0;

	int counter_arr_plus = 0,
		counter_arr_minus = 0;

	for (int i = 0; i < size_arr; i++)// ���������� ����� ������������� � ������������� ���������
		if (arr_inp[i] > 0)
			size_arr_plus++;
		else
			size_arr_minus++;

	arr_inp_plus = new double[size_arr_plus];
	arr_inp_minus = new double[size_arr_minus];
	arr_out_plus = new double[size_arr_plus];
	arr_out_minus = new double[size_arr_minus];

	// ��������� + � - �������� � �������������� �������
	for (int i = 0; i < size_arr; i++)
		if (arr_inp[i] > 0)
			arr_inp_plus[counter_arr_plus++] = arr_inp[i];
		else
			arr_inp_minus[counter_arr_minus++] = arr_inp[i];

	// ��������� ������������� ������
	if (size_arr_plus > 0)
	{
		CountingSortPlus(arr_inp_plus, arr_out_plus, size_arr_plus, 0);
		CountingSortPlus(arr_out_plus, arr_inp_plus, size_arr_plus, 1);
		CountingSortPlus(arr_inp_plus, arr_out_plus, size_arr_plus, 2);
		CountingSortPlus(arr_out_plus, arr_inp_plus, size_arr_plus, 3);
		CountingSortPlus(arr_inp_plus, arr_out_plus, size_arr_plus, 4);
		CountingSortPlus(arr_out_plus, arr_inp_plus, size_arr_plus, 5);
		CountingSortPlus(arr_inp_plus, arr_out_plus, size_arr_plus, 6);
		CountingSortPlus(arr_out_plus, arr_inp_plus, size_arr_plus, 7);
	}

	// ��������� ������������� ������
	if (size_arr_minus > 0)
	{
		CountingSortMinus(arr_inp_minus, arr_out_minus, size_arr_minus, 0);
		CountingSortMinus(arr_out_minus, arr_inp_minus, size_arr_minus, 1);
		CountingSortMinus(arr_inp_minus, arr_out_minus, size_arr_minus, 2);
		CountingSortMinus(arr_out_minus, arr_inp_minus, size_arr_minus, 3);
		CountingSortMinus(arr_inp_minus, arr_out_minus, size_arr_minus, 4);
		CountingSortMinus(arr_out_minus, arr_inp_minus, size_arr_minus, 5);
		CountingSortMinus(arr_inp_minus, arr_out_minus, size_arr_minus, 6);
		CountingSortMinus(arr_out_minus, arr_inp_minus, size_arr_minus, 7);
	}

	// �������
	for (int i = 0; i < size_arr_minus; i++)
		arr_inp[i] = arr_inp_minus[i];

	for (int i = 0; i < size_arr_plus; i++)
		arr_inp[i + size_arr_minus] = arr_inp_plus[i];

}
void merge(double* mas, int sizel, int sizer, int sizeR, int offset) {
	
	int size;

	if (omp_get_thread_num() == 0)
		if (sizeR <= sizel + sizer)
			size = sizeR;
		else
			size = sizel + sizer;
	else
		if (sizeR <= sizel + sizer)
			size = sizeR;
		else if (sizeR <= (offset + sizel + sizer) && omp_get_thread_num() == (omp_get_num_threads() - 1))
			size = sizeR - offset;
		else
			size = sizel + sizer;

	double* tempMas = new double[size];
	int i = 0, j = sizel, k = 0;
	
	while (i != sizel && j != size) {
		if (mas[i] <= mas[j]) {
			tempMas[k] = mas[i];
			++i;
			++k;
		}
		else {
			tempMas[k] = mas[j];
			++j;
			++k;
		}
	
	}

	if (i < sizel) {
		for (; i < sizel; ++i) {
			tempMas[k] = mas[i];
			++k;
		}
	}
	if (j < size) {
		for (; j < size; ++j) {
			tempMas[k] = mas[j];
			++k;
		}
	}
	for (i = 0; i < size; ++i) {
		mas[i] = tempMas[i];
	}

	delete[] tempMas;
}


int main(int argc, char* argv[])
{

	int size_arr = 0;
	
	
	double* test_arr_seq_radix = NULL;
	cout << "Enter the size:" << endl;
	cin >> size_arr;
	int n;
	std::cout << "Enter number of threads: ";
	std::cin >> n;
	int tail = size_arr % n;
	test_arr_seq_radix = Create_and_init_arr(size_arr);
	if (size_arr < MAX_SHOW_SIZE)
		Show_arr(test_arr_seq_radix, size_arr);
	double start_time = omp_get_wtime();
	omp_set_num_threads(n);
#pragma omp parallel
	{
		if (omp_get_thread_num() == 0) {
			LSDSortDouble(test_arr_seq_radix, size_arr / n + tail);
		}
		else {
			LSDSortDouble(test_arr_seq_radix + tail + omp_get_thread_num()*(size_arr / n), size_arr / n);
		}
#pragma omp barrier
	}
/*
	LSDSortDouble(test_arr_seq_radix, size_arr);*/
	int j = 1;
	int k = n / 2;
	int d = n;
	while (k != 0) {
		omp_set_num_threads(k);
#pragma omp parallel 
		{
			
			if (omp_get_thread_num() == 0) {
				
				merge(test_arr_seq_radix, (size_arr / n) * j + tail, (size_arr / n) * j, size_arr, 2 * omp_get_thread_num()*(size_arr / n) * j + tail);
				
			}
			else {
				merge(test_arr_seq_radix + 2 * omp_get_thread_num()*(size_arr / n) * j + tail, (size_arr / n) * j, (size_arr / n) * j, size_arr, 2 * omp_get_thread_num()*(size_arr / n) * j + tail);

			}
#pragma omp barrier
			if (omp_get_thread_num() == 0) {
				j *= 2;
				d = d / 2 + d % 2;
				k = d / 2;
			
			}
		}

	}
	double end_time = omp_get_wtime();
	if (size_arr < MAX_SHOW_SIZE)
	{
		cout << "Array sorted by parallel radix :" << endl;
		Show_arr(test_arr_seq_radix, size_arr);
	}
	std::cout << "Total time parallel version: " << (end_time - start_time) << "ms\n";
	
	//��������
	for(int i = 0; i<size_arr; i++)
		if(test_arr_seq_radix[i-1]<=test_arr_seq_radix[i])
		{
			cout<<"The result is correct!"<<endl;
			break;
		}
	return 0;
}