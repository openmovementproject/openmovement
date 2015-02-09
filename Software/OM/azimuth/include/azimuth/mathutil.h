// Simple 4x4 Matrix functions
// Dan Jackson, 2011-2013

#ifndef _MATHUTL_H
#define _MATHUTIL_H

#ifdef __cplusplus
extern "C" {
#endif

void MatrixCopy(float *source, float *dest);
void MatrixLoadIdentity(float *dest);
void MatrixLoadXRotation(float *dest, float angleRadians);
void MatrixLoadYRotation(float *dest, float angleRadians);
void MatrixLoadZRotation(float *dest, float angleRadians);
void MatrixLoadZXYRotation(float phi, float theta, float psi, float *matrix);
void MatrixLoadZXYInverseRotation(float phi, float theta, float psi, float *matrix);
void MatrixMultiply(float *matrix1, float *matrix2, float *result);

void Point4MultiplyMatrix(float *point, float *matrix, float *result);
void Point3Copy(float *src, float *dest);
void Point3MultiplyMatrix(float *point, float *matrix, float *result);
void Point3MultiplyScalar(float *point, float value);
float Point3Length(float *p);
void Point3Normalize(float *p);
void Point3CrossProduct(float *v1, float *v2, float *cp);
float Point3DotProduct(float *v1, float *v2);

void QuaternionToMatrix(float *quaternion, float *matrix);
void QuaternionCopy(float *src, float *dest);
void QuaternionToEuler(float *q, float *angles);
void QuaternionToYawPitchRoll(float *q, float *ypr, float *gxyz);

#ifdef __cplusplus
}
#endif

#endif
