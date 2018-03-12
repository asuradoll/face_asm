#pragma once
#pragma execution_character_set("utf-8")

#include "shapeinfo.h"

namespace modelshare {

	//! Dump info to a model file.
	void ShapeInfo::writeToFile(ModelFile &f) const {

		f.writeInt(nContours);

		for (int i = 0;i<nContours + 1;i++)
			f.writeInt(contourStartInd[i]);

		for (int i = 0;i<nContours;i++)
			f.writeInt(contourIsClosed[i]);

		f.writeInt(pointInfo.size());
		for (size_t i = 0;i<pointInfo.size();i++) {
			f.writeInt(pointInfo[i].connectFrom);
			f.writeInt(pointInfo[i].connectTo);
			f.writeInt(pointInfo[i].type);
			f.writeInt(pointInfo[i].pathId);
		}
	}

	//! Load shape info from a model file.
	void ShapeInfo::readFromFile(ModelFile &f) {
		f.readInt(nContours);

		int t;
		contourStartInd.resize(nContours + 1);
		for (int i = 0;i<nContours + 1;i++)
			contourStartInd[i] = f.readInt(t);

		contourIsClosed.resize(nContours);
		for (int i = 0;i<nContours;i++)
			contourIsClosed[i] = f.readInt(t);

		pointInfo.resize(f.readInt(t));
		for (size_t i = 0;i < pointInfo.size();i++) {
			f.readInt(pointInfo[i].connectFrom);
			f.readInt(pointInfo[i].connectTo);
			f.readInt(pointInfo[i].type);
			f.readInt(pointInfo[i].pathId);
		}
	}

	//! Load from shape description file, return the number landmark points.
	int  ShapeInfo::loadFromShapeDescFile(reader &shapeDefFile) {
		int nMarkPoints; 
		FILE *fp = shapeDefFile.FL();
		shapeDefFile.Sync();//忽略掉whitespace，comments
		fscanf(fp, "%d", &nMarkPoints);
		shapeDefFile.Sync();
		fscanf(fp, "%d", &nContours);

		int i, j;
		contourStartInd.resize(nContours + 1);
		contourIsClosed.resize(nContours);
		contourStartInd[0] = 0;
		for (i = 0;i < nContours;i++) {
			shapeDefFile.Sync();
			fscanf(fp, "%d%d", &contourStartInd[i + 1], &contourIsClosed[i]);
		}

		pointInfo.resize(nMarkPoints);//根据读取的特征点数开辟空间

		for (i = 0;i < nContours;i++) {
			for (j = contourStartInd[i]; j < contourStartInd[i + 1]; j++) {
				pointInfo[j].pathId = i;
				pointInfo[j].type = contourIsClosed[i];
				pointInfo[j].connectFrom = j - 1;
				pointInfo[j].connectTo = j + 1;
			}//contoursStartInd必须按从小大到顺序排列,nMarkPoints必须覆盖contourstartind
			if (contourIsClosed[i] == 1) {
				pointInfo[contourStartInd[i]].connectFrom = contourStartInd[i + 1] - 1;
				pointInfo[contourStartInd[i + 1] - 1].connectTo = contourStartInd[i];
			}
			else {
				pointInfo[contourStartInd[i]].connectFrom =
					contourStartInd[i];
				pointInfo[contourStartInd[i + 1] - 1].connectTo =
					contourStartInd[i + 1] - 1;
			}
		}
		return nMarkPoints;
	}

	Mat ShapeInfo::drawMarkPointsOnImg(
		Mat &img, const vector< Point > &vP, bool drawInPlace ) const {

		Mat mb;
		if (drawInPlace)
			mb = img;
		else
			mb = img.clone();

		for (uint i = 0;i<vP.size();i++) {
			circle(mb, vP[i], 3, Scalar(25, 50, 255), 1, CV_AA);
		}

		for (int i = 0;i<nContours;i++) {
			for (int j = contourStartInd[i]; j<contourStartInd[i + 1]; j++) {
				line(mb, vP[pointInfo[j].connectFrom],
					vP[j],
					Scalar(200, 30, 80), 1, CV_AA);
			}
		}
		return mb;
	}
}

