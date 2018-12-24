


#include <mpi.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#define N 10000
#define min_size 1
#include <fstream>


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
		x = 0;
		y = 0;
	};
	void operator=(point _b)
	{
		x = _b.x;
		y = _b.y;	
	}


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

bool equal_arrays(int *a, int *b, int n)
{
	for (int i = 0; i < n; i++)
	{
		if (a[i] != b[i]) return false;		
	}
	return true;
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

int new_shell(int *buffer, int point_count, vector<int> &out_index)
{

	out_index.clear();
	vector<point> points;

	for (int i = 0; i < point_count; i++)
	{
		points.push_back(point(buffer[i * 2], buffer[i * 2 + 1]));
	}
	// Получили массив точек.

	// Начнем с проверки, что различных точек вообще больше трех
	int p1 = 0, p2 = -1, p3 = -1;

	for (int i = 0; i < point_count; i++)
	{
		if (p2 == -1 && points[i] != points[p1])
		{
			p2 = i;
			continue;
		}

		if (p1 != -1 && p2 != 0 && points[i] != points[p1] && points[i] != points[p2])
		{
			p3 = i;
			break;
		}
	}

	if (p2 == -1)
	{
		out_index.push_back(0);
		return 1;
	}

	if (p2 != -1 && p3 == -1)
	{

		// Только вот тут важен порядок, а то потом массивы не сойдутся


		if (points[0].y < points[p2].y)
		{
			out_index.push_back(0);
			out_index.push_back(p2);
			return 2;
		}


		if (points[0].y == points[p2].y)
		{
			if (points[0].x < points[p2].x)
			{
				out_index.push_back(0);
				out_index.push_back(p2);
				return 2;
			}
			else
			{
				out_index.push_back(p2);
				out_index.push_back(0);
				return 2;
			}
		}
		else
		{
			out_index.push_back(p2);
			out_index.push_back(0);
			return 2;
		}

	}

	// Тут точно есть три различные точки
	// Найдем самую леву и нижнюю из всех

	int base = 0;

	for (int i = 0; i < point_count; i++)
	{

		if (points[i].y < points[base].y)
		{
			base = i;
			continue;
		}

		if (points[i].y == points[base].y && points[i].x < points[base].x)
		{
			base = i;
			continue;
		}

	}
	//Нашли самую левую из всех нижних
	point cur, prev, next;

	cur = points[base];
	prev = cur; prev.x -= 1000;
	out_index.push_back(base);




	while (true)
	{



		bool find_correct = false;

		double max_cos = -3;
		double max_dist;

		int pretendent;
		for (int i = 0; i < point_count; i++)
		{
			next = points[i];




			// Проверить, нет ли ее еще в оболочке?
			bool al_exist = false;
			for (int j = 0; j < out_index.size(); j++)
			{
				if (points[i] == points[out_index[j]])
				{
					al_exist = true;
					break;
				}
			}

			if (al_exist) continue; // Если наткнулись на точку в оболоче - пропускаем ее

									//Если точка не в оболочке, то стоит проверять уже потихоньку угол

			double cos = CosAngle(prev, cur, next);


			// Если угол получился более развернутый
			if (cos > max_cos)
			{
				max_cos = cos;
				pretendent = i;
				max_dist = dist(cur, next);
				continue;
			}
			// Если угол один и тот же
			if (cos == max_cos)
			{
				double pr_d = dist(cur, next);
				//Выбираем по максимальной дисстанции
				if (pr_d > max_dist)
				{
					pretendent = i;
					max_dist = pr_d;
				}
				continue;
			}




		}



		// На всякий случай не забудем проверить угол с начальной, базовой точкой.
		double base_cos = CosAngle(prev, cur, points[base]);
		if (base_cos >= max_cos)
		{
			//Все отлично, оболочка найдена
			// Нужно прекратить поиск
			break;
		}

		out_index.push_back(pretendent);
		prev = cur;
		cur = points[pretendent];
	}

	return int(out_index.size());

}

void ConvexHullJarvis(const vector<point> &mas, vector<int> &convex_hull, int n) // Старый вариант
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

	point first;
	first.x = mas[base].x;
	first.y = mas[base].y;

	point cur = first;
	point prev = point(first.x - 1000, first.y);
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
	return sqrt(double((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y)));
}

