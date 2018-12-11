#pragma once
#include <opencv2/opencv.hpp>
#include <mpi.h>


using namespace std;
using namespace cv;


Vec3b calculate_color(Mat sourceImage, int x, int y);
void sobel_filter(Mat *source);
void scatter_image(Mat source, Mat *recv_buffer, int *recvcount, int *old_recvcount, int root, MPI_Comm comm, int rank, int proc_count);
void gather_image(Mat *result_img, Mat image_part, int *old_recvcount, int root, MPI_Comm comm, int rank, int proc_count);
string find_name(string name);
