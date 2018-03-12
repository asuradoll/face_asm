#pragma once
#pragma execution_character_set("utf-8")
#ifndef SIMTRANS_H
#define SIMTRANS_H
#include "base function.h"

namespace modelshare {
	class ShapeVec;
	class simtrans {
	public :
		//ȡxt����ϵ��Ϊ1��y����ϵ��Ϊ0��xt��yt��ʼֵΪ0
		simtrans() :Xt(0), Yt(0), a(1), b(0) {}
		
		//��������
		simtrans operator *(const simtrans &s) const{
			simtrans ns;
			ns.a = a*s.a - b*s.b;
			ns.b = b*s.a + a*s.b;
			ns.Xt = a*s.Xt - b*s.Yt + Xt;
			ns.Yt = b*s.Xt + a*s.Yt + Yt;
			return ns;
		}

		//! Transform a shape 
		/*! \note src and dst can be the same vector
		*/
		void transform(const ShapeVec &src, ShapeVec &dst) const;
	
		//! Inverted transformation
		/*! \note src and dst can be the same vector
		*/
		void invtransform(const ShapeVec &src, ShapeVec &dst) const;

		//! Warp an image by this similarity transform.
		void warpImage(const Mat &imgSrc, Mat &imgDst) const;
		
		//! Warp an image by the inverted similarity transform. ????
		void warpImgBack(const Mat &imgSrc, Mat &imgDst, bool useDstSize = false) const;

		//! find the transformation that best align x to xp mainfunction
		void setTransformByAlign(const ShapeVec &x, const ShapeVec &xp);

		//б�߳���
		double getS() const { return sqrt(a*a + b*b); }
		//xt yt a bΪ���任�ľ���ϵ��,���asm����
		//! X translate
		double Xt;
		//! Y translate
		double Yt;

		//! a in similarity transformation matrix
		double a;
		//! b in similarity transformation matrix
		double b;
	};
}



#endif // !SIMTRANS_H