
#include <iostream>
#include "tbb/task_scheduler_init.h"
#include "tbb/task.h"
#include "opencv2/opencv.hpp"
#include <vector>
#include <string>
#include "GaussImageProcessor.h"
#include "tbb/tick_count.h"
#include "standartArgumentParser.h"

using namespace cv;
using namespace std;

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


    // init tbb
    tbb::task_scheduler_init in(tbb::task_scheduler_init::deferred);

    if (thread_num == -1)
    {
        in.initialize(tbb::task_scheduler_init::automatic);
    }
    else
    {
        in.initialize(thread_num);
    }

    //execute process

    GaussImageProcessor g(srcImg);
    g.set_core(gauss_core_size, gauss_core_sigma);

    imwrite("out_gauss_img.png", g.execute_gauss());

    return 0;
}
