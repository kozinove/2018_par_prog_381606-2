
#include <iostream>
#include "tbb/task_scheduler_init.h"
#include "tbb/task.h"
#include "opencv2/opencv.hpp"
#include <vector>
#include <string>
#include "GaussImageProcessor.h"

using namespace cv;
using namespace std;


int main(int argc, char *argv[])
{

    tbb::task_scheduler_init init;

    for (int i = 0; i < argc; i++)
        cout << argv[i] << "\n";

    Mat img = imread("image.png", CV_LOAD_IMAGE_COLOR);

    Mat out_image;


    GaussImageProcessor gauss(img);

    gauss.set_core(10, 3);

    out_image = gauss.execute_gauss();

    imshow("Display window", out_image);

    waitKey(0);
    return 0;
}
