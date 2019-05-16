#pragma once

#include "Geometry.h"
#include "Ray.h"
#include "BoundingBox.h"

#include <Eigen/Core>
#include <Eigen/LU>
#include <Eigen/Geometry>

#include <memory>

///////////////////////////////////////////////////////////////////////////////
/// \brief This class aims at providing functions to perform affine transforms
///////////////////////////////////////////////////////////////////////////////
class Transform
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

public:
	typedef std::shared_ptr<Transform> ptr;
	typedef Eigen::Matrix<real, 4, 4> Matrix4r;

public:
	Transform(void);
	Transform(real values[16]);
	Transform(real rotation[3], const Point3d& translation, const Point3d& scale = Point3d(1., 1., 1.));
	Transform(const Matrix4r& transform);
	Transform(const Matrix4r& transform, const Matrix4r& invTransform);
	~Transform(void);

	inline Point3d transform(const Point3d &point);
	inline void transform(const Point3d& inPoint, Point3d& outPoint);

	inline Vector3d transform(const Vector3d& vect);
	inline void transform(const Vector3d& in, Vector3d& out);

	inline Ray transform(const Ray& ray);
	inline void transform(const Ray& inPoint, Ray& outPoint);

	inline BoundingBox transform(const BoundingBox& box);
	inline void transform(const BoundingBox& inBox, BoundingBox& outBox);

	inline Normal3d transform(const Normal3d& n);
	inline void transform(const Normal3d& inN, Normal3d& outN);

	inline Transform inv() { return Transform(myInvMatrix, myMatrix); }

	inline Transform operator * (const Transform& t);
	inline Transform& operator *= (const Transform& t);

	static Transform identity() { return Transform(Matrix4r::Identity(), Matrix4r::Identity()); }

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Create a translation matrix
	///////////////////////////////////////////////////////////////////////////////
	static Transform translate(const Vector3d& t);

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Create a scale matrix
	///////////////////////////////////////////////////////////////////////////////
	static Transform scale(const Point3d& s);
	static Transform rotateX(real angle);
	static Transform rotateY(real angle);
	static Transform rotateZ(real angle);

	static Transform fromLookAt(const Point3d& origin, const Point3d& lookAt, const Vector3d& up);

	friend std::ostream& operator << (std::ostream& o, const Transform& t);

protected:
	Matrix4r myMatrix;
	Matrix4r myInvMatrix;
	//Eigen::Matrix4d myMatrix;
	//Eigen::Matrix4d myInvMatrix;
};

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
inline Point3d Transform::transform(const Point3d & point)
{
	Point3d res;
	transform(point, res);
	return res;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
inline void Transform::transform(const Point3d & inPoint, Point3d & outPoint)
{
	outPoint.x() = myMatrix(0, 0) * inPoint.x() + myMatrix(0, 1) * inPoint.y() + myMatrix(0, 2) * inPoint.z() + myMatrix(0, 3);
	outPoint.y() = myMatrix(1, 0) * inPoint.x() + myMatrix(1, 1) * inPoint.y() + myMatrix(1, 2) * inPoint.z() + myMatrix(1, 3);
	outPoint.z() = myMatrix(2, 0) * inPoint.x() + myMatrix(2, 1) * inPoint.y() + myMatrix(2, 2) * inPoint.z() + myMatrix(2, 3);
	real w	 = myMatrix(3, 0) * inPoint.x() + myMatrix(3, 1) * inPoint.y() + myMatrix(3, 2) * inPoint.z() + myMatrix(3, 3);

	if (w != 1. && w != 0.)
		outPoint /= w;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
inline Vector3d Transform::transform(const Vector3d & vect)
{
	Vector3d res;
	transform(vect, res);
	return res;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
inline void Transform::transform(const Vector3d & in, Vector3d & out)
{
#ifdef USE_ALIGN
	out = myMatrix * in;
#else
	out = myMatrix.topLeftCorner<3, 3>() * in;
#endif
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
inline Ray Transform::transform(const Ray & ray)
{
	Ray res = ray;
	transform(ray, res);
	return res;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
inline void Transform::transform(const Ray & in, Ray & out)
{
	Vector3d tmpDir;
	transform(in.myOrigin, out.myOrigin);
	transform(in.direction(), tmpDir);
	out.direction(tmpDir);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
inline BoundingBox Transform::transform(const BoundingBox & box)
{
	BoundingBox res;

	transform(box, res);
	return res;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
inline void Transform::transform(const BoundingBox& inBox, BoundingBox& outBox)
{
	//outBox = inBox;
	outBox = BoundingBox::unionBox(outBox, transform(Point3d(inBox.myMin.x(), inBox.myMin.y(), inBox.myMin.z())));
	outBox = BoundingBox::unionBox(outBox, transform(Point3d(inBox.myMin.x(), inBox.myMin.y(), inBox.myMax.z())));
	outBox = BoundingBox::unionBox(outBox, transform(Point3d(inBox.myMin.x(), inBox.myMax.y(), inBox.myMax.z())));
	outBox = BoundingBox::unionBox(outBox, transform(Point3d(inBox.myMax.x(), inBox.myMax.y(), inBox.myMax.z())));
	outBox = BoundingBox::unionBox(outBox, transform(Point3d(inBox.myMax.x(), inBox.myMin.y(), inBox.myMin.z())));
	outBox = BoundingBox::unionBox(outBox, transform(Point3d(inBox.myMax.x(), inBox.myMax.y(), inBox.myMin.z())));
	outBox = BoundingBox::unionBox(outBox, transform(Point3d(inBox.myMax.x(), inBox.myMin.y(), inBox.myMax.z())));
	outBox = BoundingBox::unionBox(outBox, transform(Point3d(inBox.myMin.x(), inBox.myMax.y(), inBox.myMin.z())));
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
inline Normal3d Transform::transform(const Normal3d & n)
{
	Normal3d res;
	transform(n, res);
	return res;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
inline void Transform::transform(const Normal3d& inN, Normal3d& outN)
{
#ifdef USE_ALIGN
	auto tmp = myInvMatrix.topLeftCorner<3, 3>().transpose() * inN.head<3>();
	outN = Normal3d(tmp.x(), tmp.y(), tmp.z());
#else
	outN = myInvMatrix.topLeftCorner<3, 3>().transpose() * inN;
#endif
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
inline Transform Transform::operator * (const Transform& t)
{
	return Transform(myMatrix * t.myMatrix, t.myInvMatrix * myInvMatrix);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
inline Transform & Transform::operator*=(const Transform & t)
{
	myMatrix *= t.myMatrix;
	myInvMatrix = t.myInvMatrix * myMatrix;
	return *this;
}