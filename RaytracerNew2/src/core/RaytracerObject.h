#pragma once

class Scene;
class Parameters;

#include <memory>

class RaytracerObject
{
public:
	RaytracerObject() {}
	RaytracerObject(const Parameters& params);

	~RaytracerObject();

	virtual void initialize(Scene&) {}

	const Parameters* const getParameters() const { return myParameters.get(); }

protected:
	std::unique_ptr<Parameters> myParameters;
};

