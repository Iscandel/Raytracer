#pragma once
#include "BSDF.h"
#include "AreaLight.h"
#include "Ray.h"
#include "GeometricShape.h"

#include <memory>

class Intersection;

///////////////////////////////////////////////////////////////////////////////
/// \brief Base class for primitives
///////////////////////////////////////////////////////////////////////////////
class IPrimitive : public std::enable_shared_from_this<IPrimitive>
{
public:
	typedef std::shared_ptr<IPrimitive> ptr;
public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Default constructor
	///////////////////////////////////////////////////////////////////////////
	IPrimitive();

	///////////////////////////////////////////////////////////////////////////
	/// \brief Destructor
	///////////////////////////////////////////////////////////////////////////
	virtual ~IPrimitive();

	///////////////////////////////////////////////////////////////////////////
	/// \brief Determines if the given ray intersects the primitive.
	///
	/// \param ray The ray to test
	/// \param inter intersection data
	/// \param shadowRay if true, no intersection data are computed
	///
	/// \return true if an intersection occured, false otherwise
	///////////////////////////////////////////////////////////////////////////
	virtual bool intersection(const Ray& ray, Intersection& inter, bool shadowRay = false) = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Returns the world bouding box of the primitive
	///
	/// \return World bounding box.
	///////////////////////////////////////////////////////////////////////////
	virtual BoundingBox getWorldBoundingBox() = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Computes the primitive centroid.
	///
	/// \return The centroid.
	///////////////////////////////////////////////////////////////////////////
	virtual Point3d getCentroid() { throw std::runtime_error("no"); }

	//virtual void setBSDF(BSDF::ptr bsdf) {
	//	throw std::runtime_error("This primitive class shouldn't call setBSDF().\n");
	//}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Returns the underlying BSDF.
	///
	/// \return The BSDF from the primitive.
	///////////////////////////////////////////////////////////////////////////
	virtual BSDF::ptr getBSDF() {
		throw std::runtime_error("This primitive class shouldn't call getBSDF().\n");
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Updates the intersection data and fills the differential geometry
	/// informations.
	///////////////////////////////////////////////////////////////////////////
	virtual void getDifferentialGeometry(DifferentialGeometry& trueGeometry,
		DifferentialGeometry& shadingGeometry, Intersection& inter) 
	{
		trueGeometry;
		shadingGeometry;
		inter;
		throw std::runtime_error("This primitive class shouldn't call getDifferentialGeometry().\n");
	}

	bool isLight() const { return myLight ? true : false; }

	Light::ptr getLight() { return myLight; }

	virtual void addLight(Light::ptr light) { myLight = light; }

	Color le(const Vector3d& direction, const Normal3d& normal) const { return myLight->le(direction, normal); }

protected:
	Light::ptr myLight;

};

///////////////////////////////////////////////////////////////////////////////
/// \brief Primitive class. Holds geometry and BSDF informations.
///////////////////////////////////////////////////////////////////////////////
class SimplePrimitive : public IPrimitive
{
public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor
	///////////////////////////////////////////////////////////////////////////
	SimplePrimitive(GeometricShape::ptr object, BSDF::ptr bsdf = BSDF::ptr()) :myObject(object), myBSDF(bsdf) {}
public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Determines if the given ray intersects the primitive.
	///
	/// \param ray The ray to test
	/// \param inter intersection data
	/// \param shadowRay if true, no intersection data are computed
	///
	/// \return true if an intersection occured, false otherwise
	///////////////////////////////////////////////////////////////////////////
	bool intersection(const Ray& ray, Intersection& inter, bool shadowRay = false) override;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Returns the world bouding box of the primitive
	///
	/// \return World bounding box.
	///////////////////////////////////////////////////////////////////////////
	BoundingBox getWorldBoundingBox() override
	{
		return myObject->getWorldBoundingBox();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Computes the primitive centroid.
	///
	/// \return The centroid.
	///////////////////////////////////////////////////////////////////////////
	virtual Point3d getCentroid()
	{
		return myObject->getCentroid();
	}

	void getDifferentialGeometry(DifferentialGeometry& trueGeometry, 
		DifferentialGeometry& shadingGeometry, Intersection& inter)
	{
		myObject->getDifferentialGeometry(trueGeometry, shadingGeometry, inter);
	}
	//void setBSDF(BSDF::ptr bsdf) 
	//{
	//	myBSDF = bsdf;
	//}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Returns the underlying BSDF.
	///
	/// \return The BSDF from the primitive.
	///////////////////////////////////////////////////////////////////////////
	BSDF::ptr getBSDF() override
	{
		return myBSDF;
	}
		
protected:
	GeometricShape::ptr myObject;
	BSDF::ptr myBSDF;
	//Material::ptr myMaterial;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Instance of a primitive. This class reuses an existing primitve to
/// save memory. It adds a new transformation from the primitive to the world
///////////////////////////////////////////////////////////////////////////////
class InstancePrimitive : public IPrimitive
{
public:
	///////////////////////////////////////////////////////////////////////////////
	/// \brief Constructor
	///////////////////////////////////////////////////////////////////////////////
	InstancePrimitive(Transform::ptr toObject) : myWorldToObjectWorldTransform(toObject) {}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Determines if the given ray intersects the primitive.
	///
	/// \param ray The ray to test
	/// \param inter intersection data
	/// \param shadowRay if true, no intersection data are computed
	///
	/// \return true if an intersection occured, false otherwise
	///////////////////////////////////////////////////////////////////////////
	bool intersection(const Ray& ray, Intersection& inter, bool shadowRay = false) override;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Computes the primitive centroid.
	///
	/// \return The centroid.
	///////////////////////////////////////////////////////////////////////////
	BoundingBox getWorldBoundingBox() override;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Returns the underlying BSDF.
	///
	/// \return The BSDF from the primitive.
	///////////////////////////////////////////////////////////////////////////
	BSDF::ptr getBSDF() override
	{
		return myPrimitive->getBSDF();
	}

	void addLight(Light::ptr light) override 
	{ 
		myPrimitive->addLight(light);
		IPrimitive::addLight(light);
	}

protected:
	IPrimitive::ptr myPrimitive;
	Transform::ptr myWorldToObjectWorldTransform;
	//Material::ptr myMaterial;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Base class for acceleration structures
///////////////////////////////////////////////////////////////////////////////
class AccelStructure : public IPrimitive
{
//public:
//	virtual bool intersection(const Ray& ray, Intersection& inter) = 0;
//	virtual BoundingBox getWorldBoundingBox()
//	{
//		
//	}
};


