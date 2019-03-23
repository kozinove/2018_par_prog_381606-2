#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <cmath>
#include <string>

using namespace cv;
using namespace std;

int clamp(int in, int min, int max)
{
	if (in < min)
		return min;
	if (in > max)
		return max;
	return in;
}

void createKernel(int size, float sigma, double**& kernel) {
	int radius = size / 2;
	kernel = new double*[size];
	double norm = 0; //коэффициент нормировки
	for (int i = 0; i < size; i++) {
		kernel[i] = new double[size];
	}
	for (int i = -radius; i <= radius; i++)
	{
		for (int j = -radius; j <= radius; j++)
		{
			kernel[i + radius][j + radius] = (float)exp(-(i*i + j*j) / (sigma * sigma));
			norm += kernel[i + radius][j + radius];
		}

	}
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			kernel[i][j] /= norm;
		}
	}
}

Mat gaussFilter(int size, float sigma, Mat sourceImage) {
	Mat resultImage(sourceImage.size(), CV_8UC3);
	double **gaussKernel;
	int radius = size / 2;
	createKernel(size, sigma, gaussKernel);
	double B = 0, G = 0, R = 0;
	int idX = 0, idY = 0;
	for (int x = 0; x < sourceImage.rows; x++)
	{
		for (int y = 0; y < sourceImage.cols; y++)
		{
			B = 0;
			G = 0;
			R = 0;
			for (int l = -radius; l <= radius; l++)
			{
				for (int k = -radius; k <= radius; k++)
				{
					idX = clamp(x + k, 0, sourceImage.rows - 1);
					idY = clamp(y + l, 0, sourceImage.cols - 1);
					Vec3b neighborColor = sourceImage.at<Vec3b>(idX, idY);
					B += neighborColor.val[0] * gaussKernel[k + radius][l + radius];
					G += neighborColor.val[1] * gaussKernel[k + radius][l + radius];
					R += neighborColor.val[2] * gaussKernel[k + radius][l + radius];
				}
			}
			Vec3b color(clamp(B, 0, 255), clamp(G, 0, 255), clamp(R, 0, 255));
			resultImage.at<Vec3b>(x, y) = color; 
		}
	}
	for (int i = 0; i < size; i++)
		delete[] gaussKernel[i];
	delete[] gaussKernel;
	return resultImage;
}

int main(int argc, char *argv[])
{

	int kernelSize = 3;
	float sigma = 2; //параметр фильтра
	cv::Size kSize(3, 3);
	String sourceFile = "testImage.jpg";
	/*if (argc >= 3) {
		sourceFile = argv[0];
		kernelSize = atoi(argv[1]);
		sigma = atof(argv[2]);
	}*/
	Mat sourceImage = imread(sourceFile, IMREAD_COLOR);
	Mat cvGauss(sourceImage.size(), CV_8UC3);
	Mat myGauss(sourceImage.size(), CV_8UC3);
	if (sourceImage.empty())
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}
	GaussianBlur(sourceImage, cvGauss, kSize, sigma,0,BORDER_DEFAULT);

	imwrite("myGauss.jpg", gaussFilter(kernelSize, sigma, sourceImage));
	imwrite("cvGauss.jpg", cvGauss);
	return 0;
}