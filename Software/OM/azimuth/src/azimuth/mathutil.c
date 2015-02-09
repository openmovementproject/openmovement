// Simple Vector3 and 4x4 Matrix functions
// Dan Jackson, 2011-2013

#ifdef _WIN32
#define _USE_MATH_DEFINES       // For M_PI
#endif

#include <math.h>

#include "azimuth/mathutil.h"


void MatrixCopy(float *source, float *dest)
{
	int i;
	for (i = 0; i < 16; ++i)
	{
		dest[i] = source[i];
	}
}

void MatrixLoadIdentity(float *dest)
{
	dest[0] = 1.0f; dest[4] = 0.0f; dest[8] = 0.0f; dest[12] = 0.0f; 
	dest[1] = 0.0f; dest[5] = 1.0f; dest[9] = 0.0f; dest[13] = 0.0f; 
	dest[2] = 0.0f; dest[6] = 0.0f; dest[10] = 1.0f; dest[14] = 0.0f; 
	dest[3] = 0.0f; dest[7] = 0.0f; dest[11] = 0.0f; dest[15] = 1.0f; 
}

void MatrixLoadXRotation(float *dest, float angleRadians)
{
	dest[0] = 1.0f; dest[4] = 0.0f; dest[8] = 0.0f; dest[12] = 0.0f; 
	dest[1] = 0.0f; dest[5] = (float)cos(angleRadians); dest[9] = -(float)sin(angleRadians); dest[13] = 0.0f; 
	dest[2] = 0.0f; dest[6] = (float)sin(angleRadians); dest[10] = (float)cos(angleRadians); dest[14] = 0.0f; 
	dest[3] = 0.0f; dest[7] = 0.0f; dest[11] = 0.0f; dest[15] = 1.0f; 
}

void MatrixLoadYRotation(float *dest, float angleRadians)
{
	dest[0] = (float)cos(angleRadians); dest[4] = 0.0f; dest[8] = (float)sin(angleRadians); dest[12] = 0.0f; 
	dest[1] = 0.0f; dest[5] = 1.0f; dest[9] = 0.0f; dest[13] = 0.0f; 
	dest[2] = -(float)sin(angleRadians); dest[6] = 0.0f; dest[10] = (float)cos(angleRadians); dest[14] = 0.0f; 
	dest[3] = 0.0f; dest[7] = 0.0f; dest[11] = 0.0f; dest[15] = 1.0f; 
}

void MatrixLoadZRotation(float *dest, float angleRadians)
{
	dest[0] = (float)cos(angleRadians); dest[4] = -(float)sin(angleRadians); dest[8] = 0.0f; dest[12] = 0.0f; 
	dest[1] = (float)sin(angleRadians); dest[5] = (float)cos(angleRadians); dest[9] = 0.0f; dest[13] = 0.0f; 
	dest[2] = 0.0f; dest[6] = 0.0f; dest[10] = 1.0f; dest[14] = 0.0f; 
	dest[3] = 0.0f; dest[7] = 0.0f; dest[11] = 0.0f; dest[15] = 1.0f; 
}

void MatrixLoadZXYRotation(float phi, float theta, float psi, float *matrix)
{
	// TODO: Speed this up by composing the matrix manually 
	float rotX[16], rotY[16], rotZ[16], temp[16];
	MatrixLoadZRotation(rotZ, phi);    // Z: phi (roll)
	MatrixLoadXRotation(rotX, theta);  // X: theta (pitch)
	MatrixLoadYRotation(rotY, psi);    // Y: psi (yaw)
	MatrixMultiply(rotZ, rotX, temp);
	MatrixMultiply(temp, rotY, matrix);
}

void MatrixLoadZXYInverseRotation(float phi, float theta, float psi, float *matrix)
{
	// TODO: Speed this up by composing the matrix manually 
	float rotX[16], rotY[16], rotZ[16], temp[16];
	MatrixLoadZRotation(rotZ, phi);    // Z: phi (roll)
	MatrixLoadXRotation(rotX, theta);  // X: theta (pitch)
	MatrixLoadYRotation(rotY, psi);    // Y: psi (yaw)
	MatrixMultiply(rotY, rotX, temp);
	MatrixMultiply(temp, rotZ, matrix);
}


