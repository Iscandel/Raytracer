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

	double pdf(const BSDFSamplingInfos& infos);
	double pdfReflection(const BSDFSamplingInfos& infos);
	double pdfRefraction(const BSDFSamplingInfos& infos);

protected:
	double distributionBeckmann(const Vector3d& hr);

	double shadowingTerm(const Vector3d& wi, const Vector3d& wo, const Vector3d& wh);
	double shadowingTermG1(const Vector3d& v, const Vector3d& m);

	Color fresnel(double myEtaExt, double myEtaInt, double cosThetaI);
	Color RoughDielectric::fresnel(double myEtaExt, double myEtaInt, double cosThetaI, double& etaI, double& etaT, double& relativeEta, double& cosThetaT);

	void fresnelData(double cosThetaI, double& etaI, double& etaT, double& relativeEta, double& cosThetaT);

	Vector3d halfDirTransmitted(const Vector3d& wi, const Vector3d& wo, double etaI, double etaT)
	{
		return ((etaI < etaT ? -1 : -1) * (etaI * wi + etaT * wo)).normalized();
	}

	Color evalReflection(const BSDFSamplingInfos & infos);

	Color evalRefraction(const BSDFSamplingInfos & infos);

protected:
	Texture::ptr myReflectanceTexture;
	Texture::ptr myTransmittedTexture;
	double myAlpha;
	double myEtaInt;
	double myEtaExt;
	MicrofacetDistribution myDistribution;
};



