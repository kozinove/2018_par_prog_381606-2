
#include "opencv2/opencv.hpp"
#include <vector>

using namespace std;
using namespace cv;


//Class for planing and execute gauss processing
class GaussImageProcessor
{
    public:
        GaussImageProcessor(Mat& _originalImage);

        Mat execute_gauss();

        void set_core(int size, double sigma);


    private:
        Mat originalImage;
        Mat outImage;

        vector<vector<double>> core;

};

vector<vector<double>> generate_core(int wd, double _sigma); // guass core generating func


int clamp(int min, int max, int val);


void gauss_processing(Mat& originalImage, Mat& outImage, vector<vector<double>> _core, int begin, int end); 
//Execute gauss processing to row [begin, end) on originalImage image and seve is outImage


bool check(Mat im1, Mat im2);
