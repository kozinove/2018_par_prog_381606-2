

#include "GaussImageProcessor.h"
#include <iostream>
#include "tbb/task_scheduler_init.h"
#include "tbb/task.h"
#include "opencv2/opencv.hpp"
#include <vector>
#include <string>
#include <omp.h>

using namespace cv;

GaussImageProcessor::GaussImageProcessor(Mat& _originalImage)
{
        originalImage = _originalImage;
        //imshow( "GaussImageProcessor_or", originalImage);
        core = generate_core(3, 1);
}

Mat GaussImageProcessor::execute_gauss()
{
    outImage = originalImage.clone();

	cout << "image cloned\n";

	gauss_processing(originalImage, outImage, core, 0, originalImage.rows);

    return outImage;
}



void GaussImageProcessor::set_core(int size, double sigma)
{
     if (size <= 0)
        throw std::string("negative core size!");

    core = generate_core(size, sigma);
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

void gauss_processing(Mat& originalImage, Mat& outImage, vector<vector<double>> _core, int begin, int end)
{
    vector<vector<double>> core;// = { {1,2,1}, {2,4,2}, {1,2,1} };

	core = _core;

	outImage = originalImage.clone();

	#pragma omp parallel shared(originalImage, outImage)
	{

		int thread_nums = omp_get_num_threads();

		int start_pos;
		int end_pos;

		int my_id = omp_get_thread_num();
		int standart_block;

		standart_block = originalImage.rows / thread_nums;


		// Find the barrier of img parts
		if (my_id < thread_nums - 1)
		{
			start_pos = standart_block * my_id;
			end_pos = start_pos + standart_block;
		}
		else
		{
			start_pos = standart_block * (thread_nums - 1);
			end_pos = originalImage.rows;
		}

		for (int i = start_pos; i < end_pos; i++)
			for (int j = 0; j < originalImage.cols; j++) // For all pixel
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
						row_coord = clamp(0, originalImage.rows - 1 , row_coord);
						col_coord = clamp(0, originalImage.cols - 1, col_coord);

						double tr, tg, tb;

						r += originalImage.at<Vec3b>(row_coord, col_coord)[2] * core[row][col];
						g += originalImage.at<Vec3b>(row_coord, col_coord)[1] * core[row][col];
						b += originalImage.at<Vec3b>(row_coord, col_coord)[0] * core[row][col];
					}


				outImage.at<Vec3b>(i, j)[2] = r;
				outImage.at<Vec3b>(i, j)[1] = g;
				outImage.at<Vec3b>(i, j)[0] = b;

			}

		if (my_id == thread_nums - 1) cout << "Gauss called on " << my_id + 1 << " threads\n";
	}

}

bool check(Mat im1, Mat im2)
{
    for (int i = 0; i < im1.rows; i++)
        for (int j = 0; j < im1.cols; j++)
        {
            if (im1.at<Vec3b>(i, j)[2] != im2.at<Vec3b>(i, j)[2])
                return false;
            if (im1.at<Vec3b>(i, j)[1] != im2.at<Vec3b>(i, j)[1])
                return false;
            if (im1.at<Vec3b>(i, j)[0] != im2.at<Vec3b>(i, j)[0])
                return false;
        }
    return true;
}
