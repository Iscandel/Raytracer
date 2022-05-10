#ifndef H__GEOMETRY_160520150025__H
#define H__GEOMETRY_160520150025__H

#include "tools/Logger.h"

#include <cmath>
#include <limits.h>

#include <math.h>

#include <Eigen/Core>
#include <Eigen/Geometry>

namespace priv {
#ifdef DOUBLE_PRECISION
	typedef double real;
#else
	typedef float real;
#endif
}
using priv::real;

///Forward declarations
template<class T, int DIM> struct Point;
template<class T, int DIM> struct Vect;

typedef Vect<real, 2> Vector2d;
typedef Vect<int, 2> Vector2i;

typedef Point<real, 2> Point2d;
typedef Point<int, 2> Point2i;


///////////////////////////////////////////////////////////////////////////////
/// \brief Base class for generic points.
///////////////////////////////////////////////////////////////////////////////
template<class T, int DIM>
struct Point : public Eigen::Matrix<T, DIM, 1>
{
	Point(T val = (T) 0.) {Eigen::Matrix<T, DIM, 1>::setConstant(val); }
	Point(T x, T y) : Eigen::Matrix<T, DIM, 1>(x, y) {}
#ifdef USE_ALIGN
	Point(T x, T y, T z, T w = (T)0) : Eigen::Matrix<T, DIM, 1>(x, y, z, w) {}
#else
	Point(T x, T y, T z) : Eigen::Matrix<T, DIM, 1>(x, y, z) {}
#endif

	template<typename OtherDerived>
	Point(const Eigen::MatrixBase<OtherDerived>& other)
    : Eigen::Matrix<T, DIM, 1>(other)
	{ }

	template <typename Derived> Point &operator=(const Eigen::MatrixBase<Derived>& other) {
		this->Base::operator=(other);
		return *this;
	}

	//operator Vect<real, 3>()
	//{
	//	return Vect<real, 3>(x(), y(), z());
	//}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Base class for generic vectors.
///////////////////////////////////////////////////////////////////////////////
template<class T, int DIM>
struct Vect : public Eigen::Matrix<T, DIM, 1>
{

	Vect(T val = (T) 0.) {Eigen::Matrix<T, DIM, 1>::setConstant(val);}
	Vect(T x, T y) : Eigen::Matrix<T, DIM, 1>(x, y) {}
#ifdef USE_ALIGN
	Vect(T x, T y, T z, T w = (T) 0) : Eigen::Matrix<T, DIM, 1>(x, y, z, w) {}
#else
	Vect(T x, T y, T z) : Eigen::Matrix<T, DIM, 1>(x, y, z) {}
#endif
	Vect(const Point<T, DIM>& p1, const Point<T, DIM>& p2) 
	{
		Vect<T, DIM> v(p2 - p1);
		for(unsigned int i = 0; i < DIM; i++) 
		{
			this->operator[](i) = v[i];
		}
	}

	template <typename Derived> Vect(const Eigen::MatrixBase<Derived>& p)
        : Eigen::Matrix<T, DIM, 1>(p) { }

    /// Assign a vector from MatrixBase (needed to play nice with Eigen)
    template <typename Derived> Vect &operator=(const Eigen::MatrixBase<Derived>& p) {
        this->Base::operator=(p);
        return *this;
    }

	//inline operator Point<real, 3> ()
	//{
	//	return Point<real, 3>(x(), y(), z());
	//}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Base class for generic points.
///////////////////////////////////////////////////////////////////////////////
template<class T, int DIM>
struct Normal : public Eigen::Matrix<T, DIM, 1>
{
	//typedef 3 Size;

	Normal(T val = (T) 0.) { Eigen::Matrix<T, DIM, 1>::setConstant(val); }

	//Normal(T x, T y, T z) : Eigen::Matrix<T, 3, 1>(x, y, z) {}

#ifdef USE_ALIGN
	Normal(T x, T y, T z, T w = (T)0) : Eigen::Matrix<T, DIM, 1>(x, y, z, w) {}
#else
	Normal(T x, T y, T z) : Eigen::Matrix<T, DIM, 1>(x, y, z) {}
#endif

	template<typename OtherDerived>
	Normal(const Eigen::MatrixBase<OtherDerived>& other)
		: Eigen::Matrix<T, DIM, 1>(other)
	{ }

	template <typename Derived> Normal &operator=(const Eigen::MatrixBase<Derived>& other) {
		this->Base::operator=(other);
		return *this;
	}
};


///typedef for the real data structures used in the program
#ifdef USE_ALIGN
typedef Vect<real, 4> Vector3d;
typedef Vect<int, 4> Vector3i;
template <typename Type>
using Vect3 = Vect<Type, 4>;

typedef Point<real, 4> Point3d;
typedef Point<int, 4> Point3i;
template <typename Type>
using Point3 = Point<Type, 4>;

typedef Normal<real, 4> Normal3d;
template <typename Type>
using Normal3 = Normal<Type, 4>;
#else
typedef Vect<real, 3> Vector3d;
typedef Vect<int, 3> Vector3i;
template <typename Type>
using Vect3 = Vect<Type, 3>;

typedef Point<real, 3> Point3d;
typedef Point<int, 3> Point3i;
template <typename Type>
using Point3 = Point<Type, 3>;

typedef Normal<real, 3> Normal3d;
template <typename Type>
using Normal3 = Normal<Type, 3>;
#endif

inline real sphericalPhiFromCartesian(const Vector3d& v)
{
	//real res =  std::atan2(v.y(), v.x());
	real res = std::atan2(v.x(), -v.z());
	return res < 0.f ? res + 2 * (real)3.14159265358979323846 : res;
}

inline real sphericalThetaFromCartesian(const Vector3d& v)
{
	if (std::abs(v.y()) > 1.f)
		ILogger::log() << "sphericalThetaFromCartesian: Acos() is in range [-1; 1]\n";

	//real val = v.z();
	//val = val < -1. ? -1. : val  > 1. ? 1. : val;
	//return std::acos(v.z());
	return std::acos(v.y());
}

inline Vector3d cartesianFromSpherical(real theta, real phi)
{
	real sinTheta = std::sin(theta);
	real cosTheta = std::cos(theta);
	real cosPhi = std::cos(phi);
	real sinPhi = std::sin(phi);

	return Vector3d(sinPhi * sinTheta, cosTheta, -cosPhi * sinTheta);
	//return Vector3d(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta);
}

#endif
