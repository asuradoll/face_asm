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
		//继承类的构造函数
		ShapeVec & operator =(const Mat_< double > &a) {
			Mat_< double >::operator=(a);
			return *this;
		}//？？？
		void alignTo(const ShapeVec & ref);
		//与ref向量对齐，关键函数

		void doScale(double r) {
			(*this) *= r;
		}
		//做放缩操作

		void scaleToOne() {
			doScale(1 / norm(*this));
		}
		//归一化

		void doTranslate(double vX, double vY)
		{
			for (int i = 0; i < rows / 2; i++)
				(*this)(i, 0) += vX;
			for (int i = rows / 2; i < rows; i++)
				(*this)(i, 0) += vY;
		}

		void zeroGravity() {
			doTranslate(-getXMean(), -getYMean());
		}//减去均值，将重心移到原点

		double getXMean() const { return mean(rowRange(0, rows / 2))[0]; }
		//0到rows/2行第一个元素的均值
		double getYMean() const { return mean(rowRange(rows / 2, rows))[0]; }
		//rows/2到rows行第一个元素的均值

		double X(int i) const { return (*this)(i, 0); }
		double & X(int i) { return (*this)(i, 0); }
		//同质，返回mat的（i，0）个元素，地址

		double Y(int i) const { return (*this)(i + (rows >> 1), 0); }
		double & Y(int i) { return (*this)(i + (rows >> 1), 0); }

		//! Do a similarity transform and restore it into a list of points.
		void restoreToPointList(
			vector< Point_< int > > &v,//vector容器为数组的增强
			const simtrans &st
		);

		//! Flatten a list of points to the current shape vector.
		void fromPointList(const vector< Point_<int> > &v);

		Rect_<double> getBoundRect();
		//! Number of points for this Shape，二进制右移一位相当于除2
		int nPoints() const {
			return (rows >> 1);
		}

		simtrans getShapeTransformFitingSize(
			const Size &rect,
			double scaleRatio = 0.9, double xOffset = 0, double yOffset = 0);
	};
}
#endif  