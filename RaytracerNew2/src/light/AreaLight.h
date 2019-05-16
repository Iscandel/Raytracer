#pragma once
#include "Light.h"

#include <memory>

class ISampledShape;
class Parameters;

///////////////////////////////////////////////////////////////////////////////
/// \brief General class for surface based lights.
///////////////////////////////////////////////////////////////////////////////
class AreaLight : public Light
{
public:
	typedef std::shared_ptr<AreaLight> ptr;

public:
	AreaLight(const Parameters& params);
	virtual ~AreaLight();

	virtual LightSamplingInfos sample(const Point3d& pFrom, const Point2d& sample) override;

	virtual real pdf(const Point3d& pFrom, const LightSamplingInfos& infos) override;

	virtual Color power() const override;

	virtual Color le(const Vector3d& direction, const Normal3d& normal = Normal3d()) const override
	{
		if(direction.dot(normal) > 0.)
			return myRadiance;
		return Color();
	}

	void setShape(std::shared_ptr<ISampledShape> shape) { myShape = shape; }

	bool isDelta() const override { return false; }

protected:
	std::weak_ptr<ISampledShape> myShape;

	Color myRadiance;
};

