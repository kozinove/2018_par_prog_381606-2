#include "funcs.h"


void median_filter(Mat *source)
{
	Mat copy = source->clone();
	for (int i = 1; i < source->rows - 1; ++i)
	{
		for (int j = 1; j < source->cols - 1; ++j)
		{
			vector<uchar> red, green, blue;
			for (int k = -1; k <= 1; k++)
			{
				for (int l = -1; l <= 1; l++)
				{
					blue.emplace_back(copy.at<Vec3b>(i + k, j + l)[0]);
					green.emplace_back(copy.at<Vec3b>(i + k, j + l)[1]);
					red.emplace_back(copy.at<Vec3b>(i + k, j + l)[2]);
				}
			}
			sort(blue.begin(), blue.end());
			sort(green.begin(), green.end());
			sort(red.begin(), red.end());
			source->at<Vec3b>(i, j) = Vec3b(blue[4], green[4], red[4]);
		}
	}
}


void scatter_image(Mat source, Mat *recv_buffer, int *recvcount, int root, MPI_Comm comm, int rank, int proc_count)
{
	int cols = recv_buffer->cols;
	uchar *img_part_arr = new uchar[recvcount[rank]];
	uchar *img_arr = nullptr;
	int *displs = nullptr, i, j;

	if (rank == root)
	{
		img_arr = new uchar[source.rows * cols];
		for (i = 0; i < source.rows; i++)
			for (j = 0; j < cols; j++)
			{
				img_arr[i * cols + j] = source.at<uchar>(i, 3 * j);
			}
		displs = new int[proc_count];
		for (i = 0; i < proc_count; i++)
		{
			displs[i] = 0;
			for (j = 0; j < i; j++)
				displs[i] += recvcount[j];
		}
	}

	MPI_Scatterv(img_arr, recvcount, displs, MPI_UNSIGNED_CHAR, img_part_arr, recvcount[rank], MPI_UNSIGNED_CHAR, root, comm);

	for (i = 0; i < recvcount[rank] / cols; i++)
		for (j = 0; j < cols; j++)
			recv_buffer->at<uchar>(i, 3 * j) = recv_buffer->at<uchar>(i, 3 * j + 1) = recv_buffer->at<uchar>(i, 3 * j + 2) = img_part_arr[i * cols + j];
}


void gather_image(Mat *result_img, Mat image_part, int *recvcount, int root, MPI_Comm comm, int rank, int proc_count)
{
	int cols = image_part.cols;
	uchar *img_part_arr = new uchar[recvcount[rank]];
	uchar *res_arr = nullptr;
	int *displs = nullptr, i, j;

	for (i = 0; i < recvcount[rank] / cols; i++)
		for (j = 0; j < cols; j++)
			img_part_arr[i * cols + j] = image_part.at<uchar>(i, 3 * j);

	if (rank == root)
	{
		res_arr = new uchar[result_img->rows * cols];
		displs = new int[proc_count];
		for (i = 0; i < proc_count; i++)
		{
			displs[i] = 0;
			for (j = 0; j < i; j++)
				displs[i] += recvcount[j];
		}
	}

	MPI_Gatherv(img_part_arr, recvcount[rank], MPI_UNSIGNED_CHAR, res_arr, recvcount, displs, MPI_UNSIGNED_CHAR, root, comm);

	if (rank == root)
	{
		for (i = 0; i < result_img->rows; i++)
			for (j = 0; j < cols; j++)
				result_img->at<uchar>(i, 3 * j) = result_img->at<uchar>(i, 3 * j + 1) = result_img->at<uchar>(i, 3 * j + 2) = res_arr[i * cols + j];
	}
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
