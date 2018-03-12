#pragma once
#pragma execution_character_set("utf-8")

#include "asmmodel.h"

namespace modelshare {

	vector< AsmFitResult > Asmmodel::fitAll(
		const Mat & img1,
		const vector< Rect > & detectedObjs,
		int verbose) {
		Mat YUV;
		Mat img = img1.clone();
		Mat channel_YUV[3];
		vector<Mat>change_YUV(3);
		cvtColor(img, YUV, CV_RGB2YUV);
	
		split(YUV, channel_YUV);
		channel_YUV[0] *= 0.5;
		for (int i = 0;i < 3;i++)
			change_YUV[i] = channel_YUV[i].clone();
		merge(change_YUV, YUV);
		imshow("YUVchange", YUV);
		cvWaitKey(0);
		cvtColor( YUV,img, CV_YUV2BGR);
		imshow("imgchange", img);
		cvWaitKey(0);
		
		vector< AsmFitResult > fitResultV;
		for (uint i = 0;i < detectedObjs.size();i++) {
			Rect r = detectedObjs[i];
			searchYOffset *= 2;
			r.y -= r.height*searchYOffset;
			r.x -= r.width*searchXOffset;
			if (r.x<0) r.x = 0;
			if (r.y<0) r.y = 0;
			r.width *= searchWScale;
			r.height *= searchHScale;

			if (r.x + r.width>img.cols) r.width = img.cols - r.x;
			if (r.y + r.height>img.rows) r.height = img.rows - r.y;
			rectangle(img, Point(r.x,r.y),Point(r.x+r.width,r.y+r.height), Scalar(255, 0, 0), 1, 8, 0);
			//imshow("img",img);
			//cvWaitKey(0);
			//将裁减过后感兴趣部分进行拟合
			imshow("r", img(r));
			cvWaitKey(0);
			AsmFitResult fitResult = fit(img(r), verbose);
			simtrans s2;
			s2.Xt = r.x;
			s2.Yt = r.y;
			s2.a = 1;
			fitResult.transformation = s2 * fitResult.transformation;
			fitResultV.push_back(fitResult);
		}
		//showResult(img(r), fitResultV);
		//cvWaitKey(0);

		return fitResultV;
	}
	AsmFitResult Asmmodel::fit(const Mat& img, int verbose) {

		AsmFitResult fitResult(this);//this指针实例化
		Mat grayImg;//必须为灰度图像
		if (img.channels() == 3)
			cvtColor(img, grayImg, CV_BGR2GRAY);
		else
			grayImg = img;

		//调整图像大小
		Mat resizedImg;
		double ratio;
		ratio = sqrt(double(80000) / (grayImg.rows * grayImg.cols));
		resize(grayImg, resizedImg, Size(grayImg.cols*ratio, grayImg.rows*ratio));

		ModelImage curSearch;
		curSearch.setShapeInfo(&shapeInfo);
		curSearch.loadTrainImage(resizedImg);
		fitResult.params = Mat_<double>::zeros(nShapeParams, 1);//初始化参数矩阵

		ShapeVec &sv = curSearch.shapeVec;
		ShapeVec shape_old;
		projectParamToShape(fitResult.params, sv);
		//cout << "fitResult.params" << fitResult.params << "sv" << sv << endl;
		simtrans st = sv.getShapeTransformFitingSize(resizedImg.size(),
			searchScaleRatio,
			searchInitXOffset,
			searchInitYOffset);
		fitResult.transformation = st;
		curSearch.buildFromShapeVec(st);
		
		//for (int m = 0;m < curSearch.points.size();m++)
		//	resizedImg.data[curSearch.points[m].y*resizedImg.cols+ curSearch.points[m].x] = 0;
		//imshow("resizeImage", resizedImg);
		//cvWaitKey(0);
		pyramidLevel = 2;
		int k = localFeatureRad;
		ns = 10;

		//当前迭代偏移量求和
		int totalOffset;
		if (verbose >= ASM_FIT_VERBOSE_AT_LEVEL)
			curSearch.show();
		vector< Point_< int > > V;
		for (int l = this->pyramidLevel; l >= 0; l--)//3个图像金字塔 2，1，0
		{
			if (verbose >= ASM_FIT_VERBOSE_AT_LEVEL)
				printf("Level %d\n", l);
			Mat_<double> img = curSearch.getDerivImage(l);//获取l层图像
			//imshow("getDerivImage", img);
			//	cvWaitKey(0);
			int runT;
			double avgMov;
			for (runT = 0;runT < 10;runT++) 
			{
				//备份当前形状
				shape_old.fromPointList(curSearch.points);
				
				totalOffset = 0;
				vector< Point_< int > > bestEP(nMarkPoints);
				for (int i = 0;i < this->nMarkPoints;i++)
				//75个点
				{
					if (verbose >= ASM_FIT_VERBOSE_AT_POINT)
						printf("Dealing point %d...\n", i);
					Mat_<double> nrmV(2 * k + 1, 1);
					double curBest = -1, ct;
					int bestI = 0;
					double absSum;
					for (int e = ns;e > -ns;e--) {
						curSearch.getPointsOnNorm(i, k, l, V, 2 * searchStepAreaRatio, e);
						//得到i特征点附加-k到k个点,回传给v
						
						absSum = 0;
						for (int j = -k;j <= k;j++) {
							nrmV(j + k, 0) = img(V[j + k]);
							absSum += fabs(nrmV(j + k, 0));
						}
						nrmV *= 1/absSum;//归一
						//计算马氏距离
						ct=Mahalanobis(nrmV, this->meanG[l][i], this->iCovarG[l][i]);
						if (verbose >= ASM_FIT_VERBOSE_AT_POINT)
							curSearch.show(l, i, true, e);
						//更新保存最短距离点
						if (ct<curBest || curBest<0) {
							curBest = ct;
							bestI = e;
							bestEP[i] = V[k];
						}
					}
//                 printf("best e: %d\n", bestI);
//                 bestEP[i] = V[bestI+(ns+k)];

				   totalOffset += abs(bestI);
				   if (verbose >= ASM_FIT_VERBOSE_AT_POINT)
					   curSearch.show(l, i, true, bestI);
				}
				
				//更新特征点
				for (int i = 0;i<nMarkPoints;i++) {
					curSearch.points[i] = bestEP[i];
					curSearch.points[i].x <<= l;
					if (l>0) curSearch.points[i].x += (1 << (l - 1));
					curSearch.points[i].y <<= l;
					if (l>0) curSearch.points[i].y += (1 << (l - 1));
				}
				curSearch.shapeVec.fromPointList(curSearch.points);
				//cout << "curSearch.shapeVec" << curSearch.shapeVec << endl;
				if (verbose >= ASM_FIT_VERBOSE_AT_ITERATION)
					curSearch.show(l);

				//投影到主成分分析模型再返回
				//findParamForShape(curSearch.shapeVec,  fitResult);
				findParamForShapeBTSM(curSearch.shapeVec, shape_old, fitResult, fitResult, l);
				
				pcaPyr[l].backProject(fitResult.params, sv);
				//问题出在transfromation的参数上
				//重构新形状
 
				curSearch.buildFromShapeVec(fitResult.transformation);
				//curSearch.show(l);
				avgMov = (double)totalOffset / nMarkPoints;//平均偏移量
				if (verbose >= ASM_FIT_VERBOSE_AT_ITERATION) {
					printf("Iter %d:  Average offset: %.3f\n", runT + 1, avgMov);
					curSearch.show(l);
				}

				if (avgMov < 1.3) {
					runT++;
					break;
				}
			}


			if (verbose == ASM_FIT_VERBOSE_AT_LEVEL) {
				printf("%d iterations. average offset for last iter: %.3f\n", runT, avgMov);
				curSearch.show(l);
			}
		}
		simtrans s2;
		s2.a = 1 / ratio;
		fitResult.transformation = s2 * fitResult.transformation;
		return fitResult;

	}
	
