#define _USE_MATH_DEFINES
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <omp.h>
#include <vector>
#include <time.h>


using namespace std;
using namespace cv;

bool read_img(string img_src, Mat &ret)
{

	ret = imread(img_src, CV_LOAD_IMAGE_COLOR);

	if (!ret.data)
	{
		return false;
	}
	return true;	
}

int clamp(int min, int max, int val)
{
	if (val < min) return min;

	if (val > max) return max;

	return val;
}



vector<vector<double>> generate_core(int wd, double _sigma)
{
	vector<vector<double>> res;

	double sigma = _sigma;
	

	if (wd % 2 == 0) wd += 1;

	vector<double> tmp;

	for (int i = 0; i < wd; i++)
		tmp.push_back(0);

	for (int i = 0; i < wd; i++)
		res.push_back(tmp);


	double mean = wd / 2;
	double sum = 0.0; // For accumulating the kernel values
	for (int x = 0; x < wd; ++x)
		for (int y = 0; y < wd; ++y) {
			res[x][y] = exp(-0.5 * (pow((x - mean) / sigma, 2.0) + pow((y - mean) / sigma, 2.0)))
				/ (2 * M_PI  * sigma * sigma);

			// Accumulate the kernel values
			sum += res[x][y];
		}

	// Normalize the kernel
	for (int x = 0; x < wd; ++x)
		for (int y = 0; y < wd; ++y)
			res[x][y] /= sum;

	return res;

};



void gauss_filtration(Mat &src_img, Mat &out_img, vector<vector<double>> _core)
{


	vector<vector<double>> core;// = { {1,2,1}, {2,4,2}, {1,2,1} };

	
	core = _core;

	out_img = src_img.clone();

	


	#pragma omp parallel shared(out_img, src_img)
	{
		//cout << "Im enter\n";
		int thread_nums = omp_get_num_threads();

		int start_pos;
		int end_pos;

		int my_id = omp_get_thread_num();
		int standart_block;

		standart_block = src_img.rows / thread_nums;


		// Find the barrier of img parts
		if (my_id < thread_nums - 1)
		{
			start_pos = standart_block * my_id;
			end_pos = start_pos + standart_block;
		}
		else
		{
			start_pos = standart_block * (thread_nums - 1);
			end_pos = src_img.rows;
		}



		//string log;
		//log = "pid = " +  to_string(my_id) + " st = [" + to_string(start_pos) +  " en = " + to_string(end_pos) +  ") \n";
		//cout << log;


		for (int i = start_pos; i < end_pos; i++)
			for (int j = 0; j < src_img.cols; j++) // For all pixel
			{
				double r = 0, g = 0, b = 0;
				//r = src_img.at<Vec3b>(i, j)[0];

				for (int row = 0; row < core.size(); row++)
					for (int col = 0; col < core.size(); col++)
					{
						int row_coord;
						int col_coord;
						row_coord = i - core.size() / 2 + row;
						col_coord = j - core.size() / 2 + col;
						row_coord = clamp(0, src_img.rows - 1 , row_coord);
						col_coord = clamp(0, src_img.cols - 1, col_coord);
						//string log = to_string(my_id) + " row_coord = " + to_string(row_coord) + " col_coord = " + to_string(col_coord) + "\n";
						//cout << log;

						double tr, tg, tb;

						//tr = src_img.at<Vec3b>(row_coord, col_coord)[2];
						//tg = src_img.at<Vec3b>(row_coord, col_coord)[1];
						//tb = src_img.at<Vec3b>(row_coord, col_coord)[0];


						//string plus = to_string(tr) + " " + to_string(tg) + " " + to_string(tb) + " row = " + to_string(row_coord) + " col = " + to_string(col_coord) + "\n";
						//cout << plus;

						r += src_img.at<Vec3b>(row_coord, col_coord)[2] * core[row][col];
						g += src_img.at<Vec3b>(row_coord, col_coord)[1] * core[row][col];
						b += src_img.at<Vec3b>(row_coord, col_coord)[0] * core[row][col];
					}

				//r = r / 16;
				//g = g / 16;
				//b = b / 16;


				//string rgb = to_string(r) + " " + to_string(g) + " " + to_string(b) + "\n";
				//cout << rgb;
				out_img.at<Vec3b>(i, j)[2] = r;
				out_img.at<Vec3b>(i, j)[1] = g;
				out_img.at<Vec3b>(i, j)[0] = b;
				//string log = "rgb = " +  to_string(r) + " " + to_string(g) + " " + to_string(b) + "\n";
				//cout << log;

			}

		//string log_2 = "pid n = " +  to_string(my_id) + " is out of parralel\n";
		//cout << log_2;
		if (my_id == thread_nums - 1) cout << "Gauss called on " << my_id + 1 << " threads\n";
	}
	
	
}





int main()
{
	string src_img;

	cin >> src_img;

	Mat src;
	Mat src_copy;
	Mat out;

	src = imread(src_img, CV_LOAD_IMAGE_COLOR);

	src_copy = src;

	if (src.data) cout << "data is ok\n";


	double sigma;

	int n;

	cout << "Enter size of gauss core <int>: ";
	cin >> n;
	cout << "Enter sigma for generate gauss core <double>: ";
	cin >> sigma;


	vector<vector<double>> gauss_core;

	gauss_core = generate_core(n, sigma);

	clock_t  start, end;
	double delta = 0;

	omp_set_num_threads(6);



	//firs calling
	start = clock();
	

	
	gauss_filtration(src, out, gauss_core);

	end = clock();
	delta = (double(end - start) / CLOCKS_PER_SEC);

	cout << "time on n = " << delta << "\n";




	// saving
	int last_dot;

	for (int i = 0; i < src_img.length(); i++)
	{
		if (src_img[i] == '.') last_dot = i;
	}

	string out_folder = src_img.substr(0, last_dot);
	string out_folder_tmp = out_folder;




	out_folder += "_new_gauss.png";
	out_folder_tmp += "_on_one_thread.png";

	cout << "out path = " << out_folder << "\n";

	imwrite(out_folder, out);




	//second calling

	Mat sec_out;

	omp_set_num_threads(1);

	start = clock();


	gauss_filtration(src, sec_out, gauss_core);

	end = clock();

	delta = (double(end - start) / CLOCKS_PER_SEC);

	cout << "time on 1 = " << delta << "\n";

	imwrite(out_folder_tmp, sec_out);



	return 0;
}