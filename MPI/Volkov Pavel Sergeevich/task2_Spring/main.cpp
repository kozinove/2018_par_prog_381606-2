
#include <iostream>
#include "tbb/task_scheduler_init.h"
#include "tbb/task.h"
#include "opencv2/opencv.hpp"
#include <vector>
#include <string>
#include "GaussImageProcessor.h"
#include "tbb/tick_count.h"

using namespace cv;
using namespace std;


int main(int argc, char *argv[])
{
   
    for (int i = 0; i < argc; i++)
        cout << argv[i] << "\n";

    Mat img = imread("image.png", CV_LOAD_IMAGE_COLOR);

    Mat out_image;

    GaussImageProcessor gauss(img, 50);
    gauss.set_core(10, 3);

    for (int i = 0; i < 8; i++)
    {
        tbb::task_scheduler_init in(tbb::task_scheduler_init::deferred);

        in.initialize(i + 1);

        cout << "tr_num = " << i + 1 << "\n";

        tbb::tick_count t0;
        tbb::tick_count t1;

        t0 = tbb::tick_count::now();

        imwrite("im_" + to_string(i + 1) + ".png" , gauss.execute_gauss());

        t1 = tbb::tick_count::now();
        cout << "time = " << (t1-t0).seconds() << "\n\n";

        in.terminate();
    }

    return 0;
}
