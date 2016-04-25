#include "BSDF.h"

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
	double distributionBlinn(const Vector3d& hr);
	double distributionBeckmann(const Vector3d& hr);

	double shadowingTerm(const Vector3d& wi, const Vector3d& wo, const Vector3d& wh);
	double shadowingTermG1(const Vector3d& v, const Vector3d& m);

	Color fresnel(double myEtaExt, double myEtaInt, double cosThetaI);
	Color RoughDielectric::fresnel(double myEtaExt, double myEtaInt, double cosThetaI, double& etaI, double& etaT, double& relativeEta, double& cosThetaT);

	void fresnelData(double cosThetaI, double& etaI, double& etaT, double& relativeEta, double& cosThetaT);

	Color evalReflection(const BSDFSamplingInfos & infos);

	Color evalRefraction(const BSDFSamplingInfos & infos);

protected:
	double myAlpha;
	double myEtaInt;
	double myEtaExt;
};



