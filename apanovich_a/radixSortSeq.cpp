#include <iostream> 
#include <ctime>  

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
		arr[i] = rand() % 100 - 50 + (double)(rand()) / RAND_MAX + 0.00000001; // [-50, 51] - действительные числа

	return arr;
}

// Отобразить массив
void Show_arr(double* arr, int size_arr)
{
	if (arr == NULL || size_arr < 1)
		return;
	
	cout.precision(9);
		for (int i = 0; i < size_arr; i++)
			cout << arr[i] << " ";
		cout << endl;
	
}

// Сравнить и обменять
void Swap(int& arr_el_1, int& arr_el_2)
{
	int tmp = arr_el_1;
	arr_el_1 = arr_el_2;
	arr_el_2 = tmp;
}




// Сортировка подсчетом для типа double по i-му байту для положительных чисел
void CountingSortPlus(double* arr_inp, double* arr_out, int size_arr, int byte_num)
{
	// Идея следующая:
	// Каждый байт может иметь 256 различных состояний (диапазон чисел от 0 до 255)
	// Пусть есть байты А и В с номерами состояний s_A и s_B соответственно
	// Байт A больше байта B, если s_A > s_B
	unsigned char* mas = (unsigned char*)arr_inp; // Все double элементы преобразуются в char - в 256 разрядные числа

	int counter[256];// Возможно 256 различных состояний одного байта
	int offset;

	memset(counter, 0, sizeof(int) * 256); // Заполнить первые 256 байт нулями (т.к. int содержит 4 байта, то нужно 256 * 4 байт заполнить)

	// Здесь в mas происходит смещение относительно (8 * i) байт на byte_num байт (соответственно выделяем нужный нам байт)
	// 8 * i - выбираем i-ый элемент массива mas (по сути его младший байт), а потом с помощью добавления byte_num мы достигаем нужного байта, по которому упорядочиваем на данный момент
	// mas возвращает символ с кодом от 0 до 255 (по сути номер состояния байта)
	for (int i = 0; i < size_arr; i++)
		counter[mas[8 * i + byte_num]]++; // counter показывает, сколько чисел типа double содержит определенный разряд

	// Теперь ищем номер состояния байта byte_num, который присутствует в каких-либо элементах double (состояния, которые не были обнаружены, на данный момент нам не интересны)
	int j = 0;
	for (j; j < 256; j++)
		if (counter[j] != 0)
			break;

	offset = counter[j];// Теперь offset показывает, сколько имеется элементов с определенным байтом (чтобы определить, сколько ячеек массива arr_out уйдет под числа,
	// содержащий байт с номером состояния j)
	counter[j] = 0;// Это характеризует смещение элементов, содержащих байт с номером состояния j. Причем такие элементы будут иметь "наименьший байт" и будут записаны в начале массива arr_out
	j++;

	// Далее считаем смещения и записываем их в counter
	for (j; j < 256; j++)
	{
		int tmp = counter[j];
		counter[j] = offset;
		offset += tmp;
	}

	for (int i = 0; i < size_arr; i++)
	{
		arr_out[counter[mas[8 * i + byte_num]]] = arr_inp[i];// counter содержит всю необходимую информацию по корректному раскидыванию элементов
		counter[mas[8 * i + byte_num]]++;// Увеличиваем смещение на 1 для элемента, содержащего байт с номером состояния mas[8 * i + byte_num] (чтобы корректно его записать в ячейку массива arr_out)
	}


}

