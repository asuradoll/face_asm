#pragma once
#pragma execution_character_set("utf-8")
#ifndef ASMMODEL_H
#define ASMMODEL_H
#include "shapemodel.h"

namespace modelshare {
#define ASM_FIT_VERBOSE_NOVERBOSE 0
#define ASM_FIT_VERBOSE_AT_LEVEL 1
#define ASM_FIT_VERBOSE_AT_ITERATION 2
#define ASM_FIT_VERBOSE_AT_POINT 3

	class Asmmodel;
	class AsmFitResult : public FitResult 
	{
	private:
		Asmmodel *asmmodel;
	public :

		AsmFitResult(Asmmodel *model) : asmmodel(model) {}

		//! Get the result in landmark point list form.
		void toPointList(vector< Point_<int> > &pV) const;
	};

	class Asmmodel :public ShapeModel 
	{
	private:
		//! ÿһ���ǵ��ת��Э�������
		vector< vector< Mat_< double > > > iCovarG;

		//!ÿһ���ǵ��ƽ��ֵ������ͼ�������
		vector< vector< Mat_< double > > > meanG;

		//! parameter k for ASM
		int localFeatureRad;

		//! parameter ns for ASM
		int ns;

		PCA pcaPyr[3];
		double sigma2Pyr[3];//�в�

		//Ϊÿһ���ǵ�ľֲ����콨��ͳ��ѧģ�ͣ����ڼ������Ͼ���
		void buildLocalDiffStructure();

	public:
		//final point
		vector< Point_<int> > point_out;
		//! Empty ASM model.
		Asmmodel() :localFeatureRad(8), ns(6) {}

		//! Initialize by a file.
		Asmmodel(const string& filename) { loadFromFile(filename); }

		//Ѱ��ͼ��Ŀ�겢��Ŀ�귶Χ����asm���
		/*
		  ����opencvĿ������࣬��ÿ������������asmģ��
		  param��img ����ͼ��
				 detectedObjs  ����Ŀ��İ�ΧȦ��ͨ������ΪopencvĿ����Ľ����
				 verbose ��Ϣ���༶��
		  return������ƥ������Ϊһ������
		*/
		vector< AsmFitResult > fitAll(
			const Mat & img,
			const vector< Rect > & detectedObjs,
			int verbose = 0);

		//��ÿ��ͼ�����asm��ϣ������������
		/*
		  �Ա�asmmodel::fit��������ʹ��fitall����
		  param��img ͼ��飬����Ŀ����þ���ĸ���Ȥ��ͼ������
				 verbose ��Ϣ���༶��
				 ����ams��Ͻ��
		*/
		AsmFitResult fit(const Mat &img, int verbose = 0);

		//�ҵ�������״����Ѳ����任
		/*
		param ��ShapeVec ��״����
				fitResult asm���
		*/
		void findParamForShape(const ShapeVec &Y, AsmFitResult & fitResult);


		//! Build the model
		void buildModel(const string& shapeDefFile, const string& ptsListFile) {

			ShapeModel::buildModel(shapeDefFile, ptsListFile);

			printf("(II) Building active model...\n");
			buildLocalDiffStructure();
		}

		//! Save the model into a file
		void saveToFile(const string& filename) {
			ModelFile mf;
			mf.openFile(filename.c_str(), "wb");
			saveToFile(mf);
			mf.closeFile();
		}

		//! Load the model from a file
		void loadFromFile(const string& filename)
		{
			ModelFileAscii mf;
			mf.openFile(filename.c_str(), "rb");
			loadFromFile(mf);
			mf.closeFile();
		}
		//! Show the result in the image
		
		void showResult(Mat &img, const vector< AsmFitResult > &res) {
			Mat mb;
			if (img.channels() == 1)
				cvtColor(img, mb, CV_GRAY2RGB);
			else
				mb = img.clone();

			ShapeVec sv;
			for (uint i = 0; i < res.size(); i++) {
				vector< Point_<int> > V;
				res[i].toPointList(V);//�����ɷ�ͶӰ
				shapeInfo.drawMarkPointsOnImg(mb, V, true);
				point_out = V;
			}

			if (!img.empty())
				imshow("result show", mb);
			return ;
		}
		void Save_ImgMark(Mat &img)
		{

		}
	private:
		//ʹ�ñ�Ҷ˹����ģ���ҳ�������״����Ѳ����任
		/*!
		\param l level in image pyramid.
		*/
		void findParamForShapeBTSM(const ShapeVec &Y, const ShapeVec &Y_old,
			AsmFitResult & fitResult, AsmFitResult &b_old, int l);

