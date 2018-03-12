#pragma once
#pragma execution_character_set("utf-8")

#include "shapemodel.h"
#include "reader.h"
#include <cstdio>

namespace modelshare {

	ShapeModel::ShapeModel()
	{
		/// \todo Make the "3" here a constant or a configurable variable.
		pyramidLevel = 3;
	}

	void ShapeModel::buildPCA()
	{
		// PCA
		int i, j;
		int vD = imageSet[0].shapeVec.rows;
		Mat_<double> pca_data;
		pca_data.create(vD, nTrain);//默认图片大小相同
		for (i = 0;i<nTrain;i++) {
			for (j = 0;j<vD;j++)
				pca_data(j, i) = (imageSet[i].shapeVec)(j, 0);
		}//存入pac_data
		printf("(II) Calculating PCA of shape vectors.\n");

		pcaShape = new PCA(pca_data, Mat_<double>(), CV_PCA_DATA_AS_COL, 0);
		/*pca初始化，第一个参数为要进行PCA变换的输入Mat；
		参数2为该Mat的均值向量；
		参数3为输入矩阵数据的存储方式，如果其值为CV_PCA_DATA_AS_ROW则说明输入Mat的每一行代表一个样本，同理当其值为CV_PCA_DATA_AS_COL时，代表输入矩阵的每一列为一个样本；
		最后一个参数为该PCA计算时保留的最大主成分的个数。如果是缺省值，则表示所有的成分都保留。
		*/
		double eigValueSum, sCur;
		eigValueSum = sum(pcaShape->eigenvalues)[0];
		sCur = 0;
		printf("(II) PCA Rows: %d, Var: %lf\n",
			pcaShape->eigenvalues.rows, eigValueSum);

		//printf("Total Data: %d\n", pcaShape->eigenvalues.rows);
		//     for (i=0; i<pcaShape->eigenvalues.rows; i++){
		//         printf("%d: %g\n", i, pcaShape->eigenvalues.at<double>(i, 0));}

		for (i = 0;i<pcaShape->eigenvalues.rows && i<40;i++) {
			sCur += pcaShape->eigenvalues.at<double>(i, 0);
			//         printf("%d: %g\n", i, pcaShape->eigenvalues.at<double>(i, 0));
			if (sCur>eigValueSum*0.98)
				break;//取0.98系数，若大于则代表pca模型相关性稳定，已经能代表整个样本的98%以上
		} 

		// Prepare for the BTSM
		this->sigma2 = (eigValueSum - sCur) / (vD - 4);//西伽马残差平方和系数
		printf("sssiggg: %g\n", sigma2);
		this->pcaFullShape = new PCA();
		pcaFullShape->eigenvalues = pcaShape->eigenvalues.clone();
		pcaFullShape->eigenvectors = pcaShape->eigenvectors.clone();
		pcaFullShape->mean = pcaShape->mean.clone();

		if (i<pcaShape->eigenvalues.rows)//i=40或求和大于0.98系数比的i值
			nShapeParams = i + 1;
		else
			nShapeParams = i;
		pcaShape->eigenvalues = pcaShape->eigenvalues.rowRange(0, nShapeParams);
		pcaShape->eigenvectors = pcaShape->eigenvectors.rowRange(0, nShapeParams);
		printf("(II) Shape Model: reserved parameters:%d, variance: %.2f%%\n", nShapeParams, 100 * sCur / eigValueSum);
	}

	void ShapeModel::alignShapes()
	{
		int i, nss;
		nss = imageSet.size();
		//开始对齐
		for (i = 0;i<nss;i++)
			imageSet[i].shapeVec.zeroGravity();//第一步，将重心全部移到原点

		ShapeVec curMean, newMean, x0;
		imageSet[0].shapeVec.scaleToOne();//向量归一化
		newMean = imageSet[0].shapeVec.clone();//完全拷贝，不共用一个内存
		x0 = newMean.clone();

		do {
			curMean = newMean.clone();
			newMean = ShapeVec::zeros(curMean.size());
			for (i = 0;i<nss;i++) {
				imageSet[i].shapeVec.alignTo(curMean);//第二步将当前特征向量与基准向量（第一个）对齐
				newMean += imageSet[i].shapeVec;//累计向量差
			}
			newMean /= nss;//第三步计算平均向量
			newMean.alignTo(x0);//第四步对齐到平均向量
			newMean.scaleToOne();//归一化

		} while (norm(curMean - newMean)>1e-10);//重复对齐运算至收敛
		meanShape = curMean;//得到最终的归一化对齐平均向量
	}

