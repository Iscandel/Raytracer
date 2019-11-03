#pragma once

#include "core/Color.h"
#include "tools/Common.h"
#include "core/DifferentialGeometry.h"
#include "core/Geometry.h"
#include "core/RaytracerObject.h"

#include <memory>

//enum BSDF::BSDFType;

class Intersection;

struct BSDFSamplingInfos;

class BSDF : public RaytracerObject
{
public:
	typedef std::shared_ptr<BSDF> ptr;

public:

	enum BSDFType
	{
		UNKNOWN             = 0x00000000,
		DIFFUSE_REFLECTION  = 0x00000001,
		DELTA_REFLECTION    = 0x00000002,
		GLOSSY_REFLECTION   = 0x00000004,
		DELTA_TRANSMISSION  = 0x00000008,
		GLOSSY_TRANSMISSION = 0x00000010,
		ENABLE_BACK_SIDE    = 0x00000100,
		DIFFUSE				= 0x00000200,
		DELTA = DELTA_REFLECTION | DELTA_TRANSMISSION,
		GLOSSY = GLOSSY_REFLECTION | GLOSSY_TRANSMISSION,
		REFLECTION = DIFFUSE_REFLECTION | DELTA_REFLECTION | GLOSSY_REFLECTION,
		TRANSMISSION = DELTA_TRANSMISSION | GLOSSY_REFLECTION,
		ALL = REFLECTION |TRANSMISSION
	};

public:
	BSDF(const Parameters& params);
	virtual ~BSDF();

	//in local coords, N = (0,0,1) -> Reflected = 2 N (N.D) - D = (0,0,2dz) - (dx, dy, dz) = (-dx, -dy, dz) ?
	Vector3d reflect(const Vector3d& vect) { return Vector3d(-vect.x(), -vect.y(), vect.z()); }

	Vector3d reflect(const Vector3d& vect, const Normal3d& normal) { return  2 * vect.dot(normal) * Vector3d(normal) - vect;}

	Vector3d refract(const Vector3d& vect, real cosThetaT, real relativeEta) { return Vector3d(relativeEta * -vect.x(), relativeEta * -vect.y(), cosThetaT); }

	Vector3d refract(const Vector3d& vect, const Normal3d& normal, real relativeEta, real cosThetaT)
	{
		return normal * (vect.dot(normal) * relativeEta + cosThetaT) - vect * relativeEta;
	}

	bool refract(const Vector3d& vect, Vector3d& refracted, real relativeEta, const Normal3d& normal) 
	{ 
		real c = vect.dot(normal);
		int sign = vect.z() >= 0 ? 1 : -1;
		real sqrtTerm = 1 + relativeEta * relativeEta * (c * c - 1);
		if (sqrtTerm < 0)
			return false;
		real left = relativeEta * c - sign * std::sqrt(std::max((real)0., (sqrtTerm)));
		refracted = left * normal - relativeEta * vect;
		return true;
	}

	virtual Color eval(const BSDFSamplingInfos& infos) = 0;

	virtual Color sample(BSDFSamplingInfos& infos, const Point2d& sample) = 0;

	virtual real pdf(const BSDFSamplingInfos& infos) = 0;

	virtual bool isShadowCatcher() const { return false; }
};


struct BSDFSamplingInfos
{
	BSDFSamplingInfos(const Intersection& inter, const Vector3d& _wi, const Vector3d& _wo);

	BSDFSamplingInfos(const Intersection& inter, const Vector3d& _wi);

	BSDFSamplingInfos(const Vector3d& _wi, BSDF::BSDFType _request = BSDF::ALL)
		:wi(_wi)
		, request(_request)
	{
		sampledType = BSDF::ALL;
		pdf = -1.;
		relativeEta = 1.;
		measure = Measure::SOLID_ANGLE;
		shadowCaught = false;
	}

	BSDFSamplingInfos(const Vector3d& _wi, const Vector3d& _wo)
		:wi(_wi)
		, wo(_wo)
		, request(BSDF::ALL)
	{
		sampledType = BSDF::ALL;
		pdf = -1.;
		relativeEta = 1.;
		measure = Measure::SOLID_ANGLE;
		shadowCaught = false;
	}

	///Incident direction (in local coords), but sense from BSDF intersection point to 
	///incident view point (in
	Vector3d wi;

	///Reflected / refracted direction in local coords
	Vector3d wo;

	///Flag to specify that we want a special type of action on the bsdf
	BSDF::BSDFType request;

	///Flag indicating the type of the BSDF sampled
	BSDF::BSDFType sampledType;

	///Flag indicating the type of measure done when sampling the BSDF
	Measure measure;

	real relativeEta;

	///PDF associated to the reflected direction sampled
	real pdf;

	Point2d uv;

	DifferentialGeometry shadingFrame;

	bool shadowCaught;

	real ao = 1.;
};