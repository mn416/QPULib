#ifndef _EXAMPLES_ROT3DKERNELS_H_
#define _EXAMPLES_ROT3DKERNELS_H_
#include <QPULib.h>

namespace Rot3D {
using PtrFloat = QPULib::Ptr<QPULib::Float>;

void rot3D(int n, float cosTheta, float sinTheta, float* x, float* y);
void rot3D_1(QPULib::Int n, QPULib::Float cosTheta, QPULib::Float sinTheta, PtrFloat x, PtrFloat y);
void rot3D_2(QPULib::Int n, QPULib::Float cosTheta, QPULib::Float sinTheta, PtrFloat x, PtrFloat y);
void rot3D_3(QPULib::Int n, QPULib::Float cosTheta, QPULib::Float sinTheta, PtrFloat x, PtrFloat y);

}  // namespace Rot3D


#endif  // _EXAMPLES_ROT3DKERNELS_H_
