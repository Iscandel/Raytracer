#pragma once

#include "Logger.h"
#include "ObjectFactory.h"
#include "Primitive.h"

///////////////////////////////////////////////////////////////////////////
/// \brief Creates a given shape (excepted mesh)
///////////////////////////////////////////////////////////////////////////
template<class BaseType, class ObjectType>//, typename ...Args>
class GeometricShapeFactory : public ObjectFactory<BaseType>
{
public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor.
	///////////////////////////////////////////////////////////////////////////
	GeometricShapeFactory()
		:ObjectFactory<BaseType>(ObjectStaticType<ObjectType>::get())
	{
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Creates a new object with the given name.
	///
	/// \param params : Parameter list
	///
	/// \return A smart pointer to the newly created widget.
	///////////////////////////////////////////////////////////////////////////
	//typename BaseType::ptr create(Args&&... args) override//const std::string& name) 
	//std::vector<GeometricShape::ptr> create(const Parameters& params) //override
	//{
	//	std::vector<GeometricShape::ptr> res;
	//	res.push_back(std::make_shared<ObjectType>(params));
	//	return res;
	//}
	typename BaseType::ptr create(const Parameters& params) override
	{
		GeometricShape::ptr shape = std::make_shared<ObjectType>(params);
		BSDF::ptr bsdf = params.getBSDF("bsdf", BSDF::ptr());
		Light::ptr light = params.getLight("light", Light::ptr());
		Medium::ptr interiorMedium = params.getMedium("interior", Medium::ptr());
		Medium::ptr exteriorMedium = params.getMedium("exterior", Medium::ptr());
		typename BaseType::ptr primitive = std::make_shared<SimplePrimitive>(shape, bsdf, interiorMedium, exteriorMedium);

		if (light)
		{
			((AreaLight*)light.get())->setShape(shape);
			primitive->addLight(light);
		}
		
		ILogger::log(ILogger::ALL) << "Bounding box " << primitive->getWorldBoundingBox() << "\n";
		return primitive;
	}
};

