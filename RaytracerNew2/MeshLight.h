#pragma once
#include "AreaLight.h"

class ISampledShape;
class Parameters;

/////////////////////////////////////////////////////////////////////
/// \brief Specialized class to handle mesh based lights and to do
/// fast computations
/////////////////////////////////////////////////////////////////////
class MeshLight : public AreaLight
{
public:
	MeshLight(const Parameters& params);
	~MeshLight();

	LightSamplingInfos sample(const Point3d& pFrom, const Point2d& sample) override;

	Color power() const override;

	Color le(const Vector3d& direction, const Normal3d& normal = Normal3d()) const override
	{
		if (direction.dot(normal) > 0.)
			return myRadiance;
		return Color();
	}

	//void setShape(ISampledShape* shape);//std::shared_ptr<ISampledShape> shape) { myShape = shape; }

//protected:
//	//std::weak_ptr<ISampledShape> myShape;
//	ISampledShape* myShape;
//
//	Color myRadiance;
};

