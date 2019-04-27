
#include <iostream>
#include "tbb/task_scheduler_init.h"
#include "tbb/task.h"
#include "opencv2/opencv.hpp"
#include <vector>
#include <string>

using namespace cv;
using namespace std;

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

void gauss_processing(Mat originalImage, Mat outImage, vector<vector<double>> _core, int begin, int end)
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



class ImageProcessorPart : public tbb::task
{
    private:
        Mat originalImage;
        Mat outImage;

        int begin, end; // rows

    public:
        ImageProcessorPart(Mat _orIm, Mat _outIm, int _begin, int _end)
        {
            originalImage = _orIm;
            outImage = _outIm;
            begin = _begin;
            end = _end;
        }
    
    tbb::task* execute()
    {

        gauss_processing(originalImage, outImage, generate_core(10, 3), begin, end);
        std::cout << "Task #? = [" << begin << "; " << end << ")" << " was execute" << "\n";
        return NULL;
    }
};


class GaussImageProcessor
{
    public:
        GaussImageProcessor(Mat& _originalImage, int _chunk = -1)
        {
            originalImage = _originalImage;
            if (_chunk > 0)
                chunk = _chunk;
            else chunk = -1;
            imshow( "GaussImageProcessor_or", originalImage);
        }

    Mat execute_gauss()
    {

        outImage = originalImage.clone();

        partition();

        tbb::task::spawn_root_and_wait(tasks);

        return outImage;
    }
    

    private:
        Mat originalImage;
        Mat outImage;

        tbb::task_list tasks;
        int chunk;
        

    void partition()
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
                ImageProcessorPart& ip = *new (tbb::task::allocate_root()) ImageProcessorPart(originalImage, outImage, i * chunk, i * chunk + chunk);
                tasks.push_back(ip);
                std::cout << "create part [" << i * chunk << "; " << i * chunk + chunk <<  ")" << "\n";
            }
            
            ImageProcessorPart& ip = *new (tbb::task::allocate_root()) ImageProcessorPart(originalImage, outImage, rows - (rows % chunk), rows);
            tasks.push_back(ip);
            std::cout << "create part [" << rows - (rows % chunk) << "; " << rows <<  ")" << "\n";
        }
        else
        {
            std::cout << "Num of thread by default = " << thread_num << "\n";
            int standart_pack = rows / thread_num;
            for (int i = 0; i < thread_num - 1; i++)
            {
                ImageProcessorPart& ip = *new (tbb::task::allocate_root()) ImageProcessorPart(originalImage, outImage, i * standart_pack, i * standart_pack + standart_pack);
                tasks.push_back(ip);
                std::cout << "create part [" << i * standart_pack << "; " << i * standart_pack + standart_pack <<  ")" << "\n";
            }
            ImageProcessorPart& ip = *new (tbb::task::allocate_root()) ImageProcessorPart(originalImage, outImage, standart_pack * (thread_num - 1), rows);
            tasks.push_back(ip);
            std::cout << "create part [" << standart_pack * (thread_num - 1) << "; " << rows <<  ")" << "\n";
        }
    }


};

int main(int argc, char *argv[])
{

    tbb::task_scheduler_init init;

    for (int i = 0; i < argc; i++)
        cout << argv[i] << "\n";

    Mat img = imread("image.png", CV_LOAD_IMAGE_COLOR);

    Mat out_image;


    GaussImageProcessor gauss(img, 50);

    out_image = gauss.execute_gauss();

    imshow("Display window", out_image);

    waitKey(0);
    return 0;
}
