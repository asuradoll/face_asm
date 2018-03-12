#pragma once
#pragma execution_character_set("utf-8")

#ifndef SHAPE_VEC_H
#define SHAPE_VEC_H

#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp> 
#include "cv.h"
#include <vector>
#include"simtrans.h"
using cv::Mat_;

namespace modelshare {
	class ShapeVec : public Mat_< double > {
	public:
		ShapeVec() {}

		ShapeVec(const Mat_< double > &a) :Mat_< double >(a) {}
		//�̳���Ĺ��캯��
		ShapeVec & operator =(const Mat_< double > &a) {
			Mat_< double >::operator=(a);
			return *this;
		}//������
		void alignTo(const ShapeVec & ref);
		//��ref�������룬�ؼ�����

		void doScale(double r) {
			(*this) *= r;
		}
		//����������

		void scaleToOne() {
			doScale(1 / norm(*this));
		}
		//��һ��

		void doTranslate(double vX, double vY)
		{
			for (int i = 0; i < rows / 2; i++)
				(*this)(i, 0) += vX;
			for (int i = rows / 2; i < rows; i++)
				(*this)(i, 0) += vY;
		}

		void zeroGravity() {
			doTranslate(-getXMean(), -getYMean());
		}//��ȥ��ֵ���������Ƶ�ԭ��

		double getXMean() const { return mean(rowRange(0, rows / 2))[0]; }
		//0��rows/2�е�һ��Ԫ�صľ�ֵ
		double getYMean() const { return mean(rowRange(rows / 2, rows))[0]; }
		//rows/2��rows�е�һ��Ԫ�صľ�ֵ

		double X(int i) const { return (*this)(i, 0); }
		double & X(int i) { return (*this)(i, 0); }
		//ͬ�ʣ�����mat�ģ�i��0����Ԫ�أ���ַ

		double Y(int i) const { return (*this)(i + (rows >> 1), 0); }
		double & Y(int i) { return (*this)(i + (rows >> 1), 0); }

		//! Do a similarity transform and restore it into a list of points.
		void restoreToPointList(
			vector< Point_< int > > &v,//vector����Ϊ�������ǿ
			const simtrans &st
		);

		//! Flatten a list of points to the current shape vector.
		void fromPointList(const vector< Point_<int> > &v);

		Rect_<double> getBoundRect();
		//! Number of points for this Shape������������һλ�൱�ڳ�2
		int nPoints() const {
			return (rows >> 1);
		}

		simtrans getShapeTransformFitingSize(
			const Size &rect,
			double scaleRatio = 0.9, double xOffset = 0, double yOffset = 0);
	};
}
#endif  