// Сортировка подсчетом для типа double по i-му байту для отрицательных чисел
void CountingSortMinus(double* arr_inp, double* arr_out, int size_arr, int byte_num)
{
	// Идея следующая:
	// Каждый байт может иметь 256 различных состояний (диапазон чисел от 0 до 255)
	// Пусть есть байты А и В с номерами состояний s_A и s_B соответственно
	// Байт A больше байта B, если s_A < s_B
	unsigned char* mas = (unsigned char*)arr_inp; // Все double элементы преобразуются в char - в 256 разрядные числа

	int counter[256];// Возможно 256 различных состояний одного байта
	int offset;

	memset(counter, 0, sizeof(int) * 256); // Заполнить первые 256 байт нулями (т.к. int содержит 4 байта, то нужно 256 * 4 байт заполнить)

	// Здесь в mas происходит смещение относительно (8 * i) байт на byte_num байт (соответственно выделяем нужный нам байт)
	// 8 * i - выбираем i-ый элемент массива mas (по сути его младший байт), а потом с помощью добавления byte_num мы достигаем нужного байта, по которому упорядочиваем на данный момент
	// mas возвращает символ с кодом от 0 до 255 (по сути номер состояния байта)
	for (int i = 0; i < size_arr; i++)
		counter[mas[8 * i + byte_num]]++; // counter показывает, сколько чисел типа double содержит определенный разряд

	// Теперь ищем номер состояния байта byte_num, который присутствует в каких-либо элементах double (состояния, которые не были обнаружены, на данный момент нам не интересны)
	int j = 255;
	for (j; j >= 0; j--)
		if (counter[j] != 0)
			break;

	offset = counter[j];// Теперь offset показывает, сколько имеется элементов с определенным байтом (чтобы определить, сколько ячеек массива arr_out уйдет под числа,
	// содержащий байт с номером состояния j)
	counter[j] = 0;// Это характеризует смещение элементов, содержащих байт с номером состояния j. Причем такие элементы будут иметь "наименьший байт" и будут записаны в начале массива arr_out
	j--;

	// Далее считаем смещения и записываем их в counter
	for (j; j >= 0; j--)
	{
		int tmp = counter[j];
		counter[j] = offset;
		offset += tmp;
	}

	for (int i = 0; i < size_arr; i++)
	{
		arr_out[counter[mas[8 * i + byte_num]]] = arr_inp[i];// counter содержит всю необходимую информацию по корректному раскидыванию элементов
		counter[mas[8 * i + byte_num]]++;// Увеличиваем смещение на 1 для элемента, содержащего байт с номером состояния mas[8 * i + byte_num] (чтобы корректно его записать в ячейку массива arr_out)
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

	for (int i = 0; i < size_arr; i++)// Подсчитаем число положительных и отрицательных элементов
		if (arr_inp[i] > 0)
			size_arr_plus++;
		else
			size_arr_minus++;

	arr_inp_plus = new double[size_arr_plus];
	arr_inp_minus = new double[size_arr_minus];
	arr_out_plus = new double[size_arr_plus];
	arr_out_minus = new double[size_arr_minus];

	// Раскидаем + и - элементы в соответсвующие массивы
	for (int i = 0; i < size_arr; i++)
		if (arr_inp[i] > 0)
			arr_inp_plus[counter_arr_plus++] = arr_inp[i];
		else
			arr_inp_minus[counter_arr_minus++] = arr_inp[i];

	// Сортируем положительный массив
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

	// Сортирует отрицательный массив
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

	// Слияние
	for (int i = 0; i < size_arr_minus; i++)
		arr_inp[i] = arr_inp_minus[i];

	for (int i = 0; i < size_arr_plus; i++)
		arr_inp[i + size_arr_minus] = arr_inp_plus[i];

}


int main(int argc, char* argv[])
{

	int size_arr = 0;
	
	double* test_arr_seq_radix = NULL;
	cout << "Enter the size:" << endl;
	cin >> size_arr;
	test_arr_seq_radix = Create_and_init_arr(size_arr);
	
	if (test_arr_seq_radix == NULL)
	{
		cout << "Incorrect input data, try again";
		return 0;
	}

	if (size_arr < MAX_SHOW_SIZE)
		Show_arr(test_arr_seq_radix, size_arr);

	LSDSortDouble(test_arr_seq_radix, size_arr);
	if (size_arr < MAX_SHOW_SIZE)
	{
		cout << "Array sorted by sequence radix :" << endl;
		Show_arr(test_arr_seq_radix, size_arr);
	}
	
	//Проверка
	for(int i = 0; i<size_arr; i++)
		if(test_arr_seq_radix[i-1]<=test_arr_seq_radix[i])
		{
			cout<<"The result is correct!"<<endl;
			break;
		}
	return 0;
}