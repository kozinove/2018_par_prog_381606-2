

#include "opencv2/opencv.hpp"

using namespace cv;


bool parse_argument(Mat& src_mat, int& gauss_core_size, double& gauss_core_sigma, int& thread_num, int argc, char *argv[])
{
   // cout << argc << "\n";

    //for (int i = 0; i < argc; i++)
   // {
  //      cout << argv[i] << "\n";
   // }


    if (!(argc == 4 || argc == 5))
    {
        cout << "use argument: (string)<image path> (U_int)<gauss_core_size> (U_int)<sigma_in_core> (int)[thread_num]\n";
        return false;
    }


    string path;

    // try read and parse parametr

    path = argv[1];
    src_mat = imread(path, CV_LOAD_IMAGE_COLOR);

    if (!src_mat.data)
    {
        cout << "no such image file or directory\n";
        cout << "use argument: (string)<image path> (U_int)<gauss_core_size> (U_int)<sigma_in_core> (int)[thread_num]\n";
        return false;
    }

    try
    {
        gauss_core_size = stoi(argv[2]);

        if (gauss_core_size <= 0)
            throw -1;
            
    }
    catch(...)
    {
        cout << "cnat parse gauss_core_size\n";
        cout << "use argument: (string)<image path> (U_int)<gauss_core_size> (U_int)<sigma_in_core> (int)[thread_num]\n";
        return false;
    }
    
    try
    {
        gauss_core_sigma = stod(argv[3]);
        if (gauss_core_sigma <= 0)
            throw -1;

    }
    catch(...)
    {
        cout << "cnat parse gauss_sigma\n";
        cout << "use argument: (string)<image path> (U_int)<gauss_core_size> (U_int)<sigma_in_core> (int)[thread_num]\n";
        return false;
    }

    if (argc == 5)
    {
        try
        {
            thread_num = stoi(argv[4]);
            if (thread_num <= 0)
                throw -1;

        }
        catch(...)
        {
            cout << "cnat parse thread_num\n";
            cout << "use argument: (string)<image path> (U_int)<gauss_core_size> (U_int)<sigma_in_core> (int)[thread_num]\n";
            return false;
        }
    }
    else
    {
        thread_num = -1;
    }

    cout << "\n";
    cout << "Ok parsing log:\n";
    cout << "Argc == " << argc << "\n";
    cout << "Image path == " << path << "\n";
    cout << "Core size == " << gauss_core_size << "\n";
    cout << "Core sigma == " << gauss_core_sigma << "\n";
    cout << "Thread num == " << thread_num << "\n";

    return true;

}