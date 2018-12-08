


#include <mpi.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#define N 10000
#define min_size 1


using namespace std;


struct  point;
void ConvexHullJarvis(const vector<point> &mas, vector<int> &convex_hull, int n);
bool More(double x, double y);
bool Less(double x, double y);
bool Equal(double x, double y);
double dist(point, point);
double CosAngle(point a, point b, point c);

struct point
{
	int x;
	int y;

	point(int _x, int _y)
	{
		x = _x;
		y = _y;
	}
	point()
	{
	
	};

};


bool operator!=(point a, point b)
{
	if (a.x != b.x || a.y != b.y) return true;
	return false;
}

bool operator==(point a, point b)
{
	if (a.x == b.x && a.y == b.y) return true;
	return false;
}







int shell(int *buffer, int point_count, vector<int> &out_index)
{
	
	vector<point> v_p;
	
	int i_p1 = 0;
	int i_p2 = -1;
	int i_p3 = -1;
	// Посчитать количество различных точек. Изи

	
	point *shell = new point[point_count];

	for (int i = 0; i < point_count; i++)
	{
		point point_buffer;
		point_buffer.x = buffer[i * 2];
		point_buffer.y = buffer[i * 2 + 1];
		v_p.push_back(point_buffer);

		if (v_p[i] != v_p[i_p1])
		{
			if (i_p2 == -1)
			{
				i_p2 = i;
			}
			else
			{
				if (v_p[i_p2] != v_p[i])
				{
					i_p3 = i;
				}

			}

		}
	}

	if (i_p1 == 0 && i_p2 == -1 && i_p2 == -1)
	{
		out_index.push_back(0);
		return 1;
	}
	if (i_p1 == 0 && i_p2 != -1 && i_p3 == -1)
	{
		out_index.push_back(0);
		out_index.push_back(i_p2);
		return 2;
	}


	ConvexHullJarvis(v_p, out_index, point_count);


	
	int shell_count = out_index.size() - 1;
	

	return shell_count;

}





void ConvexHullJarvis(const vector<point> &mas, vector<int> &convex_hull, int n)
{
	// находим самую левую из самых нижних
	int base = 0;
	for (int i = 1; i<n; i++)
	{
		if (mas[i].y < mas[base].y)
			base = i;
		else
			if (mas[i].y == mas[base].y &&
				mas[i].x < mas[base].x)
				base = i;
	}
	// эта точка точно входит в выпуклую оболочку
	convex_hull.push_back(base);

	point first = mas[base];
	point cur = first;
	point prev = point(first.x - 1, first.y);
	do
	{
		double minCosAngle = -1e9; // чем больше угол, тем меньше его косинус
		double maxLen = 1e9;
		int next = -1;


		for (int i = 0; i<n; i++)
		{
			
			double curCosAngle = CosAngle(prev, cur, mas[i]);
			if (More(curCosAngle, minCosAngle))
			{
				next = i;
				minCosAngle = curCosAngle;
				maxLen = dist(cur, mas[i]);
			}
			else if (Equal(curCosAngle, minCosAngle))
			{
				double curLen = dist(cur, mas[i]);
				if (More(curLen, maxLen))
				{
					next = i;
					maxLen = curLen;
				}
			}
		}



		prev = cur;
		cur = mas[next];
		convex_hull.push_back(next);
	} while (cur != first);
}


bool More(double x, double y)
{
	if (x > y) return true;
	return false;
}

bool Less(double x, double y)
{
	if (x < y) return true;
	else return false;
}


bool Equal(double x, double y)
{
	if (x == y) return true;
	else return false;
}


double dist(point a, point b)
{
	return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}


double CosAngle(point a, point b, point c)
{

	if (a == b || b == c || c == a) return -1e10;

	int ax, ay, bx, by;

	ax = b.x - a.x;
	ay = b.y - a.y;
	bx = c.x - b.x;
	by = c.y - b.y;

	a.x *= -1;
	a.y *= -1;

	double cos;

	cos = ((ax * bx) + (ay * by)) / (sqrt(ax * ax + ay * ay) * sqrt(bx * bx + by * by));
	return cos;
	

}


