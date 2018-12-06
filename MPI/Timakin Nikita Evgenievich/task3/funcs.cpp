#include "funcs.h"


Vec3b calculate_color(Mat sourceImage, int x, int y)
{
	int x_kernel[3][3] = { {-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1} };
	int y_kernel[3][3] = { {-1, -2, -1}, {0, 0, 0}, {1, 2, 1} };

	int rx = 0, gx = 0, bx = 0, ry = 0, gy = 0, by = 0;

	for (int i = -1; i <= 1; i++)
		for (int j = -1; j <= 1; j++)
		{
			Vec3b neighborColor = sourceImage.at<Vec3b>(i + x, j + y);

			rx += neighborColor[2] * x_kernel[j + 1][i + 1];
			gx += neighborColor[1] * x_kernel[j + 1][i + 1];
			bx += neighborColor[0] * x_kernel[j + 1][i + 1];

			ry += neighborColor[2] * y_kernel[j + 1][i + 1];
			gy += neighborColor[1] * y_kernel[j + 1][i + 1];
			by += neighborColor[0] * y_kernel[j + 1][i + 1];
		}

	return Vec3b((uchar)sqrt((bx * bx + by * by)), (uchar)sqrt((gx * gx + gy * gy)), (uchar)sqrt((rx * rx + ry * ry)));
}


void sobel_filter(Mat *source)
{
	Mat copy = source->clone();

	for (int x = 1; x < source->rows - 1; ++x)
		for (int y = 1; y < source->cols - 1; ++y)
			source->at<Vec3b>(x, y) = calculate_color(copy, x, y);
}


void scatter_image(Mat source, Mat *recv_buffer, int *recvcount, int root, MPI_Comm comm, int rank, int proc_count)
{
	int cols = recv_buffer->cols;
	uchar *b_img_part_arr = new uchar[recvcount[rank]], *g_img_part_arr = new uchar[recvcount[rank]], *r_img_part_arr = new uchar[recvcount[rank]];
	uchar *b_img_arr = nullptr, *g_img_arr = nullptr, *r_img_arr = nullptr;
	int *displs = nullptr, i, j;

	if (rank == root)
	{
		b_img_arr = new uchar[source.rows * cols];
		g_img_arr = new uchar[source.rows * cols];
		r_img_arr = new uchar[source.rows * cols];
		for (i = 0; i < source.rows; i++)
			for (j = 0; j < cols; j++)
			{
				b_img_arr[i * cols + j] = source.at<Vec3b>(i, j)[0];
				g_img_arr[i * cols + j] = source.at<Vec3b>(i, j)[1];
				r_img_arr[i * cols + j] = source.at<Vec3b>(i, j)[2];
			}
		displs = new int[proc_count];
		for (i = 0; i < proc_count; i++)
		{
			displs[i] = 0;
			for (j = 0; j < i; j++)
				displs[i] += recvcount[j];
		}
	}

	MPI_Scatterv(b_img_arr, recvcount, displs, MPI_UNSIGNED_CHAR, b_img_part_arr, recvcount[rank], MPI_UNSIGNED_CHAR, root, comm);
	MPI_Scatterv(g_img_arr, recvcount, displs, MPI_UNSIGNED_CHAR, g_img_part_arr, recvcount[rank], MPI_UNSIGNED_CHAR, root, comm);
	MPI_Scatterv(r_img_arr, recvcount, displs, MPI_UNSIGNED_CHAR, r_img_part_arr, recvcount[rank], MPI_UNSIGNED_CHAR, root, comm);

	for (i = 0; i < recvcount[rank] / cols; i++)
		for (j = 0; j < cols; j++)
			recv_buffer->at<Vec3b>(i, j) = Vec3b(b_img_part_arr[i*cols + j], g_img_part_arr[i*cols + j], r_img_part_arr[i*cols + j]);

	delete[] displs, b_img_part_arr, g_img_part_arr, r_img_part_arr, b_img_arr, g_img_arr, r_img_arr;
}


void gather_image(Mat *result_img, Mat image_part, int *recvcount, int root, MPI_Comm comm, int rank, int proc_count)
{
	int cols = image_part.cols;
	uchar *b_img_part_arr = new uchar[recvcount[rank]], *g_img_part_arr = new uchar[recvcount[rank]], *r_img_part_arr = new uchar[recvcount[rank]];
	uchar *b_res_arr = nullptr, *g_res_arr = nullptr, *r_res_arr = nullptr;
	int *displs = nullptr, i, j;

	for (i = 0; i < recvcount[rank] / cols; i++)
		for (j = 0; j < cols; j++)
		{
			b_img_part_arr[i * cols + j] = image_part.at<Vec3b>(i, j)[0];
			g_img_part_arr[i * cols + j] = image_part.at<Vec3b>(i, j)[1];
			r_img_part_arr[i * cols + j] = image_part.at<Vec3b>(i, j)[2];
		}

	if (rank == root)
	{
		b_res_arr = new uchar[result_img->rows * cols];
		g_res_arr = new uchar[result_img->rows * cols];
		r_res_arr = new uchar[result_img->rows * cols];
		displs = new int[proc_count];
		for (i = 0; i < proc_count; i++)
		{
			displs[i] = 0;
			for (j = 0; j < i; j++)
				displs[i] += recvcount[j];
		}
	}

	MPI_Gatherv(b_img_part_arr, recvcount[rank], MPI_UNSIGNED_CHAR, b_res_arr, recvcount, displs, MPI_UNSIGNED_CHAR, root, comm);
	MPI_Gatherv(g_img_part_arr, recvcount[rank], MPI_UNSIGNED_CHAR, g_res_arr, recvcount, displs, MPI_UNSIGNED_CHAR, root, comm);
	MPI_Gatherv(r_img_part_arr, recvcount[rank], MPI_UNSIGNED_CHAR, r_res_arr, recvcount, displs, MPI_UNSIGNED_CHAR, root, comm);

	if (rank == root)
	{
		for (i = 0; i < result_img->rows; i++)
			for (j = 0; j < cols; j++)
				result_img->at<Vec3b>(i, j) = Vec3b(b_res_arr[i*cols + j], g_res_arr[i*cols + j], r_res_arr[i*cols + j]);
	}

	delete[] displs, b_img_part_arr, g_img_part_arr, r_img_part_arr, b_res_arr, g_res_arr, r_res_arr;
}


string find_name(string name)
{
	string slash = name.rfind("\\") != string::npos ? "\\" : "/";
	size_t position = name.rfind(slash) + 1;
	string result = "";
	for (int i = position; i < name.length(); i++)
		result += name[i];
	return result;
}