	void Asmmodel::findParamForShapeBTSM(const ShapeVec &Y, const ShapeVec &Y_old,
		AsmFitResult & fitResult, AsmFitResult &b_old, int l)
	{
		const double c[3] = { 0.0005, 0.0005, 0.0005 };
		double rho2, delta2, x2;
		double p;
		ShapeVec y_r, y_rpr, xFromParam, xFromY, x;
		//cout << "Y" << Y << endl;
		ShapeVec yt = Y_old;
		yt -= Y;//两形状参数差值
		//cout << yt;
		rho2 = c[l] * yt.dot(yt);  

		simtrans curTrans = b_old.transformation;
		Mat_< double > curParam, tmpFullParam, lastParam;

		curParam.create(pcaPyr[l].eigenvalues.rows, 1);
		for (int i = 0; i < pcaPyr[l].eigenvalues.rows; i++)
		{
			if (i < b_old.params.rows)
				curParam(i, 0) = b_old.params(i, 0);
			else
				curParam(i, 0) = 0;
		}
		cout << "curParam" << curParam << endl;
		int j = 0;
		
		do {
			//EM算法
			double s = curTrans.getS();//sqrt(a^2+b^2)
			lastParam = curParam.clone();

			//先验值
			curTrans.invtransform(Y, y_r);//做逆变换
			p = sigma2Pyr[l] / (sigma2Pyr[l] + rho2 / (s * s));
			//printf("p: %g, rho2/s2: %g, sigma2: %g\n", p, rho2/(s * s), sigma2Pyr[l]);

			delta2 = 1 / (1 / sigma2Pyr[l] + s*s / rho2);
			//printf("p: %g, rho2/s2: %g, sigma2: %g, delta2: %g\n",
			//	                p, rho2/(s * s), sigma2Pyr[l], delta2);

			//cout << j << ":  y_r" << y_r << "p :" << p << "delta2" << delta2 << endl;
			/*
				curparam为提取level1pca金字塔的特征值的nX1矩阵
				xFromParam为重构降维后的curparam矩阵
				y_r为形状向量Y的逆变换，做pca降维投影得到tmpFullParam
				再做重构得到y_rpr
			*/
			this->pcaPyr[l].backProject(curParam, xFromParam);
			//cout << "curparam" << curParam << endl;
			pcaFullShape->project(y_r, tmpFullParam);
			pcaFullShape->backProject(tmpFullParam, y_rpr);
			//cout << "y_rpr"<<y_rpr << "xformparam"<<xFromParam << endl;
			x = p*y_rpr + (1 - p) * xFromParam;//极大似然估计似然项
			//cout << "x" << x << endl;
			x2 = x.dot(x) + (x.rows - 4) * delta2;
			      
			//printf("p: %g, rho2/s2: %g, sigma2: %g, delta2: %g, x.x: %g, x2: %g\n",
		      //          p, rho2/(s * s), sigma2Pyr[l], delta2, x.dot(x), x2);
			//最大化值
			pcaPyr[l].project(x, curParam);
			for (int i = 0; i<pcaPyr[l].eigenvalues.rows; i++)
				curParam(i, 0) *= (pcaShape->eigenvalues.at<double>(i,0)) /
				(pcaShape->eigenvalues.at<double>(i, 0) + sigma2Pyr[l]);
			//当前形状矩阵进行系数调整，系数为特征值/（特征值+残差平方和）
			//根据先验值预判系数至收敛
			//cout << "curparam :" << curParam<<endl;
			int nP = x.rows / 2;
			curTrans.a = Y.dot(x)/x2;
			curTrans.b = 0;
			for (int i = 0; i<nP; i++)
				curTrans.b += x.X(i) * Y.Y(i) - x.Y(i)*Y.X(i);
			curTrans.b /= x2;
			curTrans.Xt = Y.getXMean();
			curTrans.Yt = Y.getYMean();
			j++;
			double normn = norm(lastParam - curParam);
			//cout << "norm :"<<normn <<"curTrans.Xt"<< curTrans.Xt<<"curTrans.Yt"<< curTrans.Yt << endl;
		} while (norm(lastParam - curParam)>1e-4 &&j>20);//预测使L2范数接近0防止过度拟合
		fitResult.params = curParam;
		fitResult.transformation = curTrans;
	}

