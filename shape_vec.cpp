#pragma once
#pragma execution_character_set("utf-8")
#include "shape_vec.h"

namespace modelshare {

	void ShapeVec::alignTo(const ShapeVec & ref)
	{
		static simtrans trans;
		trans.setTransformByAlign(*this, ref);
		trans.transform(*this, *this);
		doScale(1 / this->dot(ref));
	}

	void ShapeVec::restoreToPointList(vector< Point_< int > > &v,
		const simtrans &st) {

		v.resize(this->nPoints());//size=rows/2
		ShapeVec sv;
		st.transform(*this, sv);
		for (int i = 0;i<nPoints();i++) {
			v[i].x = sv.X(i);
			v[i].y = sv.Y(i);
		}
	 }

	void ShapeVec::fromPointList(const vector< Point_<int> > &v) {

		this->create(v.size() * 2, 1);//size=rows/2
		for (int i = 0; i < (rows >> 1); i++) {
			(*this)(i, 0) = v[i].x;
			(*this)(i + (rows >> 1), 0) = v[i].y;
		}
	}

	Rect_<double> ShapeVec::getBoundRect() {
		int nP = nPoints();
		double minX = 1e10, minY = 1e10, maxX = -1e10, maxY = -1e10;
		double x, y;
		for (int i = 0;i<nP;i++) {
			x = X(i);
			y = Y(i);
			if (x<minX) minX = x;
			if (x>maxX) maxX = x;
			if (y<minY) minY = y;
			if (y>maxY) maxY = y;
		}
		return Rect_< double >(
			Point_<double>(minX, minY),
			Size_<double>(maxX - minX, maxY - minY));
	}

	simtrans ShapeVec::getShapeTransformFitingSize(
		const Size &rect,
		double scaleRatio , double xOffset, double yOffset ) {

		Rect_<double> bdRect = getBoundRect();
		
		double ratioX, ratioY, ratio;
		ratioX = rect.width / bdRect.width;
		ratioY = rect.height / bdRect.height;
		if (ratioX < ratioY)
			ratio = ratioX;
		else
			ratio = ratioY;

		double transX, transY;
		ratio *= scaleRatio;
		transX = bdRect.x - bdRect.width*(ratioX / ratio - 1 + xOffset) / 2;
		transY = bdRect.y - bdRect.height*(ratioY / ratio - 1 + yOffset) / 2;
		
		simtrans st;
		st.a = ratio;
		st.b = 0;
		st.Xt = -transX*ratio;
		st.Yt = -transY*ratio;
		return st;
	}
}