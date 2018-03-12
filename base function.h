#ifndef BASE FUNCTION_H
#define BASE FUNCTION_H

#include <opencv2/core/core.hpp>   
#include <opencv2/imgproc/imgproc.hpp> 
#include <opencv2/highgui/highgui.hpp>  
#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <fstream>
using namespace std;

using namespace cv;
namespace modelshare {
class base_function
{
public:
	char *file_path;
	base_function();
	~base_function();
	void desk_singlepic_show(char *filename, int iscolor,int waitkey);
	void camera_singlepic(char *filename, int waitkey);
	SYSTEMTIME get_localtime();
};

	class ModelFile//文件读写类
	{
	public:
		void writeInt(int i) { fs << i << endl; }
		int readInt(int &i) { fs >> i; return i; }

		void writeBool(bool b) { fs << b << endl; }
		int readBool(bool &b) { fs >> b; return b; }

		void writeReal(double d) { fs << d << endl; }
		double readReal(double &d) { fs >> d; return d; }

		template < class T >
		void writeMat(const Mat_<T> &m) {
			writeInt(m.rows);
			writeInt(m.cols);
			for (int i = 0;i < m.rows;i++)
				for (int j = 0;j < m.cols;j++)
					fs << m(i, j) << endl;
		}

		template < class T >
		//mat_<T>表示矩阵元素为T类型的数据
		Mat_<T> & readMat(Mat_<T> &m) {
			int r, c;
			readInt(r);
			readInt(c);
			m.create(r, c);//将m转化为数据为T类型的r行，c列的矩阵
			for (int i = 0;i < r;i++)
				for (int j = 0;j < c;j++)
					fs >> m(i, j);
			return m;
		}

		void openFile(const char *fName, const char *mode) {
			if (mode[0] == 'r')
				fs.open(fName, std::ios_base::in);
			else
				fs.open(fName, std::ios_base::out);
			if (!fs) {
				printf("Model file \"%s\" not found!!\n", fName);
				throw("");
			}
		}
		void closeFile() { fs.close(); }
		//ModelFile(){  }
		~ModelFile() { if (fs) fs.close(); }

		void writePCA(const PCA *p);
		PCA * readPCA(PCA * &p);
	private:
		fstream fs;
	};
	typedef ModelFile ModelFileAscii;
}
#endif // !BASE FUNCTION_H