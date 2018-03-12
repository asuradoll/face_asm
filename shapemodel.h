#pragma once
#pragma execution_character_set("utf-8")
#ifndef SHAPEMODEL_H
#define SHAPEMODEL_H

#include "modelimage.h"
#include "base function.h"
#include <vector>

namespace modelshare {

	struct FitResult {
		//! Parameters for the model
		Mat_< double > params;

		//! The similarity transformation needed to recover shape
		simtrans transformation;
	};

	class ShapeModel {
	protected:
		//! PCA model for shapes.
		PCA *pcaShape;
		//! Number of eigen vectors reserved for shape model.
		int nShapeParams;

		//贝叶斯切线模型
		//! Data for BTSM: \f$\sigma^2\f$残差平方和系数
		double sigma2;

		//! Data for BTSM: Full \f$\phi\f$
		PCA *pcaFullShape;

		//! level for the image pyramid.
		int pyramidLevel;

		//! Number of landmark points in a image.
		int nMarkPoints;

		//! Number of training images.
		int nTrain;

		//! All the images, with labelled markpoints.
		vector<ModelImage> imageSet;

		//! Path info for shapes;
		ShapeInfo shapeInfo;

		//! Mean shape after aligning
		ShapeVec meanShape;

		//! r.y -= r.height*?
		double searchYOffset;

		//! r.x -= r.width*?
		double searchXOffset;

		//! r.width *= ?
		double searchWScale;
		//! r.height *= ?
		double searchHScale;

		//! step: ?*100/sqrt(area)
		double searchStepAreaRatio;

		//! init scale ratio when searching
		double searchScaleRatio;
		//! init X offset when searching
		double searchInitXOffset;
		//! init Y offset when searching
		double searchInitYOffset;

		//! Refine a parameter vector by clamping.
		void clampParamVec(Mat_< double > &paramVec) {
			// Todo: Change "3" to a configurable variable.
			for (int i = 0;i<this->nShapeParams;i++) {
				double ei = sqrt(pcaShape->eigenvalues.at<double>(i, 0));
				if (paramVec(i, 0) > 3 * ei)
					paramVec(i, 0) = 3 * ei;
				else if (paramVec(i, 0) < -3 * ei)
					paramVec(i, 0) = -3 * ei;
			}
		}

		//! Align the shapes and build a model.
		/**
		* \param shapeDefFile Shape definition file.
		* \param ptsListFile File containting a list of pts files.
		*/
		void buildModel(const string& shapeDefFile, const string& ptsListFile) {

			loadShapeInfo(shapeDefFile.c_str());
			readTrainDataFromList(ptsListFile.c_str());

			this->alignShapes();
			this->buildPCA();
		}
	public:
		// For viewing the model
		//! Used for viewing model
		struct ModelViewInfo
		{
			vector< int > vList;
			int curParam;
			void *pModel;
		};

		ShapeModel();

		//! Save the model into a file
		virtual void saveToFile(ModelFile &file);

		//! Load the model from a file
		virtual void loadFromFile(ModelFile &file);

		//! File names are stored in the list file
		void readTrainDataFromList(const char *listFileName);

		//! Load shape information
		void loadShapeInfo(const char *shapeFileName);

		//! Set the level for the image pyramid
		/*!
		\param l Image from level 0 to l will be considered during training
		and searching.
		*/
		void setPyramidLevel(int l) { pyramidLevel = l; }
	
		//! Project a parameter vector to a shape
		/*!
		\param paramVec parameter vector.
		\param shapeVec the shape corresponding to the parameter vector
		*/
		void projectParamToShape(const Mat_<double> & paramVec, ShapeVec &shapeVec) {
			this->pcaShape->backProject(paramVec, shapeVec);
		}

		//! Project a shape to a parameter vector
		/*!
		\param shapeVec the shape corresponding to the parameter vector
		\param paramVec parameter vector.
		*/
		void projectShapeToParam(const ShapeVec & shapeVec,
			Mat_<double> & paramVec)
		{
			this->pcaShape->project(shapeVec, paramVec);
		}

		//! Normalize an parameter vector(0..1)
		Mat_< double > normalizeParam(const Mat_<double> &p) {
			Mat_<double> n = p.clone();
			for (int i = 0; i<p.rows; i++)
				n(i, 0) /= 3 * sqrt(pcaShape->eigenvalues.at<double>(i, 0));
			return n;
		}//

		 //! Reconstruct parameter vector from normalized vector
		Mat_< double > reConFromNorm(const Mat_<double> &p) {
			Mat_<double> n = p.clone();
			for (int i = 0; i<p.rows; i++)
				n(i, 0) *= 3 * sqrt(pcaShape->eigenvalues.at<double>(i, 0));
			return n;
		}

		ShapeInfo & getShapeInfo() { return shapeInfo; }

		//! An interactive UI for viewing the statistical model.
		void viewShapeModel();

		//! Update viewing UI with new parameters. (called by callbacks)
		void viewShapeModelUpdate(ModelViewInfo *pInfo);
		private:
			//! Build PCA model for shapes
			void buildPCA();

			//! Align shapes iteratively
			void alignShapes();

			//! Find patterns~
			void preparePatterns();
	};
}

#endif // !SHAPEMODEL_H