	void ShapeModel::preparePatterns()
	{
		// Fat or thin? determined by width/height
		Rect_<double> r;
		double ratio;
		Mat_<double> cParam, pt;
		Mat_<double> sumParam;
		double sumW = 0;
		sumParam = Mat_<double>::zeros(nShapeParams, 1);

		for (int i = 0; i < nTrain; i++) {
			r = imageSet[i].shapeVec.getBoundRect();//获取限制矩阵
			projectShapeToParam(imageSet[i].shapeVec, cParam);//将imageSet[i].shapeVec主成分投影到cparam上
			ratio = r.width / r.height;//长宽比
			if (ratio > 0.7)
			{
				pt = cParam * (ratio - 0.7);
				sumParam += pt;
				sumW += ratio - 0.7;
			}
		}
		sumParam /= sumW;
		sumParam = normalizeParam(sumParam);
		printf("Fat & Thin: ");
		for (int i = 0; i<nShapeParams; i++)
			printf("%g, ", sumParam(i, 0));
		printf("\n");

		// Eye Size 眼球标定点为27 28 29 30
		sumW = 0;
		sumParam = Mat_<double>::zeros(nShapeParams, 1);
		Point_<double> v;
		double l1, l2;
		for (int i = 0; i < nTrain; i++) {
			v = (imageSet[i].points[27] - imageSet[i].points[29]);
			l1 = v.dot(v);//27to29 length^2
			v = (imageSet[i].points[28] - imageSet[i].points[30]);
			l2 = v.dot(v);//28to30 length^2
			ratio = sqrt(l1*l2);
			projectShapeToParam(imageSet[i].shapeVec, cParam);
			printf("%f\n", ratio);
			if (ratio>350) {
				pt = cParam * (ratio - 350);
				sumParam += pt;
				sumW += ratio - 350;
			}
		}
		sumParam /= sumW;
		sumParam = normalizeParam(sumParam);
		printf("eyeSize: ");
		for (int i = 0; i<nShapeParams; i++)
			printf("%g, ", sumParam(i, 0));
		printf("\n");
	}

	void ShapeModel::saveToFile(ModelFile &file)
	{
		file.writeInt(pyramidLevel);
		file.writeInt(nMarkPoints);
		file.writeInt(nTrain);
		file.writeInt(nShapeParams);

		file.writeReal(searchYOffset);
		file.writeReal(searchXOffset);
		file.writeReal(searchWScale);
		file.writeReal(searchHScale);
		file.writeReal(searchStepAreaRatio);

		file.writeReal(searchScaleRatio);
		file.writeReal(searchInitXOffset);
		file.writeReal(searchInitYOffset);

		file.writePCA(pcaShape);
		for (int i = 0;i<nMarkPoints * 2;i++)
			file.writeReal(meanShape(i, 0));

		// Info for BTSM
		file.writeReal(sigma2);
		file.writePCA(pcaFullShape);

		shapeInfo.writeToFile(file);
	}

	void ShapeModel::loadFromFile(ModelFile &file)
	{
		printf("Loading Shape model from file...\n");
		file.readInt(pyramidLevel);
		file.readInt(nMarkPoints);
		file.readInt(nTrain);
		file.readInt(nShapeParams);

		file.readReal(searchYOffset);
		file.readReal(searchXOffset);
		file.readReal(searchWScale);
		file.readReal(searchHScale);
		file.readReal(searchStepAreaRatio);

		file.readReal(searchScaleRatio);
		file.readReal(searchInitXOffset);
		file.readReal(searchInitYOffset);

		// PCA shape model
		file.readPCA(pcaShape);

		meanShape.create(nMarkPoints * 2, 1);
		for (int i = 0;i<nMarkPoints * 2;i++)
			file.readReal(meanShape(i, 0));

		// Info for BTSM
		file.readReal(sigma2);
		file.readPCA(pcaFullShape);

		shapeInfo.readFromFile(file);
	}

	void ShapeModel::readTrainDataFromList(const char *listFileName)
	{
		// Find the directory of the list file
		string sName(listFileName), listDir;
		int posD;
		posD = sName.find_last_of("/\\");//找到最后出现/或\的位置，为得到文件名做准备
		if (posD != string::npos)//找到，即表明为根目录
			listDir = sName.substr(0, posD + 1);
		else//未找到，即在当前目录下
			listDir = "./";

		FILE *fp = fopen(listFileName, "r");
		if (fp == NULL) {
			printf("ERROR! list file %s not found!!", listFileName);
			throw("ERROR! list file not found!!");
		}

		printf("Reading data from %s...\n", listFileName);
		ModelImage *ss;
		char sBuf[300];
		int l;
		string ptsPath;
		//以文件流结束为判断依据
		while (!feof(fp)) {
			char * nk = fgets(sBuf, 300, fp);//一次取300
			l = strlen(sBuf);
			if (nk>0 && sBuf[l - 1] == '\n')
				sBuf[l - 1] = 0;//0替换换行
			if (nk == 0 || sBuf[0] == 0)
				continue;
			if (sBuf[0] == '/')
				ptsPath = sBuf;
			else
				ptsPath = listDir + sBuf;//得到一个pts文件路径

			ss = new ModelImage();
			ss->readPTS(ptsPath.data());//.data读取string没有\0  按路径读取一组特征点信息包括总数，各点xy坐标
			ss->setShapeInfo(&shapeInfo);//访问protect数据成员变量
			this->imageSet.push_back(*ss);//将得到的modelimage数据插入imageset保存
			delete ss;
		}
		//更新
		this->nTrain = imageSet.size();
		this->nMarkPoints = imageSet[0].NPoints();
		fclose(fp);
	}