int nod(int a, int b) // Наименьший общий делитель
{
	a = abs(a);
	b = abs(b);

	int k = 0;

	while (a != 0 && b != 0)
	{
		if (a > b) a %= b;
		else b %= a;
	}


	k = a + b;
	return k;
}

int one_line(point a, point b, point c) //Лежат на одной линии
{
	int ax = b.x - a.x;
	int ay = b.y - a.y;

	int bx = c.x - b.x;
	int by = c.y - b.y;

	// Нашли радиус-вектора.

	int ka = nod(ax, ay);
	int kb = nod(bx, by);

	ax /= ka;
	ay /= ka;

	bx /= kb;
	by /= kb;

	if (ax == bx && ay == by) return 1; // Сонаправлены
	if (ax == -bx && ay == -by) return -1; // противонаправлены
	return 0;


}

double CosAngle(point a, point b, point c) // Косинус угла
{

	if (a == b || b == c) return -1e10; // Бесконечность

	int line = one_line(a, b, c);

	if (line == -1) return -1;
	if (line == 1) return 1;



	double ax, ay, bx, by;

	ax = b.x - a.x;
	ay = b.y - a.y;
	bx = c.x - b.x;
	by = c.y - b.y;

	a.x *= -1;
	a.y *= -1;

	int k1, k2;
	k1 = nod(ax, ay);
	k2 = nod(bx, by);

	ax /= k1; ay /= k1;
	bx /= k2; by /= k2;


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

	int all_data_count = 1000;

	int a, b;

	double start_programm_time;
	double end_programm_time;
	double cur_proc_time = 0;
	
	double time_to_call_shell = 0;

	int act;
	// Menu
	if (taskid == 0)
	{
		cout << "Process NUM = " << numtasks << "\n";


		cout << "1. Use random data\n";
		cout << "2. Use user data\n";
		//cout << "3. Use file data\n";
		cout << "Select an action: ";
		cin >> act;

		if (act == 1)
		{
			cout << "Enter point count: ";
			cin >> all_data_count;

			cout << "Enter left and right border: ";
			cin >> a;
			cin >> b;
		}
		else
		{
			if (act == 2)
			{
				cout << "Enter point count: ";
				cin >> all_data_count;
			}
			else
			{
				act == 1;
				all_data_count = 1000;
				a = -1000;
				b = 1000;

			}
		}
	
		


		cout << "\nWork started\n";
	}
	// Menu end





	// Расчитаем объемы работ и разделение данных. Введем данные, если пользователь выбрал собственные точки
	if (taskid == 0)
	{

		srand(time(NULL));

		work_size = all_data_count;
		




		point_buffer = new int[2 * all_data_count];



		if (act == 1)
		{
			for (int i = 0; i < all_data_count * 2; i++)
			{
				int dif;
				dif = b - a;
				point_buffer[i] = rand() % dif + a;
			}
		}
		if (act == 2)
		{
			cout << "Enter points in format x1 y1 x2 y2... :";
			for (int i = 0; i < all_data_count * 2; i++)
			{
				cin >> point_buffer[i];
			}
		}

		cout << "Point count = " << all_data_count << "\n";





		p_work_size = new int[numtasks];
		
		int standart_task;

		standart_task = all_data_count / numtasks;

	//	cout << "standart_task = " << standart_task << "\n";
		
		int real_proc_count = 0;

		if (standart_task >= min_size) // All is ok. Заданий достаточно
		{
			for (int i = 0; i < numtasks - 1; i++)
			{
				p_work_size[i] = standart_task;
			}
			p_work_size[numtasks - 1] = all_data_count % numtasks + standart_task;

			real_proc_count = numtasks;
		}
		else // Точек недостаточно на все процессы
		{
			real_proc_count = all_data_count / min_size;
			if (all_data_count % min_size != 0) real_proc_count++;

			for (int i = 0; i < real_proc_count; i++)
			{	
				p_work_size[i] = min_size;
			}
			
			if (all_data_count % min_size != 0)
			p_work_size[real_proc_count - 1] = all_data_count % min_size;
			else
			{
				p_work_size[real_proc_count - 1] = min_size;
			}

			for (int i = real_proc_count; i < numtasks; i++)
			{
				p_work_size[i] = 0;
			}

		}

	


		disp = new int[numtasks]; // Массив смещений для scatter_v
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


		start_programm_time = MPI_Wtime();
	}

	MPI_Scatter(p_work_size, 1,	MPI_INT, &work_size, 1,	MPI_INT, 0,	MPI_COMM_WORLD); //Режем и отправляем количество работы

	

	int *rec_point_buf = new int[work_size * 2];

	MPI_Scatterv(point_buffer, double_p_work_size, disp, MPI_INT, rec_point_buf, work_size * 2, MPI_INT, 0, MPI_COMM_WORLD);

	// Принимаем данные, с которыми будем работать



	int *shell_buf = NULL;

	int shell_count;

	vector<int> out_index;

	// Проводим первичные построения оболочек, если данных не ноль
	if (work_size != 0)
	{

		
		shell_count = new_shell(rec_point_buf, work_size, out_index);

	

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

	



	// Первичные оболочки построены. Теперь мы должны слить все данные, переодически перестраивая оболочки на новых данных.
	// Вычислим в каждом, не лишний ли он?
	// Т.е. сколько процессов попадают под степень двойки


	int right_num_proc = 1;
	
	while ( right_num_proc * 2 <= numtasks )
	{
		right_num_proc *= 2;
	}

	
	// В каждом процессе надо понять, будет ли он отдавать себя как хвост? Т.е. что он не входит в степень двойки

	bool  send_tail = false;

	if (taskid >= right_num_proc) send_tail = true;



	// Сколько же всего хвостов?
	int tails_count = numtasks - right_num_proc;


	// А какие будут принимать хвосты?
	bool rec_tail = false;
	if (taskid < tails_count) rec_tail = true;

	


	// Так, осталось принять и отдать. Понять кому принять и кому отдать.

	int send_to, rec_from;

	int send_count, rec_count;

	int *union_buffer = NULL;

	if (send_tail) // Отправка хвостов
	{

		send_to = taskid - right_num_proc;

		send_count = shell_count;

		MPI_Send(&send_count, 1, MPI_INT, send_to, 8080, MPI_COMM_WORLD);
		MPI_Send(shell_buf, shell_count * 2, MPI_INT, send_to, 8081, MPI_COMM_WORLD);

	}

	if (rec_tail) // Прием хвостов
	{
		
		MPI_Status st1; MPI_Status st2;
		rec_from = right_num_proc + taskid;
	
		MPI_Recv(&rec_count, 1, MPI_INT, rec_from, 8080, MPI_COMM_WORLD, &st1);
		
	
		
		union_buffer = new int[(rec_count + shell_count) * 2];



		MPI_Recv(union_buffer, rec_count * 2, MPI_INT, rec_from, 8081, MPI_COMM_WORLD, &st2);


		for (int i = rec_count; i < shell_count + rec_count; i++)
		{
			union_buffer[i * 2] = shell_buf[i * 2 - rec_count * 2];
			union_buffer[i * 2 + 1] = shell_buf[i * 2 - rec_count * 2 + 1];
		}


		

		shell_count = shell_count + rec_count;
	}
	
	
	// отлично, теперь бы надо все стандартизировать. Так что назовем все массивы одинаково
	if ( !(rec_tail || send_tail)) // т.е. ничего не делали
	{
		union_buffer = shell_buf;
	}


	// Таак, теперь в каждом процессе есть shell_count - количество элементов в оболочке и union_buffer это оболочка. И все данные лежат в 2^n первых процессов

	int level = 0;
	bool live = true; // Не отдал ли процесс свои данные?

	

	while ((1 << level) < right_num_proc) // Пока не дошли до определенной степени двойки
	{

		bool for_send = false;
		bool for_rec = false;

		int mask = 1 << level;

		int k, k1;




		if (live && taskid < right_num_proc && numtasks > 1)
		{
			// Как понять, принимаем мы или пересылаем данные? Хмм, ну, посомтрим бит номер level. Если 1, то шлем, если нет, то принимаем.


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



				// Объединим текущий и пришедщий буферы
				for (int i = 0; i < k; i++)
				{
					tmp_name[i * 2 + k1 * 2] = union_buffer[i * 2];
					tmp_name[i * 2 + k1 * 2 + 1] = union_buffer[i * 2 + 1];
				}

				
				
				shell_count = k + k1;
				
				//Буфферы можно снова сократить

				if (shell_count > 0)
				{

					vector<int> index;
					
					shell_count = new_shell(tmp_name, shell_count, index);

					

					union_buffer = new int[shell_count * 2];

					for (int i = 0; i < shell_count; i++)
					{
						union_buffer[i * 2] = tmp_name[index[i] * 2];
						union_buffer[i * 2 + 1] = tmp_name[index[i] * 2 + 1];
					}
					// Заодно везде возвращаем нормально название

					// Вот и привели все в порядок. Редуцировали точки
				}

			}

		}

		level += 1;
	}



	if (taskid == 0)
	{




		if (shell_count > 0)
		{

			vector<int> index;

			shell_count = new_shell(union_buffer, shell_count, index);

			// Проводим последнее редуцирование, на случай, если процесс всего и был-то один

			int *	tmp_name = new int[shell_count * 2];

			for (int i = 0; i < shell_count; i++)
			{
				tmp_name[i * 2] = union_buffer[index[i] * 2];
				tmp_name[i * 2 + 1] = union_buffer[index[i] * 2 + 1];
			}
			union_buffer = tmp_name;
			
		}

		end_programm_time = MPI_Wtime();
		// Конец работы парралельной части программы
		
		// Вывод данных
		string st;

		st = "Shell:\n ";
		for (int i = 0; i < shell_count; i++)
		{
			st += "(" + to_string(union_buffer[i * 2]) + ", " + to_string(union_buffer[i * 2 + 1]) + ") ";
		}
		st += "\n\n";
		cout << st;


	}

	//  Check to equal on one process
	

	if (taskid == 0)
	{

		vector<int> index_n;

		int one_shell_count;


		double spent_time_one_proc;

		double str_sh_one_pr;
		double en_sh_one_pr;
		str_sh_one_pr = MPI_Wtime();

		one_shell_count = new_shell(point_buffer, all_data_count, index_n);

		en_sh_one_pr = MPI_Wtime();
		spent_time_one_proc = en_sh_one_pr - str_sh_one_pr;

		int *one_shell_buffer;

		one_shell_buffer = new int[one_shell_count * 2];

		for (int i = 0; i < shell_count; i++)
		{
			one_shell_buffer[i * 2] = point_buffer[index_n[i] * 2];
			one_shell_buffer[i * 2 + 1] = point_buffer[index_n[i] * 2 + 1];
		}

		
		if (one_shell_count == shell_count)
		{

			if (equal_arrays(one_shell_buffer, union_buffer, one_shell_count))
			{
				cout << "Answer on one process is equal to answer on " << numtasks <<  " process\n";
			}
			else
			{

				cout << "Answers is not equal!!!\n";
			}
		}
		else
		{
			cout << "Answers is not equal!!!\n";
		}
		cout << "Time to build shell on N = " << numtasks << " process including data transfer: " << end_programm_time - start_programm_time << "\n";
		cout << "Time to build shell on one process = " << spent_time_one_proc << "\n";
	

	}





	MPI_Finalize();

	return 0;
}