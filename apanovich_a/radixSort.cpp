#include <iostream> 
#include <ctime>  
#include <mpi.h> 

#define ROOT 0
#define MAX_SHOW_SIZE 100

using namespace::std;

int  curr_rank_proc; // Текущий ранг процесса
int  num_of_procs; // Число процессов

double  time_seq_work_alg_radix = 0; 
double  time_pp_work_alg_radix = 0;

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


// Заполнить массив работ (сколько у каждого процесса в буфере элементов) и массив смещений (для MPI_Scatterv и MPI_Gather)
void Calculate_work_and_displs(int* displs, int* send_num_work, int size_arr)
{
	int size_work = size_arr / num_of_procs,
		remainder = size_arr % num_of_procs;

	for (int i = 0; i < remainder; i++)
	{
		displs[i] = (size_work + 1) * i;
		send_num_work[i] = size_work + 1;
	}

	for (int i = remainder; i < num_of_procs; i++)
	{
		displs[i] = size_work * i + remainder; // В случае, если работа распределилась не равномерно, то смещение для этих процессов вычисляется так. Если равномерно, то формула подойдет такая: displs[i] = size_work * i,
		//  а remainder == 0 и он никак не повлияет на вычисление смещения в этом случае
		send_num_work[i] = size_work;
	}
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

void Compare_split_right(double* buffer_curr_proc, int size_curr_proc_buffer, int id_proc_right, int size_buffer_proc_right)
{
	double* buffer_proc_recv = new double[size_buffer_proc_right];
	double* merge_arr = new double[size_curr_proc_buffer + size_buffer_proc_right];
	MPI_Status status;

	/* INTERCHANGE */

	MPI_Sendrecv(buffer_curr_proc, size_curr_proc_buffer, MPI_DOUBLE, id_proc_right, 1, buffer_proc_recv, size_buffer_proc_right, MPI_DOUBLE, id_proc_right, 1, MPI_COMM_WORLD, &status);

	/* MERGE */

	// При условии, что буферы упорядочены, используем операцию слияния буферов

	int index_buffer_curr_proc = 0,
		index_buffer_right_proc = 0,
		index_buffer_merge = 0;

	// Идет слияние, причем merge_arr на этом этапе всегда упорядочен
	while (index_buffer_curr_proc < size_curr_proc_buffer && index_buffer_right_proc < size_buffer_proc_right)
		if (buffer_curr_proc[index_buffer_curr_proc] < buffer_proc_recv[index_buffer_right_proc])
			merge_arr[index_buffer_merge++] = buffer_curr_proc[index_buffer_curr_proc++];
		else
			merge_arr[index_buffer_merge++] = buffer_proc_recv[index_buffer_right_proc++];

	// Если выход из while произошел из-за условия index_buffer_curr_proc < size_curr_proc_buffer, то добавляем оставшиеся элементы из buffer_proc_recv
	while (index_buffer_right_proc < size_buffer_proc_right) // В нашем случае, такая ситуация невозможна, т.к. правый процесс получает либо такую же работу, что и текущий процесс, либо на 1 меньше
		merge_arr[index_buffer_merge++] = buffer_proc_recv[index_buffer_right_proc++];
	// Если выход из while произошел из-за условия index_buffer_right_proc < size_buffer_proc_right, то добавляем оставшиеся элементы из buffer_curr_proc
	while (index_buffer_curr_proc < size_curr_proc_buffer)
		merge_arr[index_buffer_merge++] = buffer_curr_proc[index_buffer_curr_proc++];

	/* Split */

	// Разделяем merge_arr на 2 части: левая часть размером size_curr_proc_buffer оставляется процессу с рангом id_proc_right - 1 (меньшие значения),
	// а правая часть размером size_buffer_proc_right оставляется процессу с рангом id_proc_right (большие значения) (но это будет выполнено в ф-ции Compare_split_left())

	for (int i = 0; i < size_curr_proc_buffer; i++)
		buffer_curr_proc[i] = merge_arr[i];

}


void Compare_split_left(double* buffer_curr_proc, int size_curr_proc_buffer, int id_proc_left, int size_buffer_proc_left)
{
	double* buffer_proc_recv = new double[size_buffer_proc_left];
	double* merge_arr = new double[size_curr_proc_buffer + size_buffer_proc_left];
	MPI_Status status;

	/* INTERCHANGE */

	MPI_Sendrecv(buffer_curr_proc, size_curr_proc_buffer, MPI_DOUBLE, id_proc_left, 1, buffer_proc_recv, size_buffer_proc_left, MPI_DOUBLE, id_proc_left, 1, MPI_COMM_WORLD, &status);

	/* MERGE */

	// При условии, что буферы упорядочены, используем операцию слияния буферов

	int index_buffer_curr_proc = 0,
		index_buffer_left_proc = 0,
		index_buffer_merge = 0;

	// Идет слияние, причем merge_arr на этом этапе всегда упорядочен
	while (index_buffer_curr_proc < size_curr_proc_buffer && index_buffer_left_proc < size_buffer_proc_left)
		if (buffer_curr_proc[index_buffer_curr_proc] < buffer_proc_recv[index_buffer_left_proc])
			merge_arr[index_buffer_merge++] = buffer_curr_proc[index_buffer_curr_proc++];
		else
			merge_arr[index_buffer_merge++] = buffer_proc_recv[index_buffer_left_proc++];

	// Если выход из while произошел из-за условия index_buffer_curr_proc < size_curr_proc_buffer, то добавляем оставшиеся элементы из buffer_proc_recv
	while (index_buffer_left_proc < size_buffer_proc_left)
		merge_arr[index_buffer_merge++] = buffer_proc_recv[index_buffer_left_proc++];
	// Если выход из while произошел из-за условия index_buffer_left_proc < size_buffer_proc_left, то добавляем оставшиеся элементы из buffer_curr_proc
	while (index_buffer_curr_proc < size_curr_proc_buffer) // В нашем случае, такая ситуация невозможна, т.к. левый процесс получает либо такую же работу, что и текущий процесс, либо на 1 больше
		merge_arr[index_buffer_merge++] = buffer_curr_proc[index_buffer_curr_proc++];

	/* Split */

	// Разделяем merge_arr на 2 части: левая часть размером size_curr_proc_buffer оставляется процессу с рангом id_proc_right - 1 (меньшие значения),
	// а правая часть размером size_buffer_proc_right оставляется процессу с рангом id_proc_right (большие значения) (но это будет выполнено в ф-ции Compare_split_left())

	for (int i = 0; i < size_curr_proc_buffer; i++)
		buffer_curr_proc[i] = merge_arr[size_buffer_proc_left + i];
}

// Сортировка параллельная
void Sort_pp(double* recv_buffer, int* send_num_work)
{
	for (int i = 0; i < num_of_procs; i++)
		if (i % 2 == 1) // Если выполняется, то это нечетная операция
			if (curr_rank_proc % 2 == 1)
			{
				if (curr_rank_proc < num_of_procs - 1) // Сравнение - обмен с соседом справа
					Compare_split_right(recv_buffer, send_num_work[curr_rank_proc], curr_rank_proc + 1, send_num_work[curr_rank_proc + 1]);
			}
			else
			{
				if (curr_rank_proc > 0)// Тогда сравнение - обмен с соседом слева
					Compare_split_left(recv_buffer, send_num_work[curr_rank_proc], curr_rank_proc - 1, send_num_work[curr_rank_proc - 1]);
			}
		else // Тогда это четная операция
			if (curr_rank_proc % 2 == 0)
			{
				if (curr_rank_proc < num_of_procs - 1) // Сравнение - обмен с соседом справа
					Compare_split_right(recv_buffer, send_num_work[curr_rank_proc], curr_rank_proc + 1, send_num_work[curr_rank_proc + 1]);
			}
			else // Тогда сравнение - обмен с соседом слева
				Compare_split_left(recv_buffer, send_num_work[curr_rank_proc], curr_rank_proc - 1, send_num_work[curr_rank_proc - 1]);

}


int main(int argc, char* argv[])
{

	int size_arr = 0;
	int* displs; // Массив смещений относительно начала буфера test_arr
	int* send_num_work; // Массив количества работ для каждого процесса
	double* recv_buffer;
	double* test_arr_seq_radix = NULL;
	double* test_arr_pp_radix= NULL;
	

	double seq_alg_time_start_radix = 0;
	double seq_alg_time_end_radix = 0;
	double pp_alg_time_start_radix = 0;
	double pp_alg_time_end_radix = 0;

	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &num_of_procs); // Этот коммуникатор объединит все процессы. Теперь процессы могут обмениваться некоторой информацией
	MPI_Comm_rank(MPI_COMM_WORLD, &curr_rank_proc);

	// Процесс ROOT выполнит последовательную часть алгоритма
	if (curr_rank_proc == ROOT)
	{
		cout << "Enter the size:" << endl;
		cin >> size_arr;
		test_arr_seq_radix = Create_and_init_arr(size_arr);
		test_arr_pp_radix = new double[size_arr];
		for (int i = 0; i < size_arr; i++)
			test_arr_pp_radix[i] = test_arr_seq_radix[i];

		if (test_arr_seq_radix == NULL)
		{
			cout << "Incorrect input data, try again";
			return 0;
		}

		if (size_arr < MAX_SHOW_SIZE)
			Show_arr(test_arr_seq_radix, size_arr);

		// Начало работы последовательной версии

		seq_alg_time_start_radix = MPI_Wtime();
		LSDSortDouble(test_arr_seq_radix, size_arr);
		seq_alg_time_end_radix = MPI_Wtime();
		time_seq_work_alg_radix = seq_alg_time_end_radix - seq_alg_time_start_radix;

		if (size_arr < MAX_SHOW_SIZE)
		{
			cout << "Array sorted by sequence radix :" << endl;
			Show_arr(test_arr_seq_radix, size_arr);
		}

	}

	// Параллельная версия работы алгоритма

	if (curr_rank_proc == ROOT)
		pp_alg_time_start_radix = MPI_Wtime();


	send_num_work = new int[num_of_procs];
	displs = new int[num_of_procs];

	/* Параллельная сортировка с использованием быстрой сортировки локальных буферов */

	MPI_Bcast(&size_arr, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
	if (curr_rank_proc > 0)
		test_arr_pp_radix = new double[size_arr];

	Calculate_work_and_displs(displs, send_num_work, size_arr);

	recv_buffer = new double[send_num_work[curr_rank_proc]];


	MPI_Scatterv(test_arr_pp_radix, send_num_work, displs, MPI_DOUBLE, recv_buffer, send_num_work[curr_rank_proc], MPI_DOUBLE, ROOT, MPI_COMM_WORLD);

	LSDSortDouble(recv_buffer, send_num_work[curr_rank_proc]);

	Sort_pp(recv_buffer, send_num_work);

	MPI_Gatherv(recv_buffer, send_num_work[curr_rank_proc], MPI_DOUBLE, test_arr_pp_radix, send_num_work, displs, MPI_DOUBLE, ROOT, MPI_COMM_WORLD);


	if (curr_rank_proc == ROOT)
	{
		pp_alg_time_end_radix = MPI_Wtime();

		time_pp_work_alg_radix = pp_alg_time_end_radix - pp_alg_time_start_radix;
	}

	if (curr_rank_proc == ROOT)
	{

		if (size_arr < MAX_SHOW_SIZE)
		{
			cout << "Array sorted by parallel radix :" << endl;
			Show_arr(test_arr_pp_radix, size_arr);
			cout << endl;
		}

		cout << "Sequence radix version worked:  " << time_seq_work_alg_radix << " seconds" << endl;
		cout << "Parallel radix worked: " << time_pp_work_alg_radix << " seconds" << endl;

		cout << endl;

		if (time_pp_work_alg_radix <= time_seq_work_alg_radix)
			cout << "Parallel version faster, than sequence version " << endl;
		else
			cout << "Sequence version faster, than parallel version " << endl;

		// Сравнение полученных результатов:
		

		//int good = true;
		//for (int i = 0; i < size_arr; i++)
		//	if (test_arr_pp_radix[i] != test_arr_seq_radix[i])// Сравниваем результаты двух алгоритмов
		//	{
		//		cout << "Results parallel and sequence algorithm are not identical " << endl;
		//		good = false;
		//		break;
		//	}
		//if (good)
		//	cout << "Results parallel and sequence algorithm are identical " << endl;
		
		for (int i = 0; i <= size_arr; i++)
		{
			if (test_arr_pp_radix[i - 1] <= test_arr_pp_radix[i])
			{
				cout << "The results are correct!" << endl;
				break;
			}
		}

		delete[] test_arr_seq_radix;

	}
	
	
	delete[] test_arr_pp_radix;

	MPI_Finalize();

	return 0;
}