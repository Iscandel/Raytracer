#pragma once

#include "Geometry.h"
#include "Ray.h"
#include "Tools.h"

#include <iostream>

///////////////////////////////////////////////////////////////////////////////
/// \brief AABB bounding box. Such a rectangular box is identified by its 
/// 2 extremum points 
///////////////////////////////////////////////////////////////////////////////
class BoundingBox
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
public:
	///////////////////////////////////////////////////////////////////////////////
	/// \brief Default constructor
	///////////////////////////////////////////////////////////////////////////////
	BoundingBox();

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Constructor
	/// 
	/// \param point : single point to initialize the box.
	///////////////////////////////////////////////////////////////////////////////
	BoundingBox(const Point3d& point);

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Constructor. The box is initialized by the min and max components 
	/// of the 2 points.
	///
	/// \param p1 : first point
	///
	/// \param p2 : second point.
	///////////////////////////////////////////////////////////////////////////////
	BoundingBox(const Point3d& p1, const Point3d& p2);

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Destructor.
	///////////////////////////////////////////////////////////////////////////////
	~BoundingBox();

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Returns the bounding box's surface area.
	///////////////////////////////////////////////////////////////////////////////
	double getSurfaceValue() const
	{
		Vector3d dist = myMax - myMin;
		
		return 2. * (dist.x() * dist.y() + dist.y() * dist.z() + dist.x() * dist.z());
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Returns the volume contained in the bounding box.
	///////////////////////////////////////////////////////////////////////////////
	double getVolumeValue() const
	{
		Vector3d dist = myMax - myMin;

		return dist.x() * dist.y() * dist.z();
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Returns the center of the box.
	///////////////////////////////////////////////////////////////////////////////
	Point3d getCentroid() const
	{
		return (0.5 * (myMax + myMin));
		//Vector3d dist = myMax - myMin;

		//return Point3d(myMin.x() + 0.5 * dist.x(),
		//			   myMin.y() + 0.5 * dist.y(),
		//			   myMin.z() + 0.5 * dist.z());
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Returns the min point of the box
	///////////////////////////////////////////////////////////////////////////////
	const Point3d& getMin() const { return myMin; }

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Returns the max point of the box
	///////////////////////////////////////////////////////////////////////////////
	const Point3d& getMax() const { return myMax; }

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Expands the box with the given point.
	/// 
	/// \param box : The box to expand.
	///
	/// \param point : given point that should belong to the new box.
	///
	/// \return The expanded bounding box.
	///////////////////////////////////////////////////////////////////////////////
	static BoundingBox unionBox(const BoundingBox& box, const Point3d& point)
	{
		BoundingBox res = box;

		res.myMin.x() = std::min(box.myMin.x(), point.x());
		res.myMin.y() = std::min(box.myMin.y(), point.y());
		res.myMin.z() = std::min(box.myMin.z(), point.z());

		res.myMax.x() = std::max(box.myMax.x(), point.x());
		res.myMax.y() = std::max(box.myMax.y(), point.y());
		res.myMax.z() = std::max(box.myMax.z(), point.z());

		return res;

	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Expands the box with the given box.
	/// 
	/// \param box : The box to expand.
	///
	/// \param point : given box that should belong to the new box.
	///
	/// \return The expanded bounding box.
	///////////////////////////////////////////////////////////////////////////////
	static BoundingBox unionBox(const BoundingBox& box1, const BoundingBox& box2) 
	{
		BoundingBox res;

		res.myMin.x() = std::min(box1.myMin.x(), box2.myMin.x());
		res.myMin.y() = std::min(box1.myMin.y(), box2.myMin.y());
		res.myMin.z() = std::min(box1.myMin.z(), box2.myMin.z());

		res.myMax.x() = std::max(box1.myMax.x(), box2.myMax.x());
		res.myMax.y() = std::max(box1.myMax.y(), box2.myMax.y());
		res.myMax.z() = std::max(box1.myMax.z(), box2.myMax.z());

		return res;
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Intersection test between a box and a ray
	/// 
	/// \param ray : the ray to test
	///
	/// \param nearT : First intersection parametric distance
	///
	/// \param farT : Second intersection parametric distance
	///
	/// \return True if the ray passes through the box, false otherwise
	///////////////////////////////////////////////////////////////////////////////
	bool intersection(const Ray &ray, double &nearT, double &farT) const {
		nearT = -tools::MAX_DOUBLE;//std::numeric_limits<double>::infinity();
		farT = tools::MAX_DOUBLE;//std::numeric_limits<double>::infinity();

		for (int i = 0; i<3; i++) {
			double origin = ray.myOrigin[i];
			double minVal = myMin[i], maxVal = myMax[i];

			if (ray.direction()[i] == 0) {
				if (origin < minVal || origin > maxVal)
					return false;
			}
			else {
				double t1 = (minVal - origin) * ray.invDir()[i];
				double t2 = (maxVal - origin) * ray.invDir()[i];

				if (t1 > t2)
					std::swap(t1, t2);

				nearT = std::max(t1, nearT);
				farT = std::min(t2, farT);

				if (!(nearT <= farT))
					return false;
			}
		}

		return (ray.myMinT <= farT && ray.myMaxT >= nearT);
		//return true;
	}

	friend std::ostream& operator << (std::ostream& o, const BoundingBox& box)
	{
		o << "[" << box.myMin.x() << " " << box.myMin.y() << " " << box.myMin.z() << "; " 
			<< box.myMax.x() << " " << box.myMax.y() << " " << box.myMax.z() << "]";

		return o;
	}

	friend class Transform;
protected:
	///Min value
	Point3d myMin;
	///Max value
	Point3d myMax;
};

