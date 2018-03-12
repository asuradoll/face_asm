#pragma once
#pragma execution_character_set("utf-8")
#include "simtrans.h"
#include "shape_vec.h"

namespace modelshare {
	void simtrans::transform(const ShapeVec &src, ShapeVec &dst) const {
		int np = src.nPoints();//rows/2
		double xt, yt;
		dst.create(np << 1, 1);//N x 1����
		for (int i = 0;i < np;i++)
		{
			
			xt = src.X(i);
			yt = src.Y(i);
			dst.X(i) = a * xt - b * yt + Xt;//src.x(i)
			dst.Y(i) = b * xt + a * yt + Yt;//src.y(i)
		}
		//��src��dst������任
	}
	void simtrans::invtransform(const ShapeVec &src, ShapeVec &dst) const {
		int np = src.nPoints();
		double x11, x12, x21, x22;
		x11 = a / (a*a + b*b);//1
		x12 = b / (a*a + b*b);//0
		x21 = -b/ (a*a + b*b);//0
		x22 = a / (a*a + b*b);//1
		//ϵ��Ϊ���Ԫһ�η�����õ���Ϊsrc����任��dst��ϵ��
		dst.create(np << 1, 1);
		//δΪ�������ֵ�����ڸı�ߴ�ʱ����Ϊ�������ݿ����ڴ�
		double xt, yt;
		for (int i = 0;i < np; i++)
		{
			xt = src.X(i) - Xt;
			yt = src.Y(i) - Yt;
			dst.X(i) = x11 * xt + x12 * yt;//xt=x(i)
			dst.Y(i) = x21 * xt + x22 * yt;//-yt=-y(i)
		}
	}

	void simtrans::warpImage(const Mat &imgSrc, Mat &imgDst) const {
		Mat_<double> M(2, 3);
		M << a, -b, Xt,
			 b,  a, Yt;
		warpAffine(imgSrc, imgDst, M, imgSrc.size(), INTER_LINEAR);
		/*˫���Բ�ֵ
		 [scale*cos(angle) , scale*sin(angle) , (1-scale*cos(angle))*center_x-scale*cos(angle)*center_y,
		  -scale*sin(angle), scale*cos(angle) ,  scale*sin(angle)*center_x+(1-scale*cos(angle))*center_y ]
		��a��b��xt��ytΪ1��0��0��0��[1,0,0  angle = 90�㣬scale = 1��center��0��0��
									 0,1,0]
		*/
	}

	void simtrans::warpImgBack(const Mat &imgSrc, Mat &imgDst, bool useDstSize) const {
		Mat_< double > M(2, 3), mV;
		M << a, -b, Xt,
			b, a, Yt;
		if (useDstSize)
			cv::warpAffine(imgSrc, imgDst, M, imgDst.size(), INTER_LINEAR | WARP_INVERSE_MAP);
		else
			cv::warpAffine(imgSrc, imgDst, M, imgSrc.size(), INTER_LINEAR | WARP_INVERSE_MAP);
	}

	void simtrans::setTransformByAlign(const ShapeVec &x, const ShapeVec &xp) {
		int nP = x.rows / 2;
		a = xp.dot(x) / x.dot(x);
		b = 0;
		for (int i = 0; i<nP; i++)
			b += x.X(i) * xp.Y(i) - x.Y(i)*xp.X(i);
		b /= x.dot(x);
		double xxm, xym;
		xxm = x.getXMean();
		xym = x.getYMean();
		Xt = -a * xxm + b * xym + xp.getXMean();
		Yt = -b * xxm - a * xym + xp.getYMean();
	}


}