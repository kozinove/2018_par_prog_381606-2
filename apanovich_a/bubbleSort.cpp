#include <iostream> 
#include <ctime>  
#include <mpi.h> 

using namespace::std;

#define ROOT 0 // ������� ����� 0 - �������� �������

int curr_rank; // ������� ���� ��������
int num_process; // ����� ���������

// �������� �������
int* Create_Array(int size)
{
	int* arr;

	arr = new int[size];

	for (int i = 0; i < size; i++)
		arr[i] = rand() % 100 -100; 

	return arr;
}

// ����� �������
void Show_Array(int* arr, int size)
{
	for (int i = 0; i < size; i++)
		cout << arr[i] << " ";
	cout << endl;
}

// ��������� � ����� ���������
void Swap(int& a1, int& a2)
{
	int tmp = a1;
	a1 = a2;
	a2 = tmp;
}

// ���������� ����������� ��� �������������� ��������� �������� ��������� (���������� ������ ������ ������� ������ ���������)
void Bubble_sort(int* arr, int size_arr)
{
	for (int i = 0; i < size_arr; i++)
		if (i % 2 == 1) // ���� �����������, �� ��� �������� ��������
		{
			for (int j = 0; j < size_arr / 2 - 1; j++)
				if (arr[2 * j + 1] > arr[2 * j + 2])
					Swap(arr[2 * j + 1], arr[2 * j + 2]);
			if (size_arr % 2 == 1) // ���� size_arr �������, �� ���������� ��������� ����, ��������� � �������� �������� (�.�. ����� � ������������ ����� ��������� �������)
				if (arr[size_arr - 2] > arr[size_arr - 1])
					Swap(arr[size_arr - 2], arr[size_arr - 1]);
		}
		else // ������ ��������
			for (int j = 0; j < size_arr / 2; j++)
				if (arr[2 * j] > arr[2 * j + 1])
					Swap(arr[2 * j], arr[2 * j + 1]);
}

void Calc_work_and_displs(int* displs, int* send_num_work, int size) // ������+��������
{
	int size_work = size / num_process,
		rest = size % num_process;

	for (int i = 0; i < rest; i++)
	{
		displs[i] = (size_work + 1) * i;
		send_num_work[i] = size_work + 1;
	}

	for (int i = rest; i < num_process; i++)
	{
		displs[i] = size_work * i + rest;
		send_num_work[i] = size_work;
	}
}
// ���������-����� � ������� ������
void Compare_split_right(int* curr_buf, int size_curr_buf, int id_right_proc, int size_buf_right_proc) 
{
	MPI_Status status;

	int* recvBuf = new int[size_buf_right_proc];
	int* merge_array = new int[size_curr_buf + size_buf_right_proc];

	// ������������ �������� � ������� ������
	MPI_Sendrecv(curr_buf, size_curr_buf, MPI_INT, id_right_proc, 1, recvBuf, size_buf_right_proc, MPI_INT, id_right_proc, 1, MPI_COMM_WORLD, &status);

	int i = 0, j = 0, k = 0;

	// �������
	while (i < size_curr_buf && j < size_buf_right_proc)
	{
		if (curr_buf[i] < recvBuf[j])
			merge_array[k++] = curr_buf[i++];
		else
			merge_array[k++] = recvBuf[j++];
	}

	while (j < size_buf_right_proc)
		merge_array[k++] = recvBuf[j++];

	while (i < size_curr_buf)
		merge_array[k++] = curr_buf[i++];

	// ���������� �������, �������� ����� ��������� ������� ��������
	for (int i = 0; i < size_curr_buf; i++)
		curr_buf[i] = merge_array[i];

	delete[] recvBuf;
	delete[] merge_array;
}

// ���������-����� � ������� �����
void Compare_split_left(int* curr_buf, int size_curr_buf, int id_left_proc, int size_buf_left_proc)
{
	MPI_Status status;

	int* recvBuf = new int[size_buf_left_proc];
	int* merge_array = new int[size_curr_buf + size_buf_left_proc];

	// ������������ �������� � ������� �����
	MPI_Sendrecv(curr_buf, size_curr_buf, MPI_INT, id_left_proc, 1, recvBuf, size_buf_left_proc, MPI_INT, id_left_proc, 1, MPI_COMM_WORLD, &status);

	int i = 0, j = 0, k = 0;

	// �������
	while (i < size_curr_buf && j < size_buf_left_proc)
	{
		if (curr_buf[i] < recvBuf[j])
			merge_array[k++] = curr_buf[i++];
		else
			merge_array[k++] = recvBuf[j++];
	}

	while (j < size_buf_left_proc)
		merge_array[k++] = recvBuf[j++];

	while (i < size_curr_buf)
		merge_array[k++] = curr_buf[i++];

	// ���������� �������, ����� � ������� ��������� ��������� ������� ��������
	for (int i = 0; i < size_curr_buf; i++)
		curr_buf[i] = merge_array[size_buf_left_proc + i];

	delete[] recvBuf;
	delete[] merge_array;
}

