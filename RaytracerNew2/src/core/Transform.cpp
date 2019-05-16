#include "Transform.h"

#include "Math.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Transform::Transform(void)
:Transform(Matrix4r::Identity(), Matrix4r::Identity())
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Transform::Transform(real values[16])
{
	//nb : matrix constructor stacks in a column-wise order, don't use it
	myMatrix << values[0], values[1], values[2], values[3],
				values[4], values[5], values[6], values[7],
				values[8], values[9], values[10], values[11],
				values[12], values[13], values[14], values[15];

	myInvMatrix = myMatrix.inverse();

	//for (auto i = 0; i < 16; i++)
	//{
	//	myMatrix << values[i];
	//}
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Transform::Transform(real rotation[3], const Point3d & translation, const Point3d & scale)
{
	Transform translate = Transform::translate(Point3d(translation[0], translation[1], translation[2]));
	Transform rotateX = Transform::rotateX(rotation[0]);
	Transform rotateY = Transform::rotateY(rotation[1]);
	Transform rotateZ = Transform::rotateZ(rotation[2]);
	
	if (scale.x() != 1. && scale.y() != 1. && scale.z() != 1.)
	{
		Transform s = Transform::scale(scale);
		myMatrix = translate.myMatrix * rotateX.myMatrix * rotateY.myMatrix * rotateZ.myMatrix * s.myMatrix ;
	}	
	else
	{
		myMatrix = translate.myMatrix * rotateX.myMatrix * rotateY.myMatrix * rotateZ.myMatrix;
	}

	myInvMatrix = myMatrix.inverse();
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Transform::Transform(const Eigen::Matrix<real, 4, 4>& transform)
:myMatrix(transform)
,myInvMatrix(transform.inverse())
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Transform::Transform(const Eigen::Matrix<real, 4, 4>& transform, const Eigen::Matrix<real, 4, 4>& invTransform)
:myMatrix(transform)
,myInvMatrix(invTransform)
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Transform::~Transform(void)
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Transform Transform::translate(const Vector3d& t)
{
	Matrix4r res;
	res << 1, 0, 0, t.x(),
		   0, 1, 0, t.y(),
		   0, 0, 1, t.z(),
		   0, 0, 0, 1;

	Matrix4r invRes;
	invRes << 1, 0, 0, -t.x(),
			  0, 1, 0, -t.y(),
			  0, 0, 1, -t.z(),
			  0, 0, 0, 1;
	
	return Transform(res, invRes);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Transform Transform::scale(const Point3d& s)
{

	Matrix4r res;
	res << s.x(), 0,     0,     0,
		   0,     s.y(), 0,     0,
		   0,     0,     s.z(), 0,
		   0,     0,     0,     1;

	Matrix4r invRes;
	invRes << 1.f/s.x(), 0,         0,         0,
			  0,         1.f/s.y(), 0,         0,
			  0,         0,         1.f/s.z(), 0,
			  0,         0,         0,         1;


	return Transform(res, invRes);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Transform Transform::rotateX(real angle)
{
	real radAngle = math::toRadian(angle);

	real sinVal = std::sin(radAngle);
	real cosVal = std::cos(radAngle);

	Matrix4r mat;
	mat << 1, 0,      0,       0,
		   0, cosVal, -sinVal, 0,
		   0, sinVal, cosVal,  0,
		   0, 0,      0,       1;

	return Transform(mat, mat.transpose());
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Transform Transform::rotateY(real angle) {
	real radAngle = math::toRadian(angle);

	real sinVal = std::sin(radAngle);
	real cosVal = std::cos(radAngle);
	
	Matrix4r mat;
	mat << cosVal,  0, sinVal, 0,
		   0,       1, 0,      0,
		   -sinVal, 0, cosVal, 0,
		   0,       0, 0,      1;

	return Transform(mat, mat.transpose());
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Transform Transform::rotateZ(real angle) {
	real radAngle = math::toRadian(angle);

	real sinVal = std::sin(radAngle);
	real cosVal = std::cos(radAngle);

	Matrix4r mat;
	mat << cosVal, -sinVal, 0, 0,
		   sinVal, cosVal,  0, 0,
		   0,      0,       1, 0,
		   0,      0,       0, 1;

	return Transform(mat, mat.transpose());
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
std::ostream& operator << (std::ostream& o, const Transform& t)
{
	o << "Transform : matrix " << std::endl;
	for (auto i = 0; i < 4; i++)
	{
		for (auto j = 0; j < 4; j++)
		{
			o << t.myMatrix(i, j) << " ";
		}

		o << std::endl;
	}

	o << std::endl;

	o << "Transform : inverse matrix " << std::endl;
	for (auto i = 0; i < 4; i++)
	{
		for (auto j = 0; j < 4; j++)
		{
			o << t.myInvMatrix(i, j) << " ";
		}

		o << std::endl;
	}

	return o;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Transform Transform::fromLookAt(const Point3d& origin, const Point3d& target, const Vector3d& up)
{
	//Eigen::Vector3d dir = (target - origin).normalized();

#ifdef USE_ALIGN
	Vector3d dir = (target - origin).normalized();
	Vector3d left = (up.normalized().cross3(dir)).normalized();
	Vector3d newUp = dir.cross3(left).normalized();
	Matrix4r trafo;
	trafo << left, newUp, dir, origin;
	trafo(3, 3) = 1.;
#else
	Vector3d dir = (target - origin).normalized();
	Vector3d left = (up.normalized().cross(dir)).normalized();
	Vector3d newUp = dir.cross(left).normalized();
	Matrix4r trafo;
	trafo << left, newUp, dir, origin,
		0, 0, 0, 1;
#endif

	//Eigen::Matrix4d trafo;
	//trafo << left, newUp, dir, origin,
	//	0, 0, 0, 1;

	//Eigen::Affine3d affine(trafo);
	Eigen::Transform<real, 3, 2> affine(trafo);

	return Transform(affine.matrix());
}