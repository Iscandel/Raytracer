#pragma once
#include "BSDF.h"

#include "MicrofacetDistribution.h"
#include "tools/Rng.h"

class CarPaint :
	public BSDF
{
public:
	CarPaint(const Parameters& params);
	~CarPaint();

	Color eval(const BSDFSamplingInfos & infos) override;
	Color sample(BSDFSamplingInfos& infos, const Point2d& sample) override;
	real pdf(const BSDFSamplingInfos& infos) override;

	real distributionBeckmann(const Vector3d& wh, real m);
	Color fresnelSchlick(const Color& reflectionNormalIncidence, real cosTheta);
	real shadowingTerm(const Vector3d& wi, const Vector3d& wo, const Vector3d& wh, real roughness);

	real shadowingTermG1(const Vector3d& v, const Vector3d& m, real roughness);

protected:
	bool fromPresets(const std::string& presetName, Color& kd, std::vector<Color>& ks, std::vector<real>& m, std::vector<real>& r);

protected:
	int myNumberOfLobes;
	Color myKd;
	std::vector<Color> myKs;
	std::vector<real> myM;
	std::vector<real> myR;

	MicrofacetDistribution myDistribution;

	Rng myRng;
};

