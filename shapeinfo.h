#pragma once
#pragma execution_character_set("utf-8")

#ifndef SHAPEINFO_H
#define SHAPEINFO_H

#include "reader.h"
#include "base function.h"
#include "shape_vec.h"

namespace modelshare {
	class ShapeInfo {
	private:
		//! Index of the first point of each contour in the landmark point list.
		vector< int > contourStartInd;
		
		/*!
		* \brief If a contour is closed or not.
		*
		* A contour can be either open (face contour) or closed (eye contour).
		*/
		vector< int > contourIsClosed;

		//! The number of contours in a shape.
		int nContours;
	public:
		//! Point in shape definition.
		struct PointInfo
		{
			int type;
			int pathId;
			int connectFrom;
			int connectTo;
		};

		//! Information for each landmark point.
		vector< PointInfo > pointInfo;

		//! Dump info to a model file.
		void writeToFile(ModelFile &f) const;

		//! Load shape info from a model file.
		void readFromFile(ModelFile &f);

		//! Load from shape description file, return the number landmark points.
		int loadFromShapeDescFile(reader &shapeDefFile);

		Mat drawMarkPointsOnImg(
			Mat &img, const vector< Point > &vP, bool drawInPlace = false) const;
	};
}
#endif // !SHAPEINFO_H