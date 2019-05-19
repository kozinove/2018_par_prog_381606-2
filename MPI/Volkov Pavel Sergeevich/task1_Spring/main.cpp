#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <omp.h>
#include <vector>
#include <time.h>
#include "GaussImageProcessor.h"
#include "standartArgumentParser.h"

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
	Mat srcImg;
    Mat outImg;
    int gauss_core_size;
    double gauss_core_sigma;
    int thread_num;


    //parse argument
    if (!parse_argument(srcImg, gauss_core_size, gauss_core_sigma, thread_num, argc, argv))
    {
        return 0;
    }

	//init parralel
	if (thread_num != -1)
	{
		omp_set_num_threads(thread_num);
	}

    //execute process
    GaussImageProcessor g(srcImg);
    g.set_core(gauss_core_size, gauss_core_sigma);

    imwrite("out_gauss_img.png", g.execute_gauss());

	return 0;
}