void MatrixMultiply(float *matrix1, float *matrix2, float *result)
{
	static float a1, b1, c1, d1, e1, f1, g1, h1, i1, j1, k1, l1;
    static float a2, b2, c2, d2, e2, f2, g2, h2, i2, j2, k2, l2;

	// gl-arrangement
    a1 = matrix1[0];  b1 = matrix1[4];  c1 = matrix1[8];  d1 = matrix1[12];
    e1 = matrix1[1];  f1 = matrix1[5];  g1 = matrix1[9];  h1 = matrix1[13];
    i1 = matrix1[2];  j1 = matrix1[6];  k1 = matrix1[10]; l1 = matrix1[14];

    a2 = matrix2[0];  b2 = matrix2[4];  c2 = matrix2[8];  d2 = matrix2[12];
    e2 = matrix2[1];  f2 = matrix2[5];  g2 = matrix2[9];  h2 = matrix2[13];
    i2 = matrix2[2];  j2 = matrix2[6];  k2 = matrix2[10]; l2 = matrix2[14];

    result[0] = a1*a2 + b1*e2 + c1*i2;
    result[1] = e1*a2 + f1*e2 + g1*i2;
    result[2] = i1*a2 + j1*e2 + k1*i2;
    result[3] = 0.0f;

    result[4] = a1*b2 + b1*f2 + c1*j2;
    result[5] = e1*b2 + f1*f2 + g1*j2;
    result[6] = i1*b2 + j1*f2 + k1*j2;
    result[7] = 0.0f;

    result[8] = a1*c2 + b1*g2 + c1*k2;
    result[9] = e1*c2 + f1*g2 + g1*k2;
    result[10] = i1*c2 + j1*g2 + k1*k2;
    result[11] = 0.0f;

    result[12] = a1*d2 + b1*h2 + c1*l2 + d1;
    result[13] = e1*d2 + f1*h2 + g1*l2 + h1;
    result[14] = i1*d2 + j1*h2 + k1*l2 + l1;
    result[15] = 1.0f;

    return;
}


void Point4MultiplyMatrix(float *point, float *matrix, float *result)
{
    result[0] = matrix[0] * point[0] + matrix[4] * point[1] + matrix[8] * point[2] + matrix[12] * point[3];
    result[1] = matrix[1] * point[0] + matrix[5] * point[1] + matrix[9] * point[2] + matrix[13] * point[3];
    result[2] = matrix[2] * point[0] + matrix[6] * point[1] + matrix[10] * point[2] + matrix[14] * point[3];
    result[3] = matrix[3] * point[0] + matrix[7] * point[1] + matrix[11] * point[2] + matrix[15] * point[3];
    return;
}


// 
void Point3Copy(float *src, float *dest)
{
    int i;
    for (i = 0; i < 3; ++i)
    {
        dest[i] = (src == (void *)0) ? 0.0f : src[i];
    }
}


void Point3MultiplyMatrix(float *point, float *matrix, float *result)
{
    result[0] = matrix[0] * point[0] + matrix[4] * point[1] + matrix[8] * point[2] + matrix[12];
    result[1] = matrix[1] * point[0] + matrix[5] * point[1] + matrix[9] * point[2] + matrix[13];
    result[2] = matrix[2] * point[0] + matrix[6] * point[1] + matrix[10] * point[2] + matrix[14];
    return;
}


void Point3MultiplyScalar(float *point, float value)
{
    point[0] *= value;
    point[1] *= value;
    point[2] *= value;
	return;
}


// Vector length
float Point3Length(float *p) 
{
    return (float)sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]);
}


// Normalize a vector by dividing by its length
void Point3Normalize(float *p) 
{
    float s = Point3Length(p);
    if (s != 0.0f) { p[0] /= s; p[1] /= s; p[2] /= s; }
    return;
}


// Calculate the cross-product of two vectors
void Point3CrossProduct(float *v1, float *v2, float *cp) 
{
    cp[0] = v1[1]*v2[2] - v1[2]*v2[1];
    cp[1] = v1[2]*v2[0] - v1[0]*v2[2];
    cp[2] = v1[0]*v2[1] - v1[1]*v2[0];
    return;
}


// Calculate the dot-product of two vectors
float Point3DotProduct(float *v1, float *v2) 
{
    // cos(theta) = dotProduct(v1, v2) / (length(v1) * length(v2))
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}


void QuaternionToMatrix(float *quaternion, float *matrix)
{
	float xSquared = quaternion[0] * quaternion[0];
	float ySquared = quaternion[1] * quaternion[1];
	float zSquared = quaternion[2] * quaternion[2];
	float wx2 = quaternion[3] * quaternion[0] * 2.0f;
	float wy2 = quaternion[3] * quaternion[1] * 2.0f;
	float wz2 = quaternion[3] * quaternion[2] * 2.0f;
	float xy2 = quaternion[0] * quaternion[1] * 2.0f;
	float xz2 = quaternion[0] * quaternion[2] * 2.0f;
	float yz2 = quaternion[1] * quaternion[2] * 2.0f;

	matrix[0] = 1.0f - 2.0f * (ySquared + zSquared);
	matrix[1] = xy2 + wz2;
	matrix[2] = xz2 - wy2;
	matrix[3] = 0.0f;

	matrix[4] = xy2 - wz2;
	matrix[5] = 1.0f - 2.0f * (xSquared + zSquared);
	matrix[6] = yz2 + wx2;
	matrix[7] = 0.0f;

	matrix[8] = xz2 + wy2;
	matrix[9] = yz2 - wx2;
	matrix[10] = 1.0f - 2.0f * (xSquared + ySquared);
	matrix[11] = 0.0f;

	matrix[12] = 0.0f;
	matrix[13] = 0.0f;
	matrix[14] = 0.0f;
	matrix[15] = 1.0f;

	return;
}


