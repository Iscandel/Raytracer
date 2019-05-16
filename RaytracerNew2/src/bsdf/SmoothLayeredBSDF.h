#pragma once
#pragma once
#include "BSDF.h"

#include "tools/Rng.h"
#include "Dielectric.h"
//#include "Conductor.h"
#include "MicrofacetDistribution.h"

//#include "Diffuse.h"

class Parameters;

class SmoothLayeredBSDF :
	public BSDF
{
public:
	SmoothLayeredBSDF(const Parameters& params);
	~SmoothLayeredBSDF();

	Color eval(const BSDFSamplingInfos& infos) override;

	Color sample(BSDFSamplingInfos& infos, const Point2d& sample) override;

	real pdf(const BSDFSamplingInfos& infos) override;
	//=============================================================================
	///////////////////////////////////////////////////////////////////////////////
	Color fresnel(real etaExt, real etaInt, real cosThetaI, real& etaI, real& etaT, real& relativeEta, real& cosThetaT);



	//inline Vector3d SmoothLayeredBSDF::refractIn(const Vector3d &wi, real &R);
	//inline Vector3d SmoothLayeredBSDF::refractOut(const Vector3d &wi, real &R);

	//real m_specularSamplingWeight;
	//Color evalReflection(const BSDFSamplingInfos & infos, const Color& fr, real alpha);

	//Color eval2(const BSDFSamplingInfos & infos);

	//Dielectric dielectric;
	BSDF::ptr myBaseBSDF;

	//MicrofacetDistribution myDistribution;

	real myEtaExt;
	real myEtaInt;
	real myAlpha;
	real myThickness;
	Color myAbsorbance;
	Rng myRng;

	//Color myEta;
	//Color myAbsorption;
};


