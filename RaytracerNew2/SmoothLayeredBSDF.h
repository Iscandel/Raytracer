#pragma once
#pragma once
#include "BSDF.h"

#include "Rng.h"
#include "Dielectric.h"
#include "Conductor.h"
#include "MicrofacetDistribution.h"

#include "Diffuse.h"

class Parameters;

class SmoothLayeredBSDF :
	public BSDF
{
public:
	SmoothLayeredBSDF(const Parameters& params);
	~SmoothLayeredBSDF();

	Color eval(const BSDFSamplingInfos& infos) override;

	Color sample(BSDFSamplingInfos& infos, const Point2d& sample) override;

	double pdf(const BSDFSamplingInfos& infos) override;
	//=============================================================================
	///////////////////////////////////////////////////////////////////////////////
	Color fresnel(double etaExt, double etaInt, double cosThetaI, double& etaI, double& etaT, double& relativeEta, double& cosThetaT);



	inline Vector3d SmoothLayeredBSDF::refractIn(const Vector3d &wi, double &R);
	inline Vector3d SmoothLayeredBSDF::refractOut(const Vector3d &wi, double &R);

	double m_specularSamplingWeight;
	//Color evalReflection(const BSDFSamplingInfos & infos, const Color& fr, double alpha);

	//Color eval2(const BSDFSamplingInfos & infos);

	//Dielectric dielectric;
	BSDF::ptr myBaseBSDF;

	//MicrofacetDistribution myDistribution;

	double myEtaExt;
	double myEtaInt;
	double myAlpha;
	double myThickness;
	Color myAbsorbance;
	Rng myRng;

	//Color myEta;
	//Color myAbsorption;
};