// Get the quaternion from the AHRS tracker
void QuaternionCopy(float *src, float *dest)
{
    int i;
    for (i = 0; i < 4; ++i)
    {
        dest[i] = src[i];
    }
}


// Gets the Euler angles in radians defined with the Aerospace sequence (psi, theta, phi).
// See Sebastian O.H. Madwick report 
// "An efficient orientation filter for inertial and inertial/magnetic sensor arrays" Chapter 2 Quaternion representation
void QuaternionToEuler(float *q, float *angles)
{
	angles[0] = (float)atan2(2 * q[1] * q[2] - 2 * q[0] * q[3], 2 * q[0]*q[0] + 2 * q[1] * q[1] - 1); // psi
	angles[1] = -(float)asin(2 * q[1] * q[3] + 2 * q[0] * q[2]); // theta
	angles[2] = (float)atan2(2 * q[2] * q[3] - 2 * q[0] * q[1], 2 * q[0] * q[0] + 2 * q[3] * q[3] - 1); // phi
}


// Gets the yaw, pitch, roll angles in radians
// See Sebastian O.H. Madwick report 
// "An efficient orientation filter for inertial and inertial/magnetic sensor arrays" Chapter 2 Quaternion representation
void QuaternionToYawPitchRoll(float *q, float *ypr, float *gxyz)
{
	float gx, gy, gz; // estimated gravity direction
	float sgyz, sgxz;

	gx = 2 * (q[1]*q[3] - q[0]*q[2]);
	gy = 2 * (q[0]*q[1] + q[2]*q[3]);
	gz = q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3];

	sgyz = (float)sqrt(gy * gy + gz * gz);
	sgxz = (float)sqrt(gx * gx + gz * gz);

    if (ypr)
    {
	    ypr[0] = (float)atan2(2 * q[1] * q[2] - 2 * q[0] * q[3], 2 * q[0]*q[0] + 2 * q[1] * q[1] - 1);
	    ypr[1] = (sgyz < 0.05f) ? 0.0f : (float)atan(gx / sgyz);
	    ypr[2] = (sgxz < 0.05f) ? 0.0f : (float)atan(gy / sgxz);
    }

    if (gxyz)
    {
	    gxyz[0] = gx;
	    gxyz[1] = gy;
	    gxyz[2] = gz;
    }
}


