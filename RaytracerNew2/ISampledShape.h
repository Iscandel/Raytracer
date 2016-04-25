#pragma once

#include "Geometry.h"

class ISampledShape
{
public:
	ISampledShape();
	~ISampledShape();

	///////////////////////////////////////////////////////////////////////////
	/// \brief Samples a point on the surface
	///
	///
	//p : [0,1] : square to shape
	//sampled : position of the sampled point
	//normal : Returned normal
	virtual void sample(const Point2d& sample, Point3d& sampled, Normal3d& normal) = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Computes the pdf value at the sampled point. The default 
	/// implementation returns the the pdf wrt the solid angle at the point p to 
	/// be lit, subtented by the surface. Pdf wrt solid angles lead to reduced
	/// variance, compared to pdf wrt surface area.
	///
	/// \param p : Point to be lit
	/// \param sampled : sampled point on the light surface
	/// \param normal : normal at the sampled point
	///
	/// \return the pdf value at the considered point.
	///////////////////////////////////////////////////////////////////////////
	virtual double pdf(const Point3d& p, const Point3d& sampled, const Normal3d& normal);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Returns surface area of the shape
	///////////////////////////////////////////////////////////////////////////
	virtual double surfaceArea() = 0;
};