	void ShapeModel::loadShapeInfo(const char* shapeFileName)
	{
		printf("Loading shape info from %s\n", shapeFileName);
		reader shapeDefFile(shapeFileName);//文件流打开
		FILE *fp = shapeDefFile.FL();

		//载入特征点信息
		nMarkPoints = shapeInfo.loadFromShapeDescFile(shapeDefFile);

		// r.y -= r.height*?
		shapeDefFile.Sync();
		fscanf(fp, "%lf", &searchYOffset);
		// r.x -= r.width*?
		shapeDefFile.Sync();
		fscanf(fp, "%lf", &searchXOffset);
		// r.width *= ?
		shapeDefFile.Sync();
		fscanf(fp, "%lf", &searchWScale);
		// r.height *= ?
		shapeDefFile.Sync();
		fscanf(fp, "%lf", &searchHScale);
		// step: ?*100/sqrt(area)
		shapeDefFile.Sync();
		fscanf(fp, "%lf", &searchStepAreaRatio);

		// init scale ratio when searching
		shapeDefFile.Sync();
		fscanf(fp, "%lf", &searchScaleRatio);
		// init X offset when searching
		shapeDefFile.Sync();
		fscanf(fp, "%lf", &searchInitXOffset);
		// init Y offset when searching
		shapeDefFile.Sync();
		fscanf(fp, "%lf", &searchInitYOffset);

	}


	//! Callback function for updating value.
	void viewShapeUpdateValue(int pos, void *data)
	{
		ShapeModel::ModelViewInfo *pInfo = (ShapeModel::ModelViewInfo *)data;
		pInfo->vList[pInfo->curParam] = pos;
		((ShapeModel *)(pInfo->pModel))->viewShapeModelUpdate(pInfo);
	}

	//! Callback function for choosing a different parameter.
	void viewShapeUpdateCurParam(int pos, void *data)
	{
		ShapeModel::ModelViewInfo *pInfo = (ShapeModel::ModelViewInfo *)data;
		pInfo->curParam = pos;
		cvSetTrackbarPos("param value", "Viewing Shape Model",
			pInfo->vList[pos]);
	}

	void ShapeModel::viewShapeModelUpdate(ModelViewInfo *pInfo)
	{
		Mat_< double > paramV;
		paramV.create(this->nShapeParams, 1);
		for (int i = 0;i<nShapeParams;i++) {
			paramV(i, 0) = (pInfo->vList[i] / 30.0 - 0.5) * 6 *
				sqrt(pcaShape->eigenvalues.at<double>(i, 0));
		}
		Mat_<Vec3b> img;
		ModelImage s;
		s.setShapeInfo(&shapeInfo);
		Mat m = Mat_<unsigned char>::ones(190 * 2, 160 * 2) * 255;
		s.loadTrainImage(m);
		//s.loadTrainImage(Mat_<unsigned char>::ones(190 * 2, 160 * 2) * 255);
		projectParamToShape(paramV, s.shapeVec);
		simtrans st = s.shapeVec.getShapeTransformFitingSize(
			Size(320, 380));
		s.buildFromShapeVec(st);
		img = s.show(0, -1, false);
		imshow("Viewing Shape Model", img);
	}

	void ShapeModel::viewShapeModel()
	{
		int q1, q2;
		static ModelViewInfo vData;
		vData.vList.resize(this->nShapeParams, 30 / 2);
		vData.pModel = this;
		vData.curParam = 0;
		viewShapeModelUpdate(&vData);
		q1 = 15;
		q2 = 0;
		namedWindow("Viewing Shape Model", CV_WINDOW_AUTOSIZE);
		createTrackbar("param value", "Viewing Shape Model",
			&q1, 30, &viewShapeUpdateValue, &vData);
		createTrackbar("which param", "Viewing Shape Model",
			&q2, nShapeParams - 1, &viewShapeUpdateCurParam, &vData);
	}

}