int main(int argc, char **argv)
{

	int taskid, numtasks;

	int work_size;

	int *point_buffer = NULL;

	int *p_work_size = NULL;

	int *disp = NULL;

	int *double_p_work_size = NULL;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);


	if (taskid == 0)
	{

		srand(time(NULL));

		work_size = N;
		
		point_buffer = new int[2 * N];

		for (int i = 0; i < N * 2; i++)
		{
			point_buffer[i] = rand() % 100000 - 50000;
		
		}
		






		
			/*string st;

			st = "";
			for (int i = 0; i < N; i++)
			{
				st += "(" + to_string(point_buffer[i * 2]) + ", " + to_string(point_buffer[i * 2 + 1]) + ") ";
			}
			st += "\n";
			cout << st;


			*/





		cout << "Process NUM = " << numtasks << "\n";

	

		p_work_size = new int[numtasks];
		
		int standart_task;

		standart_task = N / numtasks;

	//	cout << "standart_task = " << standart_task << "\n";
		
		int real_proc_count = 0;

		if (standart_task >= min_size) // All is ok. Заданий достаточно
		{
			for (int i = 0; i < numtasks - 1; i++)
			{
				p_work_size[i] = standart_task;
			}
			p_work_size[numtasks - 1] = N % numtasks + standart_task;

			real_proc_count = numtasks;
		}
		else // Точек недостаточно
		{
			real_proc_count = N / min_size;
			if (N % min_size != 0) real_proc_count++;

			for (int i = 0; i < real_proc_count; i++)
			{	
				p_work_size[i] = min_size;
			}
			
			if (N % min_size != 0)
			p_work_size[real_proc_count - 1] = N % min_size;
			else
			{
				p_work_size[real_proc_count - 1] = min_size;
			}

			for (int i = real_proc_count; i < numtasks; i++)
			{
				p_work_size[i] = 0;
			}

		}

	


		disp = new int[numtasks];
		disp[0] = 0;

		for (int i = 1; i < numtasks; i++)
		{
			disp[i] = disp[i - 1] + p_work_size[i - 1] * 2;
		}

		double_p_work_size = new int[numtasks];


		for (int i = 0; i < numtasks; i++)
		{
			double_p_work_size[i] = p_work_size[i] * 2;
		}

	}

	MPI_Scatter(p_work_size, 1,	MPI_INT, &work_size, 1,	MPI_INT, 0,	MPI_COMM_WORLD);

	//cout << "task id = " << taskid << " work_size " << work_size << "\n";

	int *rec_point_buf = new int[work_size * 2];

	MPI_Scatterv(point_buffer, double_p_work_size, disp, MPI_INT, rec_point_buf, work_size * 2, MPI_INT, 0, MPI_COMM_WORLD);


	int *shell_buf = NULL;

	int shell_count;

	vector<int> out_index;


	if (work_size != 0)
	{
		shell_count = shell(rec_point_buf, work_size, out_index);

		shell_buf = new int[shell_count * 2];

		for (int i = 0; i < shell_count; i++)
		{
			shell_buf[i * 2] = rec_point_buf[out_index[i] * 2];
			shell_buf[i * 2 + 1] = rec_point_buf[out_index[i] * 2 + 1];
		}
	}
	else
	{
		shell_count = 0;
	}


	// Вот тут мы спокойненько все отсортировали. Теперь должны понять, сколько реально процессов учавствовали во всем этом.
	// Вычислим в каждом, не лишний ли он?

	int right_num_proc = 1;
	
	while ( right_num_proc * 2 <= numtasks )
	{
		right_num_proc *= 2;
	}

	//cout << "right num proc = " << right_num_proc << "\n";

	// если процесс 1, то надо завершаться

	// В каждом процессе надо понять, будет ли он отдавать себя как хвост?

	bool  send_tail = false;

	if (taskid >= right_num_proc) send_tail = true;



	// Сколько же всего хвостов?
	int tails_count = numtasks - right_num_proc;

	//cout << "all tails = " << tails_count << "\n";

	bool rec_tail = false;

	if (taskid < tails_count) rec_tail = true;

	


	// Так, осталось принять и отдать. Понять кому принять и кому отдать.

	int send_to, rec_from;

	int send_count, rec_count;

	int *union_buffer = NULL;

	if (send_tail)
	{

		send_to = taskid - right_num_proc;
	//	cout << "numproc = " << taskid << " and I send  tail to " << send_to << "\n";
		send_count = shell_count;

		MPI_Send(&send_count, 1, MPI_INT, send_to, 8080, MPI_COMM_WORLD);
		MPI_Send(shell_buf, shell_count * 2, MPI_INT, send_to, 8081, MPI_COMM_WORLD);

		/*string st = "id = " + to_string(taskid) + " send_points -  ";

		for (int i = 0; i < shell_count; i++)
		{
			st += "(" + to_string(shell_buf[i * 2]) + ", " + to_string(shell_buf[i * 2 + 1]) + ") ";
		}
		st += "\n";
		cout << st;



		*/

	}

	if (rec_tail)
	{
		
		MPI_Status st1; MPI_Status st2;
		rec_from = right_num_proc + taskid;
	//	cout << "numproc = " << taskid << " and I rec tail from " << rec_from << "\n";
		MPI_Recv(&rec_count, 1, MPI_INT, rec_from, 8080, MPI_COMM_WORLD, &st1);
		
		//cout << "id = " << taskid << " already rec = " << rec_count << " and shell count = " << shell_count <<  "\n";
		
		union_buffer = new int[(rec_count + shell_count) * 2];



		MPI_Recv(union_buffer, rec_count * 2, MPI_INT, rec_from, 8081, MPI_COMM_WORLD, &st2);


		for (int i = rec_count; i < shell_count + rec_count; i++)
		{
			union_buffer[i * 2] = shell_buf[i * 2 - rec_count * 2];
			union_buffer[i * 2 + 1] = shell_buf[i * 2 - rec_count * 2 + 1];
		}


		/*string str;
		str = "union pints ";

		for (int i = 0; i < rec_count + shell_count; i++)
		{
			str += "(" + to_string(union_buffer[i * 2]) + ", " + to_string(union_buffer[i * 2 + 1]) + ") ";
		}
		str += "\n";
		cout << str;
		*/

		/*//string str;
		str = "my pints ";

		for (int i = 0; i < shell_count; i++)
		{
			str += "(" + to_string(shell_buf[i * 2]) + ", " + to_string(shell_buf[i * 2 + 1]) + ") ";
		}
		str += "\n";
		cout << str;
		*/

		shell_count = shell_count + rec_count;
	}
	
	
	// отлично, теперь бы надо все стандартизировать. Так что назовем все массивы одинаково
	if ( !(rec_tail || send_tail)) // т.е. ничего не делали
	{
		union_buffer = shell_buf;
	}
	// теперь попробуем вывести все это


	/*if (taskid < right_num_proc) {

		string str = "";
		str = "my id = " + to_string(taskid) + "my count" + "my pints - ";

		for (int i = 0; i < shell_count; i++)
		{
			str += "(" + to_string(union_buffer[i * 2]) + ", " + to_string(union_buffer[i * 2 + 1]) + ") ";
		}
		str += "\n";
		cout << str;
	}
	*/

	// Таак, теперь в каждом процессе есть shell_count - количество элементов в оболочке и union_buffer это оболочка

	int level = 0;
	bool live = true; // Не отдал ли процесс свои данные?

	

	while ((1 << level) < right_num_proc)
	{

		bool for_send = false;
		bool for_rec = false;

		int mask = 1 << level;

		int k, k1;




		if (live && taskid < right_num_proc && numtasks > 1)
		{
			// Как понять, принимаем мы или пересылаем данные? Хмм, ну, посомтрим бит номер level. Если 1, то шлем, если нет, то пересылаем


			if (((mask & taskid) != 0) && live) for_send = true;
			if (((mask & taskid) == 0) && live) for_rec = true;



			if (for_send)
			{
				live = false;

				k1 = shell_count;
				k = shell_count;

				send_to = 1 << level;

				MPI_Send(&k, 1, MPI_INT, taskid - send_to, 90, MPI_COMM_WORLD);
				MPI_Send(union_buffer, k * 2, MPI_INT, taskid - send_to, 91, MPI_COMM_WORLD);
			}



			if (for_rec)
			{
				MPI_Status st_l;
				rec_from = 1 << level;
				k = shell_count;
				MPI_Recv(&k1, 1, MPI_INT, taskid + rec_from, 90, MPI_COMM_WORLD, &st_l);
			//	cout << "id = " << taskid << " count = " << k1 + k << " \n";




				int *tmp_name = new int[(k1 + k) * 2];
				MPI_Recv(tmp_name, k1 * 2, MPI_INT, taskid + rec_from, 91, MPI_COMM_WORLD, &st_l);
				//int *t = new int[0];




				for (int i = 0; i < k; i++)
				{
					tmp_name[i * 2 + k1 * 2] = union_buffer[i * 2];
					tmp_name[i * 2 + k1 * 2 + 1] = union_buffer[i * 2 + 1];
					//tmp_name[1] = union_buffer[1];
				}

				// Перекопировали в конец. Попробуем
				
				shell_count = k + k1;
				/*
							string st;

							st = "";
							for (int i = 0; i < shell_count; i++)
							{
								st += "(" + to_string(union_buffer[i * 2]) + ", " + to_string(union_buffer[i * 2 + 1]) + ") ";
							}
							st += "\n";
							cout << st;

				*/
				//Буфферы можно снова сократить

				if (shell_count > 0)
				{

					vector<int> index;

					shell_count = shell(tmp_name, shell_count, index);

					union_buffer = new int[shell_count * 2];

					for (int i = 0; i < shell_count; i++)
					{
						union_buffer[i * 2] = tmp_name[index[i] * 2];
						union_buffer[i * 2 + 1] = tmp_name[index[i] * 2 + 1];
					}

					// Вот и привели все в порядок. Редуцировали точки
				}

			}

		}

		level += 1;
	}

//	if (taskid < right_num_proc && for_send) cout << "id = " << taskid << "im for send" << "\n";
//	if (taskid < right_num_proc && for_rec) cout << "id = " << taskid << "im for rec" << "\n";

	if (taskid == 0)
	{
		string st;

		st = "";
		for (int i = 0; i < shell_count; i++)
		{
			st += "(" + to_string(union_buffer[i * 2]) + ", " + to_string(union_buffer[i * 2 + 1]) + ") ";
		}
		st += "\n";
		cout << st;


	}




	MPI_Finalize();

	return 0;
}