// ���������� ������������ ������� ���-�����
void Sort_pp(int* rbuf, int* send_num_work)
{
	for (int i = 0; i < num_process; i++)
	{
		if (i % 2 == 1) // �������� ��������
		{
			if (curr_rank % 2 == 1) // �������� ����� ��������
			{
				if (curr_rank < num_process - 1) // ��������� - ����� � ������� ������
					Compare_split_right(rbuf, send_num_work[curr_rank], curr_rank + 1, send_num_work[curr_rank + 1]);
			}
			else
			{
				if (curr_rank > 0)// ��������� - ����� � ������� �����
					Compare_split_left(rbuf, send_num_work[curr_rank], curr_rank - 1, send_num_work[curr_rank - 1]);
			}
		}
		else // ׸���� ��������
		{
			if (curr_rank % 2 == 0) //������ ����� ��������
			{
				if (curr_rank < num_process - 1) // ��������� - ����� � ������� ������
					Compare_split_right(rbuf, send_num_work[curr_rank], curr_rank + 1, send_num_work[curr_rank + 1]);
			}
			else // ��������� - ����� � ������� �����
				Compare_split_left(rbuf, send_num_work[curr_rank], curr_rank - 1, send_num_work[curr_rank - 1]);
		}
	}
}

int main(int argc, char* argv[])
{
	int* Array_Bubble_Seq = NULL;
	
	int* Array_Bubble_Pp = NULL;

	int* displs; // ������ �������� ������������ ������ ������ Array
	int* send_num_work; // ������ ���-�� ������ ��� ������� ��������
	int* rbuf;

	int size = 0;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &num_process);
	MPI_Comm_rank(MPI_COMM_WORLD, &curr_rank);

	if (curr_rank == ROOT)
	{
		cout << "Enter size of array: " << endl;
		cin >> size;

		Array_Bubble_Seq = Create_Array(size);

		
		Array_Bubble_Pp = new int[size];
		

		for (int i = 0; i < size; i++)
		{
			Array_Bubble_Pp[i] = Array_Bubble_Seq[i];
			
		}

		if (size < 500)
		{
			cout << "Unorted array: " << endl;
			Show_Array(Array_Bubble_Seq, size);
		}
	}

	// Parallel
	

	send_num_work = new int[num_process];
	displs = new int[num_process];

	if (curr_rank > 0)
	{
	Array_Bubble_Pp = new int[size];
		
	}

	//// ������������ ������ �������� � ���-�� ������
	Calc_work_and_displs(displs, send_num_work, size);

	rbuf = new int[send_num_work[curr_rank]];
	// ���������� ��������� � ������� ������-�������� ������������ ������������ ������
	if (curr_rank > 0)
	{
		Array_Bubble_Pp = new int[size];
	}

	// �������� ������ �� ��������� �������� ���� ���������
	MPI_Bcast(&size, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

	Calc_work_and_displs(displs, send_num_work, size);

	MPI_Scatterv(Array_Bubble_Pp, send_num_work, displs, MPI_INT, rbuf, send_num_work[curr_rank], MPI_INT, ROOT, MPI_COMM_WORLD);

	Bubble_sort(rbuf, send_num_work[curr_rank]); //��������� ����� ������� (������)

	Sort_pp(rbuf, send_num_work); // ���-����� ����������

	MPI_Gatherv(rbuf, send_num_work[curr_rank], MPI_INT, Array_Bubble_Pp, send_num_work, displs, MPI_INT, ROOT, MPI_COMM_WORLD);

	if (curr_rank == ROOT)
	{
		

		if (size < 500)
		{
			cout << "Sorted array by Even/Odd sort:" << endl;
			Show_Array(Array_Bubble_Pp, size);
			cout << endl;
			
		}
		
		cout << endl;
		
		delete[] Array_Bubble_Pp;
		delete[] displs;
		delete[] send_num_work;
		delete[] rbuf;

		MPI_Finalize();

		return 0;
	

	}
}