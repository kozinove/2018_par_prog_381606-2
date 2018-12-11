#include "funcs.h"
#include <string>


int main(int argc, char *argv[])
{
	if (MPI_Init(&argc, &argv) != MPI_SUCCESS)
	{
		cout << "MPI initialization is failed\n" << endl;
		return 1;
	}

	double start_time = MPI_Wtime();

	int proc_count, rank, size, rem;
	const int root = 0;
	Mat img, image_part;
	int image_type, cols;
	int *recvcount = nullptr, *old_recvcount = nullptr;
	string img_name;

	MPI_Comm_size(MPI_COMM_WORLD, &proc_count);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == root)
	{
		if (argc > 1) img_name = (string)argv[1];
		else img_name = "D:\\Projects\\C++\\ParProgLab3\\ParProgLab3\\Images\\image3.jpg";

		img = imread(img_name);
		imshow("Image", img);
		size = img.rows / proc_count;
		rem = img.rows % proc_count;

		image_type = img.type();
		cols = img.cols;

		image_part = Mat(size + rem, cols, image_type);

		cout << "Image: " << img.rows << "x" << img.cols << endl;
	}

	MPI_Bcast(&rem, 1, MPI_INT, root, MPI_COMM_WORLD);
	MPI_Bcast(&size, 1, MPI_INT, root, MPI_COMM_WORLD);
	MPI_Bcast(&image_type, 1, MPI_INT, root, MPI_COMM_WORLD);
	MPI_Bcast(&cols, 1, MPI_INT, root, MPI_COMM_WORLD);

	if (rank != root) image_part = Mat(size + 2, cols, image_type);

	recvcount = new int[proc_count];
	recvcount[root] = (size + rem) * cols;
	for (int i = 1; i < proc_count; i++)
		recvcount[i] = (size + 2) * cols;

	old_recvcount = new int[proc_count];
	old_recvcount[root] = (size + rem) * cols;
	for (int i = 1; i < proc_count; i++)
		old_recvcount[i] = size * cols;

	scatter_image(img, &image_part, recvcount, old_recvcount, root, MPI_COMM_WORLD, rank, proc_count);

	sobel_filter(&image_part);

	gather_image(&img, image_part, old_recvcount, root, MPI_COMM_WORLD, rank, proc_count);

	if (rank == root)
	{
		double finish_time = MPI_Wtime();
		cout << "Total time: " << finish_time - start_time << " seconds\n";
		imwrite(string("result_") + find_name(img_name), img);
		imshow("Result", img);
		waitKey(0);
	}

	delete[] recvcount;

	MPI_Finalize();

	return 0;
}
