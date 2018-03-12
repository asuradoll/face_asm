#include "base function.h"
namespace modelshare {

	base_function::base_function()
	{
		file_path = "C:\\Users\\guanghaoyin\\Documents\\face detect\\face demo\\face demo/pic";
	}

	base_function::~base_function()
	{

	}

	SYSTEMTIME base_function::get_localtime()
	{
		SYSTEMTIME sys;
		GetLocalTime(&sys);
		return sys;
	}
	void base_function::desk_singlepic_show(char *filename, int iscolor, int waitkey)
	{
		IplImage *img = cvLoadImage(filename, iscolor);
		cvNamedWindow("desk pic", CV_WINDOW_AUTOSIZE);
		cvShowImage("desk pic", img);
		cvWaitKey(waitkey);
		cvReleaseImage(&img);
		cvDestroyWindow("desk pic");
	}

	void base_function::camera_singlepic(char *filename, int waitkey)
	{
		VideoCapture capture(0);
		capture.open(CAP_ANY);
		if (!capture.isOpened())
			cout << "fail to open!" << endl;
		Mat frame;
		capture >> frame;
		cvNamedWindow("camera pic", CV_WINDOW_AUTOSIZE);
		cvWaitKey(1);
		imshow("camera pic", frame);
		int t = cvWaitKey(waitkey);
		if (t == 's')
		{
			SYSTEMTIME sys = get_localtime();
			char imagename[1024];
			int i = sprintf(imagename, "%s", filename);
			sprintf(imagename + i, "\\Image%4d.%2d.%2d.%2d.%2d.%2d.bmp", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);
			imwrite(imagename, frame);
		}

		cvDestroyWindow("camera pic");
		return;
	}

	void ModelFile::writePCA(const PCA* p)
	{
		int i, j;
		int rows, cols;
		rows = p->eigenvectors.rows;
		cols = p->eigenvectors.cols;
		writeInt(rows);
		writeInt(cols);
		for (i = 0;i < rows;i++)
			for (j = 0;j < cols;j++)
				writeReal(p->eigenvectors.at<double>(j, i));
		for (i = 0;i < rows;i++)
			writeReal(p->eigenvalues.at<double>(i, 0));

		for (i = 0;i < cols;i++)
			writeReal(p->mean.at<double>(i, 0));
	}

	PCA * ModelFile::readPCA(PCA * &p)
	{
		int i, j;
		p = new PCA();
		int rows, cols;
		readInt(rows);
		readInt(cols);
		p->eigenvectors = Mat_<double>::zeros(rows, cols);
		for (i = 0;i < rows;i++)
			for (j = 0;j < cols;j++)
				readReal(p->eigenvectors.at<double>(i, j));
		p->eigenvalues = Mat_<double>::zeros(rows, 1);
		for (i = 0;i < rows;i++)
			readReal(p->eigenvalues.at<double>(i, 0));

		p->mean = Mat_<double>::zeros(cols, 1);
		for (i = 0;i < cols;i++) {
			readReal(p->mean.at<double>(i,0));
		}
		return p;
	}
}