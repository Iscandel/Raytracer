#include "BSDF.h"

#include "MicrofacetDistribution.h"
#include "Parameters.h"

class RoughDielectric:
	public BSDF
{
public:
	RoughDielectric(const Parameters& params);
	~RoughDielectric();

	Color eval(const BSDFSamplingInfos& infos) override;

	Color sample(BSDFSamplingInfos& infos, const Point2d& sample);

	real pdf(const BSDFSamplingInfos& infos);
	real pdfReflection(const BSDFSamplingInfos& infos);
	real pdfRefraction(const BSDFSamplingInfos& infos);

protected:
	//real distributionBeckmann(const Vector3d& hr);

	//real shadowingTerm(const Vector3d& wi, const Vector3d& wo, const Vector3d& wh);
	//real shadowingTermG1(const Vector3d& v, const Vector3d& m);

	//Color fresnel(real myEtaExt, real myEtaInt, real cosThetaI);
	//Color fresnel(real myEtaExt, real myEtaInt, real cosThetaI, real& etaI, real& etaT, real& relativeEta, real& cosThetaT);

	void fresnelData(real cosThetaI, real& etaI, real& etaT, real& relativeEta, real& cosThetaT);

	Vector3d halfDirTransmitted(const Vector3d& wi, const Vector3d& wo, real etaI, real etaT)
	{
		return ((etaI < etaT ? -1.f : -1.f) * (etaI * wi + etaT * wo)).normalized();
	}

	Color evalReflection(const BSDFSamplingInfos & infos, bool adjustAlpha = false);

	Color evalRefraction(const BSDFSamplingInfos & infos, bool adjustAlpha = false);

protected:
	Texture::ptr myReflectanceTexture;
	Texture::ptr myTransmittedTexture;
	Texture::ptr myAlphaTexture;
	//real myAlpha;
	real myEtaInt;
	real myEtaExt;
	MicrofacetDistribution myDistribution;
};



