

#include "GaussImageProcessor.h"
#include <iostream>
#include "tbb/task_scheduler_init.h"
#include "tbb/task.h"
#include "opencv2/opencv.hpp"
#include <vector>
#include <string>

using namespace cv;

GaussImageProcessor::GaussImageProcessor(Mat& _originalImage, int _chunk)
{
        originalImage = _originalImage;
        if (_chunk > 0)
            chunk = _chunk;
        else chunk = -1;
        //imshow( "GaussImageProcessor_or", originalImage);
        core = generate_core(3, 1);
}

Mat GaussImageProcessor::execute_gauss()
{
    outImage = originalImage.clone();

        partition();

        tbb::task::spawn_root_and_wait(tasks);

        return outImage;
}


void GaussImageProcessor::set_core(int size, double sigma)
{
     if (size <= 0)
        throw std::string("negative core size!");

    core = generate_core(size, sigma);
}

void GaussImageProcessor::partition()
{
    int rows;
        int cols;

        rows = originalImage.rows;
        cols = originalImage.cols;

        int thread_num = tbb::task_scheduler_init::default_num_threads();

        if (chunk > 0)
        {
            int chunk_count = rows / chunk;
            for (int i = 0; i < chunk_count; i++)
            {
                ImageProcessorPart& ip = *new (tbb::task::allocate_root()) ImageProcessorPart(originalImage, outImage, i * chunk, i * chunk + chunk, core);
                tasks.push_back(ip);
                //std::cout << "create part [" << i * chunk << "; " << i * chunk + chunk <<  ")" << "\n";
            }
            
            cout << "chun count " << chunk_count << "\n";
            ImageProcessorPart& ip = *new (tbb::task::allocate_root()) ImageProcessorPart(originalImage, outImage, rows - (rows % chunk), rows, core);
            tasks.push_back(ip);
            //std::cout << "create part [" << rows - (rows % chunk) << "; " << rows <<  ")" << "\n";
        }
        else
        {
            //std::cout << "Num of thread by default = " << thread_num << "\n";
            int standart_pack = rows / thread_num;
            for (int i = 0; i < thread_num - 1; i++)
            {
                ImageProcessorPart& ip = *new (tbb::task::allocate_root()) ImageProcessorPart(originalImage, outImage, i * standart_pack, i * standart_pack + standart_pack, core);
                tasks.push_back(ip);
                //std::cout << "create part [" << i * standart_pack << "; " << i * standart_pack + standart_pack <<  ")" << "\n";
            }
            ImageProcessorPart& ip = *new (tbb::task::allocate_root()) ImageProcessorPart(originalImage, outImage, standart_pack * (thread_num - 1), rows, core);
            tasks.push_back(ip);
            //std::cout << "create part [" << standart_pack * (thread_num - 1) << "; " << rows <<  ")" << "\n";
        }
        
}


ImageProcessorPart::ImageProcessorPart(Mat& _orIm, Mat& _outIm, int _begin, int _end, vector<vector<double>>& _core)
{
    originalImage = _orIm;
    outImage = _outIm;
    begin = _begin;
    end = _end;
    core = _core;
}

tbb::task* ImageProcessorPart::execute()
{
    gauss_processing(originalImage, outImage, core, begin, end);
    //std::cout << "Task #? = [" << begin << "; " << end << ")" << " was execute" << "\n";
    return NULL;
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
    vector<vector<double>> core;
	core = _core;

    for (int i = begin; i < end; i++)
			for (int j = 0; j < originalImage.cols; j++) // For all pixel
			{
				double r = 0, g = 0, b = 0;

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
}
