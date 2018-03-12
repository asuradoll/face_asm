#pragma once
#pragma execution_character_set("utf-8")

#include "modelimage.h"
#include "math.h"
namespace modelshare {
	ModelImage::ModelImage()
	{
		imgPyramid.resize(4);
		imgLoaded = false;
		this->shapeInfo = NULL;
	}

	bool ModelImage::readPTS(const char * filename)
	{
		reader r(filename);

		if (!r.FlValid()) {
			printf("File %s not found!\n");
			throw "";
			return false;
		}
		int npoints,c;
		r.Sync();
		do {
			c = fgetc(r.FL());
		} while (c<'0' || c>'9');
		ungetc(c, r.FL());
		fscanf(r.FL(), "%i", &npoints);//���ļ����ж�ȡ��Ҫȡ�õ���������
		fgetc(r.FL());
		r.Sync();//�����������룬��Ҫ�ٴΰ��޹��ַ�ȥ��
		
		this->nMarkPoints = npoints;

		// resize this shape
		this->points.resize(npoints);

		// input point data
		int ix, iy;
		int i;
		for (i = 0;i<npoints;i++) {
			fscanf(r.FL(), "%d %d/t", &ix, &iy);
			r.Sync();
			// save point data
			points[i].x = ix;
			points[i].y = iy;
		}
		shapeVec.fromPointList(points);

		// File name of the host image is the same as the name pts file
		string sFname(filename);
		this->hostImageName = sFname.substr(0, sFname.rfind('.')) + ".jpg";
		FILE *fp = fopen(hostImageName.data(), "rb");
		if (!fp)
			this->hostImageName = sFname.substr(0, sFname.rfind('.'));
		else
			fclose(fp);

		return true;
	}

	void ModelImage::initPointsByVector(const std::vector< cv::Point2i >& V)
	{
		this->nMarkPoints = V.size();

		this->points = V;
		this->shapeVec.fromPointList(V);
	}

	void ModelImage::buildFromShapeVec(simtrans& trans)
	{
		nMarkPoints = shapeVec.nPoints();
		shapeVec.restoreToPointList(points, trans);
	}

	bool ModelImage::loadTrainImage()
	{
		if (!imgLoaded) {
			Mat img = imread(this->hostImageName);
			if (img.empty()) {
				cerr << "(EE) Loading image " + this->hostImageName + " failed!" << endl;
				return false;
			}
			loadTrainImage(img);
		}
		return imgLoaded;
	}

