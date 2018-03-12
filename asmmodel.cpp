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
			//���ü��������Ȥ���ֽ������
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

		AsmFitResult fitResult(this);//thisָ��ʵ����
		Mat grayImg;//����Ϊ�Ҷ�ͼ��
		if (img.channels() == 3)
			cvtColor(img, grayImg, CV_BGR2GRAY);
		else
			grayImg = img;

		//����ͼ���С
		Mat resizedImg;
		double ratio;
		ratio = sqrt(double(80000) / (grayImg.rows * grayImg.cols));
		resize(grayImg, resizedImg, Size(grayImg.cols*ratio, grayImg.rows*ratio));

		ModelImage curSearch;
		curSearch.setShapeInfo(&shapeInfo);
		curSearch.loadTrainImage(resizedImg);
		fitResult.params = Mat_<double>::zeros(nShapeParams, 1);//��ʼ����������

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

		//��ǰ����ƫ�������
		int totalOffset;
		if (verbose >= ASM_FIT_VERBOSE_AT_LEVEL)
			curSearch.show();
		vector< Point_< int > > V;
		for (int l = this->pyramidLevel; l >= 0; l--)//3��ͼ������� 2��1��0
		{
			if (verbose >= ASM_FIT_VERBOSE_AT_LEVEL)
				printf("Level %d\n", l);
			Mat_<double> img = curSearch.getDerivImage(l);//��ȡl��ͼ��
			//imshow("getDerivImage", img);
			//	cvWaitKey(0);
			int runT;
			double avgMov;
			for (runT = 0;runT < 10;runT++) 
			{
				//���ݵ�ǰ��״
				shape_old.fromPointList(curSearch.points);
				
				totalOffset = 0;
				vector< Point_< int > > bestEP(nMarkPoints);
				for (int i = 0;i < this->nMarkPoints;i++)
				//75����
				{
					if (verbose >= ASM_FIT_VERBOSE_AT_POINT)
						printf("Dealing point %d...\n", i);
					Mat_<double> nrmV(2 * k + 1, 1);
					double curBest = -1, ct;
					int bestI = 0;
					double absSum;
					for (int e = ns;e > -ns;e--) {
						curSearch.getPointsOnNorm(i, k, l, V, 2 * searchStepAreaRatio, e);
						//�õ�i�����㸽��-k��k����,�ش���v
						
						absSum = 0;
						for (int j = -k;j <= k;j++) {
							nrmV(j + k, 0) = img(V[j + k]);
							absSum += fabs(nrmV(j + k, 0));
						}
						nrmV *= 1/absSum;//��һ
						//�������Ͼ���
						ct=Mahalanobis(nrmV, this->meanG[l][i], this->iCovarG[l][i]);
						if (verbose >= ASM_FIT_VERBOSE_AT_POINT)
							curSearch.show(l, i, true, e);
						//���±�����̾����
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
				
				//����������
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

				//ͶӰ�����ɷַ���ģ���ٷ���
				//findParamForShape(curSearch.shapeVec,  fitResult);
				findParamForShapeBTSM(curSearch.shapeVec, shape_old, fitResult, fitResult, l);
				
				pcaPyr[l].backProject(fitResult.params, sv);
				//�������transfromation�Ĳ�����
				//�ع�����״
 
				curSearch.buildFromShapeVec(fitResult.transformation);
				//curSearch.show(l);
				avgMov = (double)totalOffset / nMarkPoints;//ƽ��ƫ����
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
		yt -= Y;//����״������ֵ
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
			//EM�㷨
			double s = curTrans.getS();//sqrt(a^2+b^2)
			lastParam = curParam.clone();

			//����ֵ
			curTrans.invtransform(Y, y_r);//����任
			p = sigma2Pyr[l] / (sigma2Pyr[l] + rho2 / (s * s));
			//printf("p: %g, rho2/s2: %g, sigma2: %g\n", p, rho2/(s * s), sigma2Pyr[l]);

			delta2 = 1 / (1 / sigma2Pyr[l] + s*s / rho2);
			//printf("p: %g, rho2/s2: %g, sigma2: %g, delta2: %g\n",
			//	                p, rho2/(s * s), sigma2Pyr[l], delta2);

			//cout << j << ":  y_r" << y_r << "p :" << p << "delta2" << delta2 << endl;
			/*
				curparamΪ��ȡlevel1pca������������ֵ��nX1����
				xFromParamΪ�ع���ά���curparam����
				y_rΪ��״����Y����任����pca��άͶӰ�õ�tmpFullParam
				�����ع��õ�y_rpr
			*/
			this->pcaPyr[l].backProject(curParam, xFromParam);
			//cout << "curparam" << curParam << endl;
			pcaFullShape->project(y_r, tmpFullParam);
			pcaFullShape->backProject(tmpFullParam, y_rpr);
			//cout << "y_rpr"<<y_rpr << "xformparam"<<xFromParam << endl;
			x = p*y_rpr + (1 - p) * xFromParam;//������Ȼ������Ȼ��
			//cout << "x" << x << endl;
			x2 = x.dot(x) + (x.rows - 4) * delta2;
			      
			//printf("p: %g, rho2/s2: %g, sigma2: %g, delta2: %g, x.x: %g, x2: %g\n",
		      //          p, rho2/(s * s), sigma2Pyr[l], delta2, x.dot(x), x2);
			//���ֵ
			pcaPyr[l].project(x, curParam);
			for (int i = 0; i<pcaPyr[l].eigenvalues.rows; i++)
				curParam(i, 0) *= (pcaShape->eigenvalues.at<double>(i,0)) /
				(pcaShape->eigenvalues.at<double>(i, 0) + sigma2Pyr[l]);
			//��ǰ��״�������ϵ��������ϵ��Ϊ����ֵ/������ֵ+�в�ƽ���ͣ�
			//��������ֵԤ��ϵ��������
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
		} while (norm(lastParam - curParam)>1e-4 &&j>20);//Ԥ��ʹL2�����ӽ�0��ֹ�������
		fitResult.params = curParam;
		fitResult.transformation = curTrans;
	}

	void Asmmodel::findParamForShape(const ShapeVec &Y, AsmFitResult & fitResult) {
		ShapeVec x, y;
		//��ʼ��
		fitResult.params = Mat_<double>::zeros(nShapeParams, 1);
		simtrans &st = fitResult.transformation;

		Mat_<double> resOld;
		do
		{
			resOld = fitResult.params.clone();
			//ͶӰ��x
			projectParamToShape(fitResult.params, x);
			//x��y����
			st.setTransformByAlign(x, Y);
			//��Y��y����任
			st.invtransform(Y, y);
			//��ƽ����״����
			y.alignTo(meanShape);
			//������״����,��yͶӰ����������
			projectShapeToParam(y, fitResult.params);
			//ͨ����ֵϸ����������
			clampParamVec(fitResult.params);
			double normn = norm(resOld - fitResult.params);
			cout << normn << endl;
		} while (norm(resOld - fitResult.params)>1e-3);//L2���Է����ӽ�0,֤�����������ֹͣ
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
		Mat_< double > *tCovar, *tMean;//����Э������󣬾�ֵ����
		Mat_< double > datMat(2 * localFeatureRad + 1, nTrain);//�洢����������-k��k�����ڵ�
		meanG.resize(this->pyramidLevel + 1);
		iCovarG.resize(this->pyramidLevel + 1);
		for (l = 0;l < pyramidLevel;l++) {
			for (i = 0;i < nMarkPoints;i++) {
				tCovar = new Mat_<double>;
				tMean = new Mat_<double>;
				for (j = 0;j < nTrain;j++) {
					Mat_<double> M;
					M = datMat.col(j);//ȡj��
					imageSet[j].getLocalStruct(i, localFeatureRad, 1, myStep[j]).copyTo(M);
				}
				calcCovarMatrix(datMat, *tCovar, *tMean,
					CV_COVAR_NORMAL | CV_COVAR_COLS);//�����ֵ��Э�������
				*tCovar = tCovar->inv(DECOMP_SVD);//Э�����������
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