		//! Save the model into a file
		void saveToFile(ModelFile &file)
		{
			ShapeModel::saveToFile(file);

			file.writeInt(localFeatureRad);
			file.writeInt(ns);

			int i, j;
			int rows, cols;
			file.writeInt(rows = iCovarG[0][0].rows);
			file.writeInt(cols = iCovarG[0][0].cols);
			for (i = 0;i <= pyramidLevel;i++) {
				for (j = 0;j < nMarkPoints;j++) {
					for (int ii = 0;ii < rows;ii++)
						for (int jj = 0;jj < cols;jj++)
							file.writeReal(iCovarG[i][j](ii, jj));
				}
			}

			file.writeInt(rows = meanG[0][0].rows);
			file.writeInt(cols = meanG[0][0].cols);
			for (i = 0;i <= pyramidLevel;i++) {
				for (j = 0;j < nMarkPoints;j++) {
					for (int ii = 0;ii < rows;ii++)
						for (int jj = 0;jj < cols;jj++)
							file.writeReal(meanG[i][j](ii, jj));
				}
			}
		}


		//! Load the model from a file
		void loadFromFile(ModelFile &file)
		{
			ShapeModel::loadFromFile(file);
			printf("Loading ASM model from file...\n");

			file.readInt(localFeatureRad);
			file.readInt(ns);

			int i, j;
			int rows, cols;
			file.readInt(rows);
			file.readInt(cols);
			iCovarG.resize(pyramidLevel + 1);
			for (i = 0;i <= pyramidLevel;i++) {
				iCovarG[i].resize(nMarkPoints);
				for (j = 0;j < nMarkPoints;j++) {
					iCovarG[i][j].create(rows, cols);
					for (int ii = 0;ii < rows;ii++)
						for (int jj = 0;jj < cols;jj++)
							file.readReal(iCovarG[i][j](ii, jj));
				}
			}

			file.readInt(rows);
			file.readInt(cols);
			meanG.resize(pyramidLevel + 1);
			for (i = 0;i <= pyramidLevel;i++) {
				meanG[i].resize(nMarkPoints);
				for (j = 0;j < nMarkPoints;j++) {
					meanG[i][j].create(rows, cols);
					for (int ii = 0;ii < rows;ii++)
						for (int jj = 0;jj < cols;jj++)
							file.readReal(meanG[i][j](ii, jj));
				}
			}

			// Prepare BTSM pyramid
			double curSigma2 = 0;
			for (i = 0; i < pcaFullShape->eigenvalues.rows; i++) {
				curSigma2 += pcaFullShape->eigenvalues.at<double>(i, 0);
			}

			// Layer 2, 5 parameter
			for (i = 0; i < 5 && i < pcaFullShape->eigenvalues.rows; i++) {
				curSigma2 -= pcaFullShape->eigenvalues.at<double>(i, 0);
			}
			sigma2Pyr[2] = curSigma2 / (nMarkPoints * 2 - 4);
			pcaPyr[2].eigenvalues = pcaFullShape->eigenvalues.rowRange(0, i);
			pcaPyr[2].eigenvectors = pcaFullShape->eigenvectors.rowRange(0, i);
			pcaPyr[2].mean = pcaFullShape->mean;

			// Layer 1, 20 parameter
			for (; i < 20 && i < pcaFullShape->eigenvalues.rows; i++) {
				curSigma2 -= pcaFullShape->eigenvalues.at<double>(i, 0);
			}
			sigma2Pyr[1] = curSigma2 / (nMarkPoints * 2 - 4);
			pcaPyr[1].eigenvalues = pcaFullShape->eigenvalues.rowRange(0, i);
			pcaPyr[1].eigenvectors = pcaFullShape->eigenvectors.rowRange(0, i);
			pcaPyr[1].mean = pcaFullShape->mean;

			sigma2Pyr[0] = sigma2;
			/*pcaPyr[2] = pcaPyr[1]= */pcaPyr[0] = *pcaShape;
		}
	};
}




#endif // !ASMMODEL_H