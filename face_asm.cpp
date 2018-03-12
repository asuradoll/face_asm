// face_asm.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>  
#include <string>
#include "asmmodel.h"
#include <opencv2\objdetect\objdetect.hpp>
#include "gui.h"
#include "base function.h"

using namespace cv;
using namespace std;
using namespace modelshare;

void BuildAsmModel(Asmmodel &asmModel, string shapeDef, string ptsList, string modelPath)
{
	asmModel.buildModel(shapeDef, ptsList);
	asmModel.saveToFile(modelPath);
}

void readAsmModel(Asmmodel &asmModel, string modelPath)
{
	asmModel.loadFromFile(modelPath);
}

void searchAndFit(Asmmodel &asmModel,  CascadeClassifier &objCascadeClassfifier, const string &picPath, int verbosel)
{
	Mat img = imread(picPath);
	if (img.empty()) {
		cerr << "Load image" << picPath << "failed." << endl;
		exit(2);
		}
	Mat Gray;
	cvtColor(img, Gray, CV_BGR2GRAY);
	//利用opencv自带分类器识别脸部区域
	vector<Rect> Faces;
	Faces.resize(2);
	equalizeHist(Gray, Gray);
 	objCascadeClassfifier.detectMultiScale(Gray, Faces, 1.1, 2, 0 | CV_HAAR_FIND_BIGGEST_OBJECT|CV_HAAR_SCALE_IMAGE, Size(30,30));
	vector<Rect> Face1(1);
	Face1[0] = Faces[0];																																																																																																																																													
	vector<AsmFitResult> fitResult = asmModel.fitAll(img, Face1, verbosel);
	asmModel.showResult(img, fitResult);
	cvWaitKey();
}

void  asmOnWebCam(Asmmodel &asmModel, CascadeClassifier &objCascadeClassfifier, int verbosel) {

	Mat img, imgT;
	VideoCapture capture;
	capture.open(0);
	if (!capture.isOpened()) {
		cerr << "Failed to open your webcam" << endl;
		exit(2);
	}
	while (cvWaitKey(5) == -1) {
		capture >> imgT;
		flip(imgT, img, 1);//flipping around y-axis
		vector<Rect> faces;
		objCascadeClassfifier.detectMultiScale(img, faces, 1.2, 2, CV_HAAR_FIND_BIGGEST_OBJECT | CV_HAAR_SCALE_IMAGE, Size(60, 60));

		vector<AsmFitResult>fitResult = asmModel.fitAll(img, faces, verbosel);
		asmModel.showResult(img, fitResult);
	}
}
void showHelp()
{
	cerr << "Arguments: " << endl
		<< "===========" << endl
		<< "Tasks: " << endl
		<< "   -b : Build a model!" << endl
		<< "   -v : View a model." << endl
		<< "   -f : Fit images to a model." << endl
		<< endl
		<< "General options:" << endl
		<< "   -m <path_to_model> : Path to the model file." << endl
		<< "   -V <number>        : Verbose level." << endl
		<< endl
		<< "Build specific options:" << endl
		<< "   -d <def_file>  : Model definition file, see wiki." << endl
		<< "   -l <list_file> : List of labelled points, see wiki." << endl
		<< endl
		<< "Fitting specific options: " << endl
		<< "   -C <detector_xml> : Face/Object detector XML (OpenCV)." << endl
		<< "   -p <image>        : Path to an image" << endl
		<< "   -pc               : Run on your webcam!" << endl
		<< endl;
}
void input_message(int &num_str, string str_input[])
{
	string cmd;
	cout << "Please read help and input the command" << endl;
	getline(cin, cmd);
	string str_inputcpy = cmd;
	int length = str_inputcpy.size(),j=0,single_str;
	for (int i = 0;i < length;i++) {
		if (cmd[i] == ' ' )
		{
			single_str = i - j ;
			str_input[num_str] = str_inputcpy.substr(j,single_str);
			num_str++;
			j = i + 1;
		}
		else if (cmd[i + 1] == '\0')
		{
			single_str = i - j + 1;
			str_input[num_str] = str_inputcpy.substr(j, single_str);
			num_str++;
			j = i + 1;
		}
	}
}
int main()
{
	string modelFile, modelType, action;
	string ptsDefFile, ptsListFile, picPath;
	string faceCasecadePath = "haarcascade_frontalface_alt.xml";
	string str_input[10];
	int verbosel = 0;
	char ch;
	int num_str = 0;

	opterr = 0;
	cout << "this is the help list" << endl;
	showHelp();
	input_message(num_str, str_input);
	while ((ch = getopt(num_str, str_input, "cV:p:C:m:t:d:l:a:vbf?")) != EOF) {
		switch (ch)
		{//选项初始化
		case'm':
			//得到模型文件path
			modelFile = optarg;
			break;
		case'V':
			//设置verbose日志级别
			verbosel = optarg.size();
			break;
		case'v':
			//浏览模型
			action = "view";
			break;
		case'b':
			//创建模型
			action = "build";
			break;
		case'f':
			//使用模型拟合图像
			action = "fit";
			break;
		case'd':
			//标定定义file
			ptsDefFile = optarg;
			break;
		case'l':
			//pts文件列表
			ptsListFile = optarg;
			break;
		case'C':
			faceCasecadePath = optarg;
			break;
		case'p':
			picPath = optarg;
			break;
		default:
			break;
		}
	}
	if (action == "") {
		showHelp();
		cerr << "You have to specify a task!" << endl;
		exit(1);
	}
	if (modelFile == "") {
		showHelp();
		cerr << "You have to specify a task!" << endl;
		exit(1);
	}

	Asmmodel asmModel;
	if (action == "build") {
		if (ptsDefFile == "") {
			showHelp();
			cerr << "You have to specify a task!" << endl;
			exit(1);
		}
		if (ptsListFile == "") {
			showHelp();
			cerr << "You have to specify a task!" << endl;
			exit(1);
		}
		BuildAsmModel(asmModel, ptsDefFile, ptsListFile, modelFile);
	}
	else
	{
		readAsmModel(asmModel, modelFile);
		if (action == "view") {
 			asmModel.viewShapeModel();
			cvWaitKey();
		}
		else if (action == "fit")
		{
			if (picPath == "") {
				showHelp();
				cerr << "You have to specify a task!" << endl;
				exit(1);
			}

			Mat img, imgT;
			CascadeClassifier faceCascade;
			if (!faceCascade.load(faceCasecadePath)) {
				showHelp();
				cerr << "Now, a (face) detector is needed. "
					<< "Please check the URL above."
					<< endl;
				exit(1);
			}
			if (picPath == "c")
				asmOnWebCam(asmModel, faceCascade, verbosel);
			else
				searchAndFit(asmModel, faceCascade, picPath, verbosel);
		}
	}
	return 0;
}