	bool ModelImage::loadTrainImage(const Mat &img)
	{
		imgdata = img;
	//	imshow("a", img);
	//	cvWaitKey(0);
		//cv::buildPyramid(imgdata, this->imgPyramid, 3);//3������ͼ�������
		this->imgPyramid[0] = img;
		for (int i = 1;i < 4;i++)
		{
			pyrDown(this->imgPyramid[i-1], this->imgPyramid[i], Size(this->imgPyramid[i - 1].cols/2, this->imgPyramid[i - 1].rows/2));
			
		}
		if (imgdata.channels() == 3) {
			this->imgPyrGray.resize(4);
			for (int i = 0; i <= 3; i++) 
			{
				GaussianBlur(imgPyramid[i], imgPyramid[i], Size(3, 3), 0, 0, BORDER_DEFAULT);
				cv::cvtColor(imgPyramid[i], imgPyrGray[i], CV_BGR2GRAY);
			}
		}//rgb��ͨ����ת��Ϊ�Ҷ�ͼ
		else if (imgdata.channels() == 1)
		{
			this->imgPyrGray.resize(4);
			for (int i = 0; i <= 3; i++)
				imgPyrGray[i] = imgPyramid[i];
		}
		imgPyrGrayDeriv.resize(4);
		for (int i = 0; i <= 3; i++) {
			Mat grad_x, grad_y;
            Mat abs_grad_x, abs_grad_y;
			Mat kernel(3, 3, CV_32F, Scalar(-1));
		    imgPyrGray[i].convertTo(imgPyrGrayDeriv[i], CV_64F);//���ҶȽ�����ת��ΪCV_64F���ͣ��޳߶ȱ任��ƫ��
			GaussianBlur(imgPyrGrayDeriv[i], imgPyrGrayDeriv[i], Size(3, 3), 0, 0, BORDER_DEFAULT);
			kernel.at<float>(1, 1) = 8.9;
			filter2D(imgPyrGrayDeriv[i], imgPyrGrayDeriv[i], imgPyrGrayDeriv[i].depth(), kernel);

			cv::Sobel(imgPyrGrayDeriv[i], imgPyrGrayDeriv[i], CV_64F, 1, 0, 3, 1, 1, BORDER_DEFAULT);
			cv::Sobel(imgPyrGrayDeriv[i], imgPyrGrayDeriv[i], CV_64F, 0, 1, 3, 1, 1, BORDER_DEFAULT);

			//bilateralFilter(imgPyrGrayDeriv[i], imgPyrGrayDeriv[i], 10, 75, 75);

			cv::Sobel(imgPyrGray[i], grad_x, CV_16S, 1, 0, 3, 1, 1, BORDER_DEFAULT);//ԭ�ҶȽ�������sobel�����˲���xy�����ȡһ�׵���
			convertScaleAbs(grad_x, abs_grad_x);
			////imshow("��Ч��ͼ�� X����Sobel", abs_grad_x);
			////cvWaitKey(0);

			cv::Sobel(imgPyrGray[i], grad_y, CV_16S, 0, 1, 3, 1, 1, BORDER_DEFAULT);//ԭ�ҶȽ�������sobel�����˲���xy�����ȡһ�׵���
			convertScaleAbs(grad_y, abs_grad_y);
			////imshow("��Ч��ͼ��Y����Sobel", abs_grad_y);
			////cvWaitKey(0);

			////addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, imgPyrGrayDeriv[i]);
			////
			addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, imgPyrGray[i]);
			
			for (int m = 0;m < imgPyrGray[i].rows;m++)
				for (int n = 0;n < imgPyrGray[i].cols;n++)
					imgPyrGray[i].data[m*imgPyrGray[i].cols + n] = 255 - imgPyrGray[i].data[m*imgPyrGray[i].cols + n];

			//imshow("i", imgPyrGray[i]);			
			//cvWaitKey(0);
			//imshow("j", imgPyrGrayDeriv[i]);
		    //cvWaitKey(0);

		}
		this->imgLoaded = true;
		return imgLoaded;
	}
		
	bool ModelImage::releaseTrainImage()
	{
		if (imgLoaded) {
			imgdata.release();
			for (int i = 0; i <= 3; i++) {
				imgPyramid[i].release();
				imgPyrGray[i].release();
				imgPyrGrayDeriv[i].release();
			}
			imgLoaded = false;
		}
		return !imgLoaded;//����true �ͷųɹ�
	}

	void ModelImage::getPointsOnNorm(int pId, int k, int level,
		vector< Point_< int > > &V,
		double step, int pOffset ) {

		// Find the norm direction
		Point_< double > va, vb, vDirection;
		va = points[shapeInfo->pointInfo[pId].connectFrom] - points[pId];//ǰһ�����x,y������
		vb = points[shapeInfo->pointInfo[pId].connectTo] - points[pId];//��һ�����x,y������
		double td;
		td = norm(va);//��ǰһ����Ķ�ά���μ��ƽ��
		if (td>1e-10) va *= 1/ td;//������㹻���һ������ֹ������С��
		td = norm(vb);
		if (td>1e-10) vb *= 1/ td;//ͬ��
		//��������Ϊ��a��b������������Ϊ��-b��a��
		vDirection.x = -va.y + vb.y;//Ĭ�ϴ���һ���㵽ǰһ����Ϊ��
		vDirection.y = va.x - vb.x;//Ĭ�ϴ�ǰһ���㵽��һ����Ϊ��

		if (norm(vDirection)<1e-10) {
			if (norm(va)>1e-10)
				vDirection = -va;
			else
				vDirection.x = 1, vDirection.y = 0;
		}/*ǰ�����㵽��ǰ�������ʱ����ǰ����뵱ǰ�����㹻������Ϊ��ǰ�㵽ǰ������Ϊ����
		   ��ǰ�������뵱ǰ���������ǰ�������뵱ǰ���༫С��ֱ��ȡ��1��0��Ϊ���򣬼���x����
		*/
		else 
		{
			vDirection *= 1 / norm(vDirection);
		}
		// Find the center point, here step===1
		int i = 1, j = 1;
		int nx = 0, ny = 0;
		int offsetX, offsetY;
		int prevX = 0, prevY = 0;

		for (i = 1; i < abs(pOffset);i++)
		{
			do {
				nx = cvRound(j*vDirection.x);
				ny = cvRound(j*vDirection.y);
				j++;
			} while (nx == prevX && ny == prevY);

			prevX = nx;
			prevY = ny;
		}
		j--;
		if (pOffset>0)
			offsetX = nx, offsetY = ny;
		else
			offsetX = -nx, offsetY = -ny;
		//     offsetX = offsetY = 0;

		// Apply the "step", and find points
		vDirection *= step;
		prevX = 0;
		prevY = 0;
		nx = ny = 0;

		// Test best j =1Ϊ�������õ��ۼƵ���k�����nx��ny
		j = 1;
		for (i = 1; i <= k;i++) {
			do {
				nx = cvRound(j*vDirection.x);
				ny = cvRound(j*vDirection.y);//��������ȡ��
				j++;
			} while (nx == prevX && ny == prevY);

			prevX = nx;
			prevY = ny;
		}
		j--;//���һ����ִ�����жϣ������һ��

		//���ۼ�k�ε�nx��ny
		V.resize(2 * k + 1);
		int rX, rY;
		//����ǰ��k���㵽����k��������
		for (i = k;i >= -k;i--) {
			rX = (points[pId].x >> level) + nx + offsetX;
			rY = (points[pId].y >> level) + ny + offsetY;//��Ӧ���������������
			
			//����ͼ�����귶Χ�Ĵ���
			if (rX<0) rX = 0;
			if (rY<0) rY = 0;
			if (rX >= (imgdata.cols >> level)) rX = (imgdata.cols >> level) - 1;
			if (rY >= (imgdata.rows >> level)) rY = (imgdata.rows >> level) - 1;

			V[i + k] = Point_< int >(rX, rY);
			do {
				nx = cvRound(j*vDirection.x);
				ny = cvRound(j*vDirection.y);
				j--;
			} while (nx == prevX && ny == prevY);
			prevX = nx;
			prevY = ny;
		}
	}

	Mat_< double > ModelImage::getLocalStruct(int pId, int k, int level, double step) {

		this->loadTrainImage();
		vector< Point_< int > > pV;
		
		//���-k��k����洢��pv
		this->getPointsOnNorm(pId, k, level, pV, step);
		
		Mat_< double > diffV(2 * k + 1, 1);
		double absSum = 0;
		for (int i = k;i >= -k;i--) {
			diffV(i + k, 0) = imgPyrGrayDeriv[level](pV[i + k]);
			absSum += fabs(diffV(i + k, 0));//����ֵ���
		}
		if (absSum == 0) {
			printf("Warning: absSum=0....Level: %d, pID: %d\n", level, pId);
			show(level, pId);
		}

		else
			diffV *= 1 / absSum;

		return diffV;
	}

	Mat ModelImage::show(int l, int pId, bool showInWin, int highLight)
	{
		Mat mb;
		if (imgPyramid[0].channels() == 1)
			cvtColor(imgPyramid[0], mb, CV_GRAY2RGB);//αrgbͼ
		else
			mb = imgPyramid[0].clone();

		//��ԭ����markpoint��0��ͼ�ϱ��
		shapeInfo->drawMarkPointsOnImg(mb, points, true);

		for (int i = 0;i<nMarkPoints;i++) {
			if (pId == -1 || pId != i)
				continue;
			vector< Point_< int > > pV;
			getPointsOnNorm(i, 4, l, pV, 2, highLight);//��k��ģ�������㣬kȡ4��level1,pvΪ�ش�����������Ϊ2��ƫ����Ĭ�ϳ�ֵΪ0
			for (int j = 0; j<9; j++)
				if (highLight == 100/*j-3*/) {
					cv::circle(mb, Point_< int >(pV[j].x << l, pV[j].y << l), 3, CV_RGB(230, 100, 50),
						1, CV_AA);
				}
				else
					cv::circle(mb, Point_< int >(pV[j].x << l, pV[j].y << l), 3, CV_RGB(50, 230, 100),
						1, CV_AA);
		}
		if (showInWin) {
			cvNamedWindow("result show", CV_WINDOW_AUTOSIZE);
			cv::imshow("result show", mb);

			printf("Press any key to continue...\n");
			cv::waitKey();
		}
		return mb;
	}

	Mat & ModelImage::getTrainImage(int level, bool gray) {
		if (gray)
			return this->imgPyrGray[level];
		else
			return this->imgPyramid[level];
	}
}