/*

// Calculate the distance between the point P and the segment
float distanceToSegment(float px, float py, float x1, float y1, float x2, float y2, float *nx, float *ny)
{
  float dx, dy, t;
  dx = x2 - x1;
  dy = y2 - y1;
  if (dx == 0 && dy == 0) 
  {
    // point only, not a line 
    dx = px - x1; dy = py - y1;
    *nx = x1; *ny = y1;
    return (float)sqrt(dx * dx + dy * dy); 
  }
  // calc minimum t with distance
  t = ((px - x1) * dx + (py - y1) * dy) / (dx * dx + dy * dy);

  // Check if this is a segment end point or mid-segment
  if (t < 0)
  {
    dx = px - x1; dy = py - y1;
    if (nx != NULL) { *nx = x1; }
    if (ny != NULL) { *ny = y1; }
  }
  else if (t > 1)
  {
    dx = px - x2; dy = py - y2;
    if (nx != NULL) { *nx = x2; }
    if (ny != NULL) { *ny = y2; }
  }
  else 
  {
    dx = px - (x1 + t * dx); dy = py - (y1 + t * dy);
    if (nx != NULL) { *nx = x1 + t * dx; }
    if (ny != NULL) { *ny = y1 + t * dy; }
  }
  return (float)sqrt(dx * dx + dy * dy);
}


// Closest point (B) to point A along the line with a segment from P1 to P2
void nearestPoint(float p1[3], float p2[3], float a[3], float b[3])
{
    float t[3];
    float u[3];
    float dp;

    // T = A - P1;
    t[0] = a[0] - p1[0]; t[1] = a[1] - p1[1]; t[2] = a[2] - p1[2];  

    // U = Normalize(P2 - P1);
    u[0] = p2[0] - p1[0]; u[1] = p2[1] - p1[1]; u[2] = p2[2] - p1[2];  
    normalize(u);

    // B = P1 + DotProduct(T, U) * U;
    dp = dotProduct(t, u);
    b[0] = dp * u[0]; b[1] = dp * u[1]; b[2] = dp * u[2];
    b[0] += p1[0]; b[1] += p1[1]; b[2] += p1[2]; 

    return;
}

// Calculate the intersection of a line segment (p1 to p2) and a sphere (origin sc, radius r) - returns 0 if no intersection, non-zero if intersection.
// The two potential points of intersection are at:
// p = p1 + mu1(p2 - p1)
// p = p1 + mu2(p2 - p1)
int lineSphereIntersect(float p1[3], float p2[3], float sc[3], float r, float *mu1, float *mu2)
{
    float a, b, c;
    float bb4ac;
    float dp[3];

    dp[0] = p2[0] - p1[0];
    dp[1] = p2[1] - p1[1];
    dp[2] = p2[2] - p1[2];

    a = dp[0] * dp[0] + dp[1] * dp[1] + dp[2] * dp[2];

    b = 2 * (dp[0] * (p1[0] - sc[0]) + dp[1] * (p1[1] - sc[1]) + dp[2] * (p1[2] - sc[2]));

    c = sc[0] * sc[0] + sc[1] * sc[1] + sc[2] * sc[2];
    c += p1[0] * p1[0] + p1[1] * p1[1] + p1[2] * p1[2];
    c -= 2 * (sc[0] * p1[0] + sc[1] * p1[1] + sc[2] * p1[2]);
    c -= r * r;

    bb4ac = b * b - 4 * a * c;

    if ((float)fabs(a) < 0.001f || bb4ac < 0.0f)
    {
        *mu1 = 0.0f;
        *mu2 = 0.0f;
        return 0;
    }

    *mu1 = (-b + (float)sqrt(bb4ac)) / (2 * a);
    *mu2 = (-b - (float)sqrt(bb4ac)) / (2 * a);
    return 1;
}


// Returns 1 if the segments intersect, 0 otherwise
int lineIntersect(float ax1, float ay1, float ax2, float ay2, float bx1, float by1, float bx2, float by2, float *ix, float *iy, float *it)
{
    float dax, day, dbx, dby, ta, tb;
    dax = ax2 - ax1; day = ay2 - ay1;
    dbx = bx2 - bx1; dby = by2 - by1;
    // If segments are not parallel: find the intersection point along the infinite lines, and check it occurred on the actual line segment itself
    if (dbx * day - dby * dax != 0) {
        tb = (dax * (by1 - ay1) + day * (ax1 - bx1)) / (dbx * day - dby * dax);
        ta = (dbx * (ay1 - by1) + dby * (bx1 - ax1)) / (dby * dax - dbx * day);
        if (tb >= 0.0f && tb <= 1.0f && ta >= 0.0f && ta <= 1.0f) {
            // return intersection coordinates and proportion along line
            if (ix != NULL) { *ix = ax1 + ta * dax; }
            if (iy != NULL) { *iy = ay1 + ta * day; }
            if (it != NULL) { *it = ta; }
            return 1;
        }
    }
    // does not intersect
    return 0;
}

int lineCircleIntersect(float lx1, float ly1, float lx2, float ly2, float cx, float cy, float cr, float *ix, float *iy, float *it)
{
    float dx = lx2 - lx1;
    float dy = ly2 - ly1;
    float a = (dx * dx + dy * dy);
    float b = 2.0f * (lx1 * dx + ly1 * dy - dx * cx - dy * cy);
    float c = lx1 * lx1 - 2 * lx1 * cx + cx * cx + ly1 * ly1 - 2 * ly1 * cy + cy * cy - cr * cr;
    float d = b * b - 4 * a * c;
    float t1, t2, t;
    if (d >= 0.0f && a != 0.0f)
    {
        t1 = (-b - (float)sqrt(d)) / (2.0f * a);
        t2 = (-b + (float)sqrt(d)) / (2.0f * a);

        t = -1.0f;
        if ((t1 >= 0.0f && t1 <= 1.0f) && (t2 >= 0.0f && t2 <= 1.0f))
        {
            t = t1 < t2 ? t1 : t2;
        } 
        else if (t1 >= 0.0f && t1 <= 1.0f)
        {
            t = t1;
        }
        else if (t2 >= 0.0f && t2 <= 1.0f)
        {
            t = t2;
        }

        if (t >= 0.0f && t <= 1.0f)
        {
            if (ix != NULL) { *ix = lx1 + t * dx; }
            if (iy != NULL) { *iy = ly1 + t * dy; }
            if (it != NULL) { *it = t; }
            return 1;
        }

    }
    return 0;
}

*/