	void Asmmodel::findParamForShape(const ShapeVec &Y, AsmFitResult & fitResult) {
		ShapeVec x, y;
		//初始化
		fitResult.params = Mat_<double>::zeros(nShapeParams, 1);
		simtrans &st = fitResult.transformation;

		Mat_<double> resOld;
		do
		{
			resOld = fitResult.params.clone();
			//投影到x
			projectParamToShape(fitResult.params, x);
			//x与y对齐
			st.setTransformByAlign(x, Y);
			//从Y到y的逆变换
			st.invtransform(Y, y);
			//和平均形状对齐
			y.alignTo(meanShape);
			//更新形状参数,将y投影到参数矩阵
			projectShapeToParam(y, fitResult.params);
			//通过阈值细化参数矩阵
			clampParamVec(fitResult.params);
			double normn = norm(resOld - fitResult.params);
			cout << normn << endl;
		} while (norm(resOld - fitResult.params)>1e-3);//L2绝对范数接近0,证明收敛，拟合停止
	}

	void Asmmodel::buildLocalDiffStructure() {

		int i, j, l;
		// First, we have find a proper "step" based on the size of face
		int xMin, xMax, yMin, yMax;
		vector< double > myStep;
		myStep.resize(nTrain);
		for (i = 0;i < nTrain;i++) {
			xMin = yMin = 100000000;
			xMax = yMax =0;
			for (j = 0;j < nMarkPoints;j++) {
				if (imageSet[i].points[j].x < xMin)
					xMin = imageSet[i].points[j].x;
				if (imageSet[i].points[j].y < yMin)
					yMin = imageSet[i].points[j].y;
				if (imageSet[i].points[j].x > xMax)
					xMax = imageSet[i].points[j].x;
				if (imageSet[i].points[j].y > yMax)
					yMax = imageSet[i].points[j].y;
			}
			myStep[i] = 1.3* sqrt((xMax - xMin)*(yMax - yMin) / 10000.);
		}
		Mat_< double > *tCovar, *tMean;//求逆协方差矩阵，均值矩阵
		Mat_< double > datMat(2 * localFeatureRad + 1, nTrain);//存储单个特征点-k到k的相邻点
		meanG.resize(this->pyramidLevel + 1);
		iCovarG.resize(this->pyramidLevel + 1);
		for (l = 0;l < pyramidLevel;l++) {
			for (i = 0;i < nMarkPoints;i++) {
				tCovar = new Mat_<double>;
				tMean = new Mat_<double>;
				for (j = 0;j < nTrain;j++) {
					Mat_<double> M;
					M = datMat.col(j);//取j列
					imageSet[j].getLocalStruct(i, localFeatureRad, 1, myStep[j]).copyTo(M);
				}
				calcCovarMatrix(datMat, *tCovar, *tMean,
					CV_COVAR_NORMAL | CV_COVAR_COLS);//计算均值和协方差矩阵
				*tCovar = tCovar->inv(DECOMP_SVD);//协方差矩阵求逆
				this->iCovarG[l].push_back(*tCovar);
				this->meanG[l].push_back(*tMean);
				delete tMean;
				delete tCovar;
			}
		}
	}

	void AsmFitResult::toPointList(vector< Point_<int> > &pV) const {
		ShapeVec sv;
		asmmodel->projectParamToShape(params, sv);
		sv.restoreToPointList(pV, transformation);
	}
}