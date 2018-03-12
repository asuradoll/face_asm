#pragma once
#pragma execution_character_set("utf-8")
#ifndef MODELIMAGE_H
#define MODELIMAGE_H

#include "shape_vec.h"
#include "shapeinfo.h"
#include <string>
#include <vector>

namespace modelshare {
	//! Image and image related operations.
	class ModelImage {
	protected:
		// The number of landmark points.
		int nMarkPoints;

		// Optional 'host image' filename including full path.
		string hostImageName;

		// the training image
		Mat imgdata;

		// Image pyramid
		vector< Mat > imgPyramid;
		vector< Mat_<double> > imgPyrGrayDeriv;
		vector< Mat_<uchar> > imgPyrGray;

		// Is the image loaded?
		bool imgLoaded;

		// Information about shapes and paths.
		ShapeInfo *shapeInfo;

	public:
		ModelImage();

		// Landmark points
		vector< Point_< int > > points;

		// Shape vectors
		ShapeVec shapeVec;

		// Load training image from saved host image
		bool loadTrainImage();

		// Load training image
		bool loadTrainImage(const Mat &img);

		void setShapeInfo(ShapeInfo *si) { shapeInfo = si; }

		// Release Training Image
		bool releaseTrainImage();

		// Get local structure
		/*!将得到的-k到k个点转换为2k*1矩阵，并归一化
		\param pId id of the point
		\param k how many points to get on either direction
		\param level position in the pyramid level
		\param step VERY IMPORTANT, for a image with area of 10000, 1.0 may be a good choice
		*/
		Mat_< double > getLocalStruct(int pId, int k, int level, double step);

		// Get the coordinates of points at normal direction of a landmark point
		/*!搜索在前后点连线过的法线过当前点方向-k到k个点保存在v中
		\param pId id of the point
		\param k how many points to get on either direction
		\param level position in the pyramid level
		\param V the vector that save results.
		\param step VERY IMPORTANT, for a image with area of 10000, 1.0 may be a good choice
		\param pOffset when searching for best points, use the p'th  point along the profile as the center
		*/
		void getPointsOnNorm(int pId, int k, int level,
			vector< Point_< int > > &V,
			double step, int pOffset = 0);

		//! Get the image saved at specified level
		Mat & getTrainImage(int level = 0, bool gray = false);

		// Return the derivative image at specified level;
		Mat & getDerivImage(int level) { return imgPyrGrayDeriv[level]; }

		// Host image (if any).
		inline const string &HostImage() const { return hostImageName; }

		void buildFromShapeVec(simtrans &trans);

		// Read mark points information from a PTS file
		bool readPTS(const char * filename);

		// Set mark points information from a vector of points
		void initPointsByVector(const vector< cv::Point2i > &V);

		//获取protected成员
		void setHostImage(const char * hostImageFilename) {
			hostImageName = hostImageFilename;
		}
		
		// The number of shape points.
		inline int NPoints() const { return nMarkPoints; }

		//! Show the image interactively
		Mat show(int level = 0, int pId = -1, bool showInWin = true, int highLight = 0);
	};
}
#endif // !MODELIMAGE_H