#pragma once
#include "BSDF.h"
#include "BSSRDF.h"
#include "AreaLight.h"
#include "Ray.h"
#include "GeometricShape.h"
#include "Medium.h"
#include "RaytracerObject.h"

#include <memory>

class Intersection;

///////////////////////////////////////////////////////////////////////////////
/// \brief Base class for primitives
///////////////////////////////////////////////////////////////////////////////
class IPrimitive : public std::enable_shared_from_this<IPrimitive>, public RaytracerObject
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

	virtual BSSRDF::ptr getBSSRDF()
	{
		throw std::runtime_error("This primitive class shouldn't call getBSSRDF().\n");
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

	virtual Medium::ptr getInteriorMedium() {
		throw std::runtime_error("This primitive class shouldn't call getInteriorMedium().\n");
	}

	virtual Medium::ptr getExteriorMedium() {
		throw std::runtime_error("This primitive class shouldn't call getExteriorMedium().\n");
	}

	//virtual void initialize(const Scene&) {}

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
	SimplePrimitive(GeometricShape::ptr object, BSDF::ptr bsdf = BSDF::ptr(), BSSRDF::ptr bssrdf = BSSRDF::ptr(), 
		Medium::ptr interior = Medium::ptr(), Medium::ptr exterior = Medium::ptr())
		:myObject(object), myBSDF(bsdf), myBSSRDF(bssrdf), myInteriorMedium(interior), myExteriorMedium(exterior)
	{
		//if(myExteriorMedium)
		//	myExteriorMedium->setOwnerBBox(getWorldBoundingBox()); //bvh getWorldBoundingBox() should be called....
		//if (myInteriorMedium)
		//	myInteriorMedium->setOwnerBBox(getWorldBoundingBox());
	}
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

	///////////////////////////////////////////////////////////////////////////
	/// \brief Returns the underlying BSDF.
	///
	/// \return The BSDF from the primitive.
	///////////////////////////////////////////////////////////////////////////
	BSSRDF::ptr getBSSRDF() override
	{
		return myBSSRDF;
	}

	Medium::ptr getInteriorMedium() override {
		//myInteriorMedium->setOwnerBBox(getWorldBoundingBox());
		return myInteriorMedium;
	}

	Medium::ptr getExteriorMedium() override {
		//if(myExteriorMedium->setOwnerBBox(getWorldBoundingBox());
		return myExteriorMedium;
	}

	void initialize(Scene& scene) override
	{
		myObject->initialize(scene);
		if(myBSDF) myBSDF->initialize(scene);
		if(myBSSRDF) myBSSRDF->initialize(scene);
		if(myInteriorMedium) myInteriorMedium->initialize(scene);
		if(myExteriorMedium) myExteriorMedium->initialize(scene);
	}
		
protected:
	GeometricShape::ptr myObject;
	BSDF::ptr myBSDF;
	BSSRDF::ptr myBSSRDF;
	Medium::ptr myInteriorMedium;
	Medium::ptr myExteriorMedium;
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

	Medium::ptr getInteriorMedium() override {
		return myPrimitive->getInteriorMedium();
	}

	Medium::ptr getExteriorMedium() override {
		return myPrimitive->getExteriorMedium();
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


