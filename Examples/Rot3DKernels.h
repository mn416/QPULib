#ifndef _EXAMPLES_ROT3DKERNELS_H_
#define _EXAMPLES_ROT3DKERNELS_H_
#include <QPULib.h>

void rot3D(int n, float cosTheta, float sinTheta, float* x, float* y);

void rot3D_1(
	QPULib::Int n,
	QPULib::Float cosTheta,
	QPULib::Float sinTheta,
	QPULib::Ptr<QPULib::Float> x,
	QPULib::Ptr<QPULib::Float> y);

void rot3D_2(
	QPULib::Int n,
	QPULib::Float cosTheta,
	QPULib::Float sinTheta,
	QPULib::Ptr<QPULib::Float> x,
	QPULib::Ptr<QPULib::Float> y);

void rot3D_3(
	QPULib::Int n,
	QPULib::Float cosTheta,
	QPULib::Float sinTheta,
	QPULib::Ptr<QPULib::Float> x,
	QPULib::Ptr<QPULib::Float> y);


#endif  // _EXAMPLES_ROT3DKERNELS_H_
