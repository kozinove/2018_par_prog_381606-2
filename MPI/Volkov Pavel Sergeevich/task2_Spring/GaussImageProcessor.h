
#include "opencv2/opencv.hpp"
#include "tbb/task_scheduler_init.h"
#include "tbb/task.h"
#include <vector>

using namespace std;
using namespace cv;


//Class for planing and execute gauss processing
class GaussImageProcessor
{
    public:
        GaussImageProcessor(Mat& _originalImage, int _chunk = -1);

        Mat execute_gauss();

        void set_core(int size, double sigma);


    private:
        Mat originalImage;
        Mat outImage;

        tbb::task_list tasks;
        int chunk;
        vector<vector<double>> core;
        

        void partition();
};


//Class for save task
//Consist originalImage and outImage link. And rows, which need process
// In execute calling:
// void gauss_processing(Mat& originalImage, Mat& outImage, vector<vector<double>> _core, int begin, int end);
class ImageProcessorPart : public tbb::task
{
    private:
        Mat originalImage;
        Mat outImage;

        int begin, end; // rows
        vector<vector<double>> core;

    public:
        ImageProcessorPart(Mat _orIm, Mat _outIm, int _begin, int _end, vector<vector<double>>& _core);
    
    tbb::task* execute();
};


vector<vector<double>> generate_core(int wd, double _sigma); // guass core generating func


int clamp(int min, int max, int val);


void gauss_processing(Mat& originalImage, Mat& outImage, vector<vector<double>> _core, int begin, int end); 
//Execute gauss processing to row [begin, end) on originalImage image and seve is outImage