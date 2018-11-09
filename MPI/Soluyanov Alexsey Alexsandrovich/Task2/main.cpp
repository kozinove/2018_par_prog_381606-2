#include<mpi.h>
#include <queue>
#include <thread>
#include<iostream>
#include <ctime>
using namespace std;

#define MANAGER 0
#define PRODUCER 1
#define CONSUMER 2


const int GET = 1;
const int PUT = 2;
const int EXITC = 3;
const int EXITP = 4;
const int STOP = -1;

struct info {
	int rank;
	int request;
	int res;
};

//--------------------ѕотребитель--------------------//

class Consumer { 
	info Consumer_Info;
	queue<int> Cons_Queue;
	int resources_for_consume;
	 
	void Request_Resource() 
	{
		MPI_Send(&Consumer_Info, 3, MPI_INT, MANAGER, 0, MPI_COMM_WORLD);
	}
	void Get_Answer() {
		int res = 0;
		MPI_Status status;
		MPI_Recv(&res, 1, MPI_INT, MANAGER, 3, MPI_COMM_WORLD, &status);
		if (res == -1) {
			MPI_Finalize();
			exit(0);
		}
		if (res == 0 ) {
			cout << "Buffer is empty!" << endl;
		}
		if (res != 0) {
			Cons_Queue.push(res);
			cout << "Consumer " << Consumer_Info.rank << " get the resource " << Consumer_Info.res << endl;
			resources_for_consume--;
		}
	}
public:
	Consumer(int rank, int num_of_res) {
		Consumer_Info.rank = rank;
		resources_for_consume = num_of_res;
		Consumer_Info.request = GET;
	}
		
	void Start() {
		while (resources_for_consume) {
			Request_Resource();
			Get_Answer();
		}
		Consumer_Info.request = EXITC;
		MPI_Send(&Consumer_Info, 3, MPI_INT, MANAGER, 0, MPI_COMM_WORLD);
	}
};

//------------------ѕроизводитель------------------//

class Producer {
	queue<int> prod_queue;
	int resources_for_prod;
	info Producer_Info;
	int k;


	void put_resource() {
		Producer_Info.res = prod_queue.front();
		int answer;
		MPI_Status status;		
		MPI_Send(&Producer_Info, 3, MPI_INT, MANAGER, 0, MPI_COMM_WORLD);
		MPI_Recv(&answer, 1, MPI_INT, MANAGER, 1, MPI_COMM_WORLD, &status);
		if (answer == -1) {
			MPI_Finalize();
			exit(0);
		}
		else if (answer == 0) {
			cout << "Producer " << Producer_Info.rank << " send the resource " << Producer_Info.res << endl;
			prod_queue.pop();
		}
		else {
			cout << "Producer "<< Producer_Info.rank <<" could not put resource. Buffer is full!" << endl;
		}
	}
	void create_resource() {
		int res;
		res = Producer_Info.rank;
		//res  = k - resources_for_prod + (Producer_Info.rank - 1) * 5 + 1;
		prod_queue.push(res);
		resources_for_prod--;
	}

public:
	Producer(int rank, int num) {
		Producer_Info.rank = rank;
		resources_for_prod = num;
		Producer_Info.request = PUT;
		k = resources_for_prod;
	}

	void Start() {
		/*while (resources_for_prod && !prod_queue.empty()) {
			create_resource();
			put_resource();
		}*/
		while (resources_for_prod) {
			create_resource();
			put_resource(); 
		}
		while (!prod_queue.empty()) {
			put_resource(); 
		}
		Producer_Info.request = EXITP;
		MPI_Send(&Producer_Info, 3, MPI_INT, MANAGER, 0, MPI_COMM_WORLD);
	}
};

class Manager {
	int total_res;
	int *buf;
	int N;
	int cons, prod;
	int p_size;
	info info_manager;
	MPI_Status status;

	void Put(int proc_id, int resource) {
		int answer = 1;
		for (int i = 0; i < N; i++) {
			if (buf[i] == 0) {
				answer = 0;
				buf[i] = resource;
				break;
			}
		}
		if (answer == 0) {
			cout << "Manager: producer " << proc_id << " put resourse " << resource << endl;
		}
		MPI_Send(&answer, 1, MPI_INT, proc_id, 1, MPI_COMM_WORLD);
	}

	void Get(int proc_id) {
		int resource = 0;
		for (int i = 0; i < N; i++) {
			if (buf[i] != 0) {
				resource = buf[i];
				buf[i] = 0;
				break;
			}
		}
		if (resource != 0) {
			cout << "Manager: consumer " << proc_id << " get resourse " << resource << endl;
		}
		else {
			if (prod == 0) {
				for (int i = 1; i < N; i++)
				{
					cout << "Ptoduser Size = " << i << endl;
					if (i % 2)
						MPI_Send(&STOP, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
					else
						MPI_Send(&STOP, 1, MPI_INT, i, 3, MPI_COMM_WORLD);
				}
			}
		}
		MPI_Send(&resource, 1, MPI_INT, proc_id, 3, MPI_COMM_WORLD);
	}

public:
	Manager(int totalres, int proc_size, int num_prod, int num_cons) {
		total_res = totalres;	
		p_size = proc_size;
		N = total_res;
		cons = num_cons;
		prod = num_prod;
		buf = new int[N];
		for (int i = 0; i < N; i++) {
			buf[i] = 0;
		}
	}

	void Start() {
		while (true) {
			MPI_Recv(&info_manager, 3, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
			if (info_manager.request == EXITP) {
				prod--;
			}
			if (info_manager.request == EXITC) {
				cons--;
				if (cons == 0) {
					for (int i = 1; i < p_size; i++) {
						if (i % 2)
							MPI_Send(&STOP, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
						else
							MPI_Send(&STOP, 1, MPI_INT, i, 3, MPI_COMM_WORLD);
					}
					MPI_Finalize();
					exit(0);
				}
			}
			if (info_manager.request == PUT) {
				Put(info_manager.rank, info_manager.res);
			}
			if (info_manager.request == GET) {
				Get(info_manager.rank);
			}
		}
	}
};


int main(int argc, char **argv)
{
	MPI_Init(&argc, &argv);
	MPI_Status status;

	int rank = -1;
	int process_num = -1;
	int pro, com;
	int res = 0;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank); //определ€ем ранг процесса
	MPI_Comm_size(MPI_COMM_WORLD, &process_num); //определ€ем кол-во процессов

	int a = (process_num - 1);
	pro = a / 2;
	com = a / 2;
	if (a % 2)
		pro++;

	if (rank == 0) { //если в нулевом процессе
		Manager manager(3, process_num ,pro, com);
		manager.Start(); //запускаем менеджера
	}
	else { //если не в нулевом процессе
		if (rank % 2) { //каждый второй процесс будет производителем
			cout << "The process with the rank of " << rank << " is Producer" << endl;
			Producer producer(rank, 5); //создаем производител€ дл€ определенного процесса с 5 ресурсами
			producer.Start(); //запускаем его
		}
		else { //а остальные потребител€ми
			cout << "The process with the rank of " << rank << " is Consumer" << endl;
			Consumer consumer(rank, 5); //создаем потребител€ дл€ определенного процесса с 5 ресурсами
			consumer.Start(); //создаем потребител€
		}
	}

	MPI_Finalize(); //заканчивем работу MPI
	return 0;
}