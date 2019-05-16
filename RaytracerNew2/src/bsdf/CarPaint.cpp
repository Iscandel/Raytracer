#include "CarPaint.h"

#include "core/Mapping.h"
#include "core/Math.h"
#include "factory/ObjectFactoryManager.h"
#include "tools/Tools.h"

#include <cmath>

CarPaint::CarPaint(const Parameters& params)
{
	myDistribution = MicrofacetDistribution("beckmann");

	if (params.hasString("preset"))
	{
		if (!fromPresets(params.getString("preset", ""), myKd, myKs, myM, myR))
		{
			ILogger::log(ILogger::ERRORS) << "Something got wrong in car paint preset: " << params.getString("presetName", "") << "\n";
		}

		myNumberOfLobes = myKs.size();
	}
	else
	{
		myNumberOfLobes = params.getInt("numberOfLobes", 3);

		myKs.resize(myNumberOfLobes);
		myM.resize(myNumberOfLobes);
		myR.resize(myNumberOfLobes);

		myKd = params.getColor("kd", Color());
			for (int i = 0; i < myNumberOfLobes; i++)
		{
			std::string ks = "ks" + tools::numToString(i + 1);
			std::string m = "m" + tools::numToString(i + 1);
			std::string r = "r" + tools::numToString(i + 1);

			if (!params.hasColor(ks) || !params.hasReal(m) || !params.hasReal(r))
				ILogger::log(ILogger::ERRORS) << "Incoherent parameters CarPaint\n";

			myKs[i] = params.getColor(ks, Color());
			myM[i] = params.getReal(m, 1.);
			myR[i] = params.getReal(r, 1.);
		}
	}
}


CarPaint::~CarPaint()
{
}

bool CarPaint::fromPresets(const std::string& presetName, Color& kd, std::vector<Color>& ks, std::vector<real>& m, std::vector<real>& r)
{
	if(presetName == "ford f8")
	{
		kd = Color::fromRGB(0.0012f, 0.0015f, 0.0018f);
		ks.push_back(Color::fromRGB(0.0049f, 0.0076f, 0.0120f));
		ks.push_back(Color::fromRGB(0.0100f, 0.0130f, 0.0180f));
		ks.push_back(Color::fromRGB(0.0070f, 0.0065f, 0.0077f));
		m.push_back(0.3200f);
		m.push_back(0.1100f);
		m.push_back(0.0130f);
		r.push_back(0.1500f);
		r.push_back(0.0870f);
		r.push_back(0.9000f);

		return true;
	}
	else if(presetName == "polaris silber")
	{
		kd = Color::fromRGB(0.0550f, 0.0630f, 0.0710f);
		ks.push_back(Color::fromRGB(0.0650f, 0.0820f, 0.0880f));
		ks.push_back(Color::fromRGB(0.1100f, 0.1100f, 0.1300f));
		ks.push_back(Color::fromRGB(0.0080f, 0.0130f, 0.0150f));
		m.push_back(0.38f);
		m.push_back(0.17f);
		m.push_back(0.013f);
		r.push_back(1.f);
		r.push_back(0.92f);
		r.push_back(0.9f);

		return true;
	}
	else if (presetName == "opel titan")
	{
		kd = Color::fromRGB(0.0110f, 0.0130f, 0.0150f);
		ks.push_back(Color::fromRGB(0.0570f, 0.0660f, 0.0780f));
		ks.push_back(Color::fromRGB(0.1100f, 0.1200f, 0.1300f));
		ks.push_back(Color::fromRGB(0.0095f, 0.0140f, 0.0160f));
		m.push_back(0.38f);
		m.push_back(0.17f);
		m.push_back(0.014f);
		r.push_back(0.85f);
		r.push_back(0.86f);
		r.push_back(0.90f);

		return true;
	}
	else if (presetName == "bmw 339")
	{
		kd = Color::fromRGB(0.0120f, 0.0150f, 0.0160f);
		ks.push_back(Color::fromRGB(0.0620f, 0.0760f, 0.0800f));
		ks.push_back(Color::fromRGB(0.1100f, 0.1200f, 0.1200f));
		ks.push_back(Color::fromRGB(0.0083f, 0.0150f, 0.0160f));
		m.push_back(0.39f);
		m.push_back(0.17f);
		m.push_back(0.013f);
		r.push_back(0.92f);
		r.push_back(0.87f);
		r.push_back(0.90f);

		return true;
	}
	else if (presetName == "2k acrylack")
	{
		kd = Color::fromRGB(0.4200f, 0.3200f, 0.1000f);
		ks.push_back(Color::fromRGB(0.0000f, 0.0000f, 0.0000f));
		ks.push_back(Color::fromRGB(0.0280f, 0.0260f, 0.0060f));
		ks.push_back(Color::fromRGB(0.0170f, 0.0075f, 0.0041f));
		m.push_back(0.88f);
		m.push_back(0.8f);
		m.push_back(0.015f);
		r.push_back(1.f);
		r.push_back(0.9f);
		r.push_back(0.17f);

		return true;
	}
	else if (presetName == "white")
	{
		kd = Color::fromRGB(0.6100f, 0.6300f, 0.5500f);
		ks.push_back(Color::fromRGB(2.6e-06f, 0.00031f, 3.1e-08f));
		ks.push_back(Color::fromRGB(0.0130f, 0.0110f, 0.0083f));
		ks.push_back(Color::fromRGB(0.0490f, 0.0420f, 0.0370f));
		m.push_back(1.f);
		m.push_back(0.15f);
		m.push_back(0.015f);
		r.push_back(0.049f);
		r.push_back(0.45f);
		r.push_back(0.17f);

		return true;
	}
	else if (presetName == "blue")
	{
		kd = Color::fromRGB(0.0079f, 0.0230f, 0.1000f);
		ks.push_back(Color::fromRGB(0.0011f, 0.0015f, 0.0019f));
		ks.push_back(Color::fromRGB(0.0250f, 0.0300f, 0.0430f));
		ks.push_back(Color::fromRGB(0.0590f, 0.0740f, 0.0820f));
		m.push_back(0.15f);
		m.push_back(0.043f);
		m.push_back(0.02f);
		r.push_back(1.f);
		r.push_back(0.094f);
		r.push_back(0.17f);

		return true;
	}
	else if (presetName == "blue matte")
	{
		kd = Color::fromRGB(0.0099f, 0.0360f, 0.1200f);
		ks.push_back(Color::fromRGB(0.0032f, 0.0045f, 0.0059f));
		ks.push_back(Color::fromRGB(0.1800f, 0.2300f, 0.2800f));
		ks.push_back(Color::fromRGB(0.0400f, 0.0490f, 0.0510f));
		m.push_back(0.16f);
		m.push_back(0.075f);
		m.push_back(0.034f);
		r.push_back(1.f);
		r.push_back(0.046f);
		r.push_back(0.17f);

		return true;
	}

	return false;
}

Color CarPaint::eval(const BSDFSamplingInfos & infos)
{
	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	if (cosThetaI <= 0. || cosThetaO <= 0.)
		return Color();

	Vector3d wh = (infos.wi + infos.wo).normalized();// / (infos.wi + infos.wo).squaredNorm();

	Color diffusePart = myKd * math::INV_PI;
	Color specularPart;
	for (int i = 0; i < myNumberOfLobes; i++)
	{
		specularPart += myKs[i] * myDistribution.D(wh, myM[i] * myM[i]) * //distributionBeckmann(wh, myM[i]) *
		//fresnel(myEtaExt, myEtaInt, infos.wi.dot(wh)) * //cosThetaI) *
			fresnelSchlick(Color(myR[i]), wh.dot(infos.wi)) * 
			myDistribution.G(infos.wi, infos.wo, wh, myM[i] * myM[i]) / (4 * cosThetaI * cosThetaO);;
		//shadowingTerm(infos.wi, infos.wo, wh, myM[i]) / (4 * cosThetaI * cosThetaO);
	}
	

	return diffusePart + specularPart;
}

Color CarPaint::sample(BSDFSamplingInfos& infos, const Point2d& sample)
{
	if (DifferentialGeometry::cosTheta(infos.wi) <= 0.f)
		return Color();

	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	//real alpha = (1.2f - 0.2f * std::sqrt(std::abs(cosThetaI))) * myAlpha;
	//Normal3d normal = myDistribution.sampleNormal(sample, alpha);

	if (sample.x() < 1. / (myNumberOfLobes + 1.))
	{
		infos.sampledType = BSDF::DIFFUSE;
		infos.wo = Mapping::squareToCosineWeightedHemisphere(Point2d(myRng.nextReal(), myRng.nextReal()));//sample);
	}
	else
	{
		for (int i = 0; i < myNumberOfLobes; i++)
		{
			if (sample.x() >= (i + 1.) / (myNumberOfLobes + 1.) && sample.x() < (i + 2.) / (myNumberOfLobes + 1.))
			{
				//real theta = std::atan(std::sqrt(-myM[i] * myM[i] * std::log(1 - myRng.nextReal())));
				//real phi = 2 * math::PI * sample.y();
				
				//Normal3d normal(std::sin(theta) * std::cos(phi), std::sin(theta) * std::sin(phi), std::cos(theta));
				Normal3d normal = myDistribution.sampleNormal(Point2d(myRng.nextReal(), sample.y()), myM[i] * myM[i]);
				infos.sampledType = BSDF::GLOSSY_REFLECTION;
				infos.wo = reflect(infos.wi, normal);
				break;
			}
		}
	}

	real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
	infos.pdf = pdf(infos);
	if (infos.pdf == 0)
		return Color();

	return eval(infos) * std::abs(cosThetaO) / infos.pdf;
}

real CarPaint::pdf(const BSDFSamplingInfos& infos)
{
	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	//if (cosThetaI <= 0. || cosThetaO <= 0.)
	//	return 0.;

	Vector3d wh = (infos.wi + infos.wo).normalized();
	real cosThetaH = DifferentialGeometry::cosTheta(wh);

	//real alpha = (1.2f - 0.2f * std::sqrt(std::abs(cosThetaI))) * myAlpha;

	real ratio = 1. / (myNumberOfLobes + 1.);

	real pdf = 0;
	real Jh = 1.f / (4 * wh.dot(infos.wo));
	for (int i = 0; i < myNumberOfLobes; i++)
		pdf += myDistribution.D(wh, myM[i] * myM[i]);//distributionBeckmann(wh, myM[i]);// *cosThetaH;
	return (cosThetaO * math::INV_PI + pdf * Jh) * ratio;
	//return myKs * myDistribution.D(wh, alpha) /*distributionBeckmann(wh)*/ * cosThetaH * Jh + (1 - myKs) * cosThetaO * math::INV_PI;
}

real CarPaint::distributionBeckmann(const Vector3d& wh, real m)
{
	if (DifferentialGeometry::cosTheta(wh) <= 0.)
		return 0.;

	real cosThetaH = DifferentialGeometry::cosTheta(wh);
	real tanThetaH = DifferentialGeometry::sinTheta(wh) / cosThetaH;

	return (real)1. / (m * m * cosThetaH * cosThetaH * cosThetaH) * math::fastExp((-tanThetaH * tanThetaH) / m * m);

	//real cosThetaH2 = DifferentialGeometry::cosTheta(wh) * DifferentialGeometry::cosTheta(wh);
	//real tanThetaH2 = DifferentialGeometry::sinTheta(wh) / cosThetaH;
	//real alpha2 = myAlpha * myAlpha;
	//return (math::fastExp(-tanThetaH2 / alpha2)) / (math::PI * alpha2 * cosThetaH2 * cosThetaH2);
}

real CarPaint::shadowingTerm(const Vector3d& wi, const Vector3d& wo, const Vector3d& wh, real roughness)
{
	return shadowingTermG1(wi, wh, roughness) * shadowingTermG1(wo, wh, roughness);
}

real CarPaint::shadowingTermG1(const Vector3d& v, const Vector3d& m, real roughness)
{
	if ((v.dot(m)) / (DifferentialGeometry::cosTheta(v)) <= 0)
		return 0.f;

	real tanThetaV = DifferentialGeometry::sinTheta(v) /
		DifferentialGeometry::cosTheta(v);
	real b = 1.f / (roughness * tanThetaV);

	if (b < 1.6f)
	{
		return (3.535f * b + 2.181f * b * b) / (1 + 2.276f * b + 2.577f * b * b);
	}
	else
	{
		return 1.f;
	}
}

Color CarPaint::fresnelSchlick(const Color& reflectionNormalIncidence, real cosTheta) 
{
	return reflectionNormalIncidence + (Color((real)1) - reflectionNormalIncidence) * std::pow((real)1 - cosTheta, 5);
}
























//inline Color SchlickEvaluate(const Color &normalIncidence, const float cosi) {
//	return normalIncidence + (Color(1.f) - normalIncidence) *
//		powf(1.f - cosi, 5.f);
//}
//
//
//inline float SchlickDistribution_SchlickZ(const float roughness, const float cosNH) {
//	if (roughness > 0.f) {
//		const float cosNH2 = cosNH * cosNH;
//		// expanded for increased numerical stability
//		const float d = cosNH2 * roughness + (1.f - cosNH2);
//		// use double division to avoid overflow in d*d product
//		return (roughness / d) / d;
//	}
//	return 0.f;
//}
//
//inline float SchlickDistribution_SchlickA(const Vector3d &H, const float anisotropy) {
//	const float h = sqrtf(H.x() * H.x() + H.y() * H.y());
//	if (h > 0.f) {
//		const float w = (anisotropy > 0.f ? H.x() : H.y()) / h;
//		const float p = 1.f - fabsf(anisotropy);
//		return sqrtf(p / (p * p + w * w * (1.f - p * p)));
//	}
//
//	return 1.f;
//}
//
//inline float SchlickDistribution_D(const float roughness, const Vector3d &wh,
//	const float anisotropy) {
//	const float cosTheta = fabsf(wh.z());
//	return SchlickDistribution_SchlickZ(roughness, cosTheta) * SchlickDistribution_SchlickA(wh, anisotropy) * math::INV_PI;
//}
//
//inline float SchlickDistribution_SchlickG(const float roughness,
//	const float costheta) {
//	return costheta / (costheta * (1.f - roughness) + roughness);
//}
//
//inline float SchlickDistribution_G(const float roughness, const Vector3d &localFixedDir,
//	const Vector3d &localSampledDir) {
//	return SchlickDistribution_SchlickG(roughness, fabsf(localFixedDir.z())) *
//		SchlickDistribution_SchlickG(roughness, fabsf(localSampledDir.z()));
//}
//
//static float GetPhi(const float a, const float b) {
//	return M_PI * .5f * sqrtf(a * b / (1.f - a * (1.f - b)));
//}
//
//inline void SchlickDistribution_SampleH(const float roughness, const float anisotropy,
//	const float u0, const float u1, Vector3d *wh, float *d, float *pdf) {
//	float u1x4 = u1 * 4.f;
//	const float cos2Theta = u0 / (roughness * (1 - u0) + u0);
//	const float cosTheta = sqrtf(cos2Theta);
//	const float sinTheta = sqrtf(1.f - cos2Theta);
//	const float p = 1.f - fabsf(anisotropy);
//	float phi;
//	if (u1x4 < 1.f) {
//		phi = GetPhi(u1x4 * u1x4, p * p);
//	}
//	else if (u1x4 < 2.f) {
//		u1x4 = 2.f - u1x4;
//		phi = M_PI - GetPhi(u1x4 * u1x4, p * p);
//	}
//	else if (u1x4 < 3.f) {
//		u1x4 -= 2.f;
//		phi = M_PI + GetPhi(u1x4 * u1x4, p * p);
//	}
//	else {
//		u1x4 = 4.f - u1x4;
//		phi = M_PI * 2.f - GetPhi(u1x4 * u1x4, p * p);
//	}
//
//	if (anisotropy > 0.f)
//		phi += M_PI * .5f;
//
//	*wh = Vector3d(sinTheta * cosf(phi), sinTheta * sinf(phi), cosTheta);
//	*d = SchlickDistribution_SchlickZ(roughness, cosTheta) * SchlickDistribution_SchlickA(*wh, anisotropy) * math::INV_PI;
//	*pdf = *d;
//}
//
//inline float SchlickDistribution_Pdf(const float roughness, const Vector3d &wh,
//	const float anisotropy) {
//	return SchlickDistribution_D(roughness, wh, anisotropy);
//}
//
////------------------------------------------------------------------------------
//// Schlick BSDF
////------------------------------------------------------------------------------
//
////float slg::SchlickBSDF_CoatingWeight(const Spectrum &ks, const Vector &localFixedDir) {
////	// Approximate H by using reflection direction for wi
////	const float u = fabsf(localFixedDir.z);
////	const Spectrum S = FresnelTexture::SchlickEvaluate(ks, u);
////
////	// Ensures coating is never sampled less than half the time
////	return .5f * (1.f + S.Filter());
////}
////
////Spectrum slg::SchlickBSDF_CoatingF(const bool fromLight, const Spectrum &ks, const float roughness,
////	const float anisotropy, const bool mbounce, const Vector &localFixedDir, const Vector &localSampledDir) {
////	const float coso = fabsf(localFixedDir.z);
////	const float cosi = fabsf(localSampledDir.z);
////
////	const Vector wh(Normalize(localFixedDir + localSampledDir));
////	const Spectrum S = FresnelTexture::SchlickEvaluate(ks, AbsDot(localSampledDir, wh));
////	assert(S.IsValid());
////
////	const float G = SchlickDistribution_G(roughness, localFixedDir, localSampledDir);
////
////	// Multibounce - alternative with interreflection in the coating creases
////	float factor = SchlickDistribution_D(roughness, wh, anisotropy) * G;
////	if (!fromLight)
////		factor = factor / (4.f * coso) +
////		(mbounce ? cosi * Clamp((1.f - G) / (4.f * coso * cosi), 0.f, 1.f) : 0.f);
////	else
////		factor = factor / (4.f * cosi) +
////		(mbounce ? coso * Clamp((1.f - G) / (4.f * cosi * coso), 0.f, 1.f) : 0.f);
////	assert(IsValid(factor));
////
////	return factor * S;
////}
////
////Spectrum slg::SchlickBSDF_CoatingSampleF(const bool fromLight, const Spectrum ks,
////	const float roughness, const float anisotropy, const bool mbounce,
////	const Vector &localFixedDir, Vector *localSampledDir, float u0, float u1, float *pdf) {
////	Vector wh;
////	float d, specPdf;
////	SchlickDistribution_SampleH(roughness, anisotropy, u0, u1, &wh, &d, &specPdf);
////	const float cosWH = Dot(localFixedDir, wh);
////	*localSampledDir = 2.f * cosWH * wh - localFixedDir;
////
////	if ((fabsf(localSampledDir->z) < DEFAULT_COS_EPSILON_STATIC) || (localFixedDir.z * localSampledDir->z < 0.f))
////		return Spectrum();
////
////	const float coso = fabsf(localFixedDir.z);
////	const float cosi = fabsf(localSampledDir->z);
////
////	*pdf = specPdf / (4.f * cosWH);
////	if (*pdf <= 0.f)
////		return Spectrum();
////
////	Spectrum S = FresnelTexture::SchlickEvaluate(ks, fabsf(cosWH));
////
////	const float G = SchlickDistribution_G(roughness, localFixedDir, *localSampledDir);
////	if (!fromLight)
////		//CoatingF(sw, *wi, wo, f_);
////		S *= (d / *pdf) * G / (4.f * coso) +
////		(mbounce ? cosi * Clamp((1.f - G) / (4.f * coso * cosi), 0.f, 1.f) / *pdf : 0.f);
////	else
////		//CoatingF(sw, wo, *wi, f_);
////		S *= (d / *pdf) * G / (4.f * cosi) +
////		(mbounce ? coso * Clamp((1.f - G) / (4.f * cosi * coso), 0.f, 1.f) / *pdf : 0.f);
////
////	return S;
////}
////
////float SchlickBSDF_CoatingPdf(const float roughness, const float anisotropy,
////	const Vector &localFixedDir, const Vector &localSampledDir) {
////	const Vector wh(Normalize(localFixedDir + localSampledDir));
////	return SchlickDistribution_Pdf(roughness, wh, anisotropy) / (4.f * AbsDot(localFixedDir, wh));
////}
//
//
//
//void ConcentricSampleDisk(const float u1, const float u2, float *dx, float *dy) {
//	float r, theta;
//	// Map uniform random numbers to $[-1,1]^2$
//	const float sx = 2.f * u1 - 1.f;
//	const float sy = 2.f * u2 - 1.f;
//	// Map square to $(r,\theta)$
//	// Handle degeneracy at the origin
//	if (sx == 0.f && sy == 0.f) {
//		*dx = 0.f;
//		*dy = 0.f;
//		return;
//	}
//	if (sx >= -sy) {
//		if (sx > sy) {
//			// Handle first region of disk
//			r = sx;
//			if (sy > 0.f)
//				theta = sy / r;
//			else
//				theta = 8.f + sy / r;
//		}
//		else {
//			// Handle second region of disk
//			r = sy;
//			theta = 2.f - sx / r;
//		}
//	}
//	else {
//		if (sx <= sy) {
//			// Handle third region of disk
//			r = -sx;
//			theta = 4.f - sy / r;
//		}
//		else {
//			// Handle fourth region of disk
//			r = -sy;
//			theta = 6.f + sx / r;
//		}
//	}
//	theta *= M_PI / 4.f;
//	*dx = r * cosf(theta);
//	*dy = r * sinf(theta);
//}
//
//
//inline Vector3d CosineSampleHemisphere(const float u1, const float u2, float *pdfW = NULL) {
//	Vector3d ret;
//	ConcentricSampleDisk(u1, u2, &ret.x(), &ret.y());
//	ret.z() = sqrtf(std::max(0.f, 1.f - ret.x() * ret.x() - ret.y() * ret.y()));
//
//	if (pdfW)
//		*pdfW = ret.z() * math::INV_PI;
//
//	return ret;
//}
//
//
//
//
//Color CarPaint::eval(const BSDFSamplingInfos & infos)
//{
//	Vector3d H = (infos.wi + infos.wo).normalized();
//	if (H.isZero())
//	{
//		return Color();
//	}
//	if (H.z() < 0.f)
//		H = -H;
//
//	float pdf = 0.f;
//	int n = 1; // already counts the diffuse layer
//
//			   // Absorption
//	const float cosi = fabsf(infos.wi.z());
//	const float coso = fabsf(infos.wo.z());
//	//const Spectrum alpha = Ka->GetSpectrumValue(hitPoint).Clamp(0.f, 1.f);
//	//const float d = depth->GetFloatValue(hitPoint);
//	//const Spectrum absorption = CoatingAbsorption(cosi, coso, alpha, d);
//
//	// Diffuse layer
//	Color result = /*absorption * */ myKd * math::INV_PI;//*fabsf(localLightDir.z);
//
//	// 1st glossy layer
//	const Color ks1 = myKs[0];
//	const real m1 = myM[0];
//	if (!ks1.isZero() && m1 > 0.f)
//	{
//		const float rough1 = m1 * m1;
//		const float r1 = myR[0];
//		result += (SchlickDistribution_D(rough1, H, 0.f) * SchlickDistribution_G(rough1, infos.wo, infos.wi) / (4.f * coso)) *
//			(ks1 * SchlickEvaluate(Color(r1), infos.wi.dot(H)));
//		pdf += SchlickDistribution_Pdf(rough1, H, 0.f);
//		++n;
//	}
//	const Color ks2 = myKs[1];
//	const float m2 = myM[1];
//	if (!ks2.isZero() && m2 > 0.f)
//	{
//		const float rough2 = m2 * m2;
//		const float r2 = myR[1];
//		result += (SchlickDistribution_D(rough2, H, 0.f) * SchlickDistribution_G(rough2, infos.wo, infos.wi) / (4.f * coso)) *
//			(ks2 * SchlickEvaluate(Color(r2), infos.wi.dot(H)));
//		pdf += SchlickDistribution_Pdf(rough2, H, 0.f);
//		++n;
//	}
//	const Color ks3 = myKs[2];
//	const float m3 = myM[2];
//	if (!ks3.isZero() && m3 > 0.f)
//	{
//		const float rough3 = m3 * m3;
//		const float r3 = myR[2];
//		result += (SchlickDistribution_D(rough3, H, 0.f) * SchlickDistribution_G(rough3, infos.wo, infos.wi) / (4.f * coso)) *
//			(ks3 * SchlickEvaluate(Color(r3), infos.wi.dot(H)));
//		pdf += SchlickDistribution_Pdf(rough3, H, 0.f);
//		++n;
//	}
//
//	//infos.sampledType = BSDFType::GLOSSY_REFLECTION
//
//	// Finish pdf computation
//	pdf /= 4.f * std::abs(infos.wi.dot(H));
//	//const Vecto2d &localFixedDir = hitPoint.fromLight ? localLightDir : localEyeDir;
//	//const Vector &localSampledDir = hitPoint.fromLight ? localEyeDir : localLightDir;
//	//if (directPdfW)
//	//	*directPdfW = (pdf + fabsf(localSampledDir.z) * INV_PI) / n;
//	//if (reversePdfW)
//	//	*reversePdfW = (pdf + fabsf(localFixedDir.z) * INV_PI) / n;
//
//	return result;
//}
//
//Color CarPaint::sample(BSDFSamplingInfos& infos, const Point2d& sample)
//{
//	if (fabsf(infos.wi.z()) < math::EPSILON)
//		return Color();
//
//	// Test presence of components
//	int n = 1; // already count the diffuse layer
//	int sampled = 0; // sampled layer
//	Color result(0.f);
//	float pdf = 0.f;
//	bool l1 = false, l2 = false, l3 = false;
//	// 1st glossy layer
//	const Color ks1 = myKs[0];
//	const float m1 = myM[0];
//	if (!ks1.isZero() && m1 > 0.f)
//	{
//		l1 = true;
//		++n;
//	}
//	// 2nd glossy layer
//	const Color ks2 = myKs[1];
//	const float m2 = myM[1];
//	if (!ks2.isZero() && m2 > 0.f)
//	{
//		l2 = true;
//		++n;
//	}
//	// 3rd glossy layer
//	const Color ks3 = myKs[2];
//	const float m3 = myM[2];
//	if (!ks3.isZero() && m3 > 0.f) {
//		l3 = true;
//		++n;
//	}
//
//	Vector3d wh;
//	float cosWH;
//	real absCosSampledDir;
//	if (sample.x() < 1.f / n) {
//		// Sample diffuse layer
//		infos.wo = /*Sgn(localFixedDir.z) * */CosineSampleHemisphere(myRng.nextReal(), myRng.nextReal(), &pdf);
//		infos.sampledType = BSDF::DIFFUSE;
//		//infos.wo = Mapping::squareToCosineWeightedHemisphere(Point2d(myRng.nextReal(), myRng.nextReal()));//sample);
//
//		absCosSampledDir = fabsf(infos.wo.z());
//		if (absCosSampledDir < math::EPSILON)
//			return Color();
//
//		//// Absorption
//		//const float cosi = fabsf(infos.wi.z());
//		//const float coso = fabsf(infos.wo.z());
//		//const Color alpha = Ka->GetSpectrumValue(hitPoint).Clamp(0.f, 1.f);
//		//const float d = depth->GetFloatValue(hitPoint);
//		//const Spectrum absorption = CoatingAbsorption(cosi, coso, alpha, d);
//
//		// Evaluate base BSDF
//		result = /*absorption * */ myKd * pdf;
//
//		wh = (infos.wi + infos.wo).normalized();
//		if (wh.z() < 0.f)
//			wh = -wh;
//		cosWH = std::abs(infos.wi.dot(wh));
//	}
//	else if (sample.x() < 2.f / n && l1) {
//		// Sample 1st glossy layer
//		sampled = 1;
//		const float rough1 = m1 * m1;
//		float d;
//		SchlickDistribution_SampleH(rough1, 0.f, myRng.nextReal(), myRng.nextReal(), &wh, &d, &pdf);
//		cosWH = infos.wi.dot(wh);
//		infos.wo = 2.f * cosWH * wh - infos.wi;
//		absCosSampledDir = fabsf(infos.wo.z());
//		cosWH = fabsf(cosWH);
//
//		if ((infos.wo.z() < math::EPSILON) ||
//			(infos.wi.z() * infos.wo.z() < 0.f))
//			return Color();
//
//		pdf /= 4.f * cosWH;
//		if (pdf <= 0.f)
//			return Color();
//
//		result = SchlickEvaluate(Color(myR[0]), cosWH);
//
//		const float G = SchlickDistribution_G(rough1, infos.wi, infos.wo);
//		
//		result *= d * G / (4.f * fabsf(infos.wo.z()));
//	}
//	else if ((sample.x() < 2.f / n ||
//		(!l1 && sample.x() < 3.f / n)) && l2) {
//		// Sample 2nd glossy layer
//		sampled = 2;
//		const float rough2 = m2 * m2;
//		float d;
//		SchlickDistribution_SampleH(rough2, 0.f, myRng.nextReal(), myRng.nextReal(), &wh, &d, &pdf);
//		cosWH = infos.wi.dot(wh);
//		infos.wo = 2.f * cosWH * wh - infos.wi;
//		absCosSampledDir = fabsf(infos.wo.z());
//		cosWH = fabsf(cosWH);
//
//		if ((infos.wo.z() < math::EPSILON) ||
//			(infos.wi.z() * infos.wo.z() < 0.f))
//			return Color();
//
//		pdf /= 4.f * cosWH;
//		if (pdf <= 0.f)
//			return Color();
//
//		result = SchlickEvaluate(Color(myR[1]), cosWH);
//
//		const float G = SchlickDistribution_G(rough2, infos.wi, infos.wo);
//
//		result *= d * G / (4.f * fabsf(infos.wo.z()));
//	}
//	else if (l3) {
//		// Sample 3rd glossy layer
//		sampled = 3;
//		const float rough3 = m3 * m3;
//		float d;
//		SchlickDistribution_SampleH(rough3, 0.f, myRng.nextReal(), myRng.nextReal(), &wh, &d, &pdf);
//		cosWH = infos.wi.dot(wh);
//		infos.wo = 2.f * cosWH * wh - infos.wi;
//		absCosSampledDir = fabsf(infos.wo.z());
//		cosWH = fabsf(cosWH);
//
//		if ((infos.wo.z() < math::EPSILON) ||
//			(infos.wi.z() * infos.wo.z() < 0.f))
//			return Color();
//
//		pdf /= 4.f * cosWH;
//		if (pdf <= 0.f)
//			return Color();
//
//		result = SchlickEvaluate(Color(myR[2]), cosWH);
//
//		const float G = SchlickDistribution_G(rough3, infos.wi, infos.wo);
//		
//		result *= d * G / (4.f * fabsf(infos.wo.z()));
//	}
//	else {
//		// Sampling issue
//		return Color();
//	}
//	infos.sampledType = BSDFType::GLOSSY_REFLECTION;
//	//*event = GLOSSY | REFLECT;
//	// Add other components
//	// Diffuse
//	if (sampled != 0) {
//		// Absorption
//		const float cosi = fabsf(infos.wi.z());
//		const float coso = fabsf(infos.wo.z());
//		//const Color alpha = Ka->GetSpectrumValue(hitPoint).Clamp(0.f, 1.f);
//		//const float d = depth->GetFloatValue(hitPoint);
//		//const Spectrum absorption = CoatingAbsorption(cosi, coso, alpha, d);
//
//		const float pdf0 = fabsf(infos.wo.z()) * math::INV_PI;
//		pdf += pdf0;
//		result = /*absorption * */myKd * pdf0;
//	}
//	// 1st glossy
//	if (l1 && sampled != 1) {
//		const float rough1 = m1 * m1;
//		const float d1 = SchlickDistribution_D(rough1, wh, 0.f);
//		const float pdf1 = SchlickDistribution_Pdf(rough1, wh, 0.f) / (4.f * cosWH);
//		if (pdf1 > 0.f) {
//			result += (d1 *
//				SchlickDistribution_G(rough1, infos.wi, infos.wo) /
//				(4.f * infos.wi.z())) *
//				SchlickEvaluate(Color(myR[0]), cosWH);
//			pdf += pdf1;
//		}
//	}
//	// 2nd glossy
//	if (l2 && sampled != 2) {
//		const float rough2 = m2 * m2;
//		const float d2 = SchlickDistribution_D(rough2, wh, 0.f);
//		const float pdf2 = SchlickDistribution_Pdf(rough2, wh, 0.f) / (4.f * cosWH);
//		if (pdf2 > 0.f) {
//			result += (d2 *
//				SchlickDistribution_G(rough2, infos.wi, infos.wo) /
//				(4.f * infos.wi.z())) *
//				SchlickEvaluate(Color(myR[1]), cosWH);
//			pdf += pdf2;
//		}
//	}
//	// 3rd glossy
//	if (l3 && sampled != 3) {
//		const float rough3 = m3 * m3;
//		const float d3 = SchlickDistribution_D(rough3, wh, 0.f);
//		const float pdf3 = SchlickDistribution_Pdf(rough3, wh, 0.f) / (4.f * cosWH);
//		if (pdf3 > 0.f) {
//			result += (d3 *
//				SchlickDistribution_G(rough3, infos.wi, infos.wo) /
//				(4.f * infos.wi.z())) *
//				SchlickEvaluate(Color(myR[2]), cosWH);
//			pdf += pdf3;
//		}
//	}
//	// Adjust pdf and result
//	infos.pdf = pdf / n;
//	return result / infos.pdf;
//}
//
//real CarPaint::pdf(const BSDFSamplingInfos& infos)
//{
//	Vector3d H = (infos.wi + infos.wo).normalized();
//	if (H.isZero())
//	{
//		return 0;
//	}
//	if (H.z() < 0.f)
//		H = -H;
//
//	float pdf = 0.f;
//	int n = 1; // already counts the diffuse layer
//
//			   // First specular lobe
//	const Color ks1 = myKs[0];
//	const float m1 = myM[0];
//	if (!ks1.isZero() && m1 > 0.f)
//	{
//		const float rough1 = m1 * m1;
//		pdf += SchlickDistribution_Pdf(rough1, H, 0.f);
//		++n;
//	}
//
//	// Second specular lobe
//	const Color ks2 = myKs[1];
//	const float m2 = myM[1];
//	if (!ks2.isZero() && m2 > 0.f)
//	{
//		const float rough2 = m2 * m2;
//		pdf += SchlickDistribution_Pdf(rough2, H, 0.f);
//		++n;
//	}
//
//	// Third specular lobe
//	const Color ks3 = myKs[2];
//	const float m3 = myM[2];
//	if (!ks3.isZero() && m3 > 0.f)
//	{
//		const float rough3 = m3 * m3;
//		pdf += SchlickDistribution_Pdf(rough3, H, 0.f);
//		++n;
//	}
//
//	// Finish pdf computation
//	pdf /= 4.f * std::abs(infos.wo.dot(H));
//	
//	return (pdf + fabsf(infos.wo.z()) * math::INV_PI) / n;
//}







/*
//yellow
<bsdf type="CarPaint">
<color name="kd" value="0.42 0.32 0.1"/>
<color name="ks1" value="0 0 0"/>
<color name="ks2" value="0.11 0.12 0.12"/>
<color name="ks3" value="0.028 0.026 0.006"/>
<double name="m1" value="0.88"/>
<double name="m2" value="0.8"/>
<double name="m3" value="0.015"/>
<double name="r1" value="1."/>
<double name="r2" value="0.9"/>
<double name="r3" value="0.17"/>
</bsdf>

//blue
<bsdf type="CarPaint">
<color name="kd" value="0.0079 0.023 0.1"/>
<color name="ks1" value="0.0011 0.0015 0.0019"/>
<color name="ks2" value="0.025 0.03 0.043"/>
<color name="ks3" value="0.059 0.074 0.082"/>
<real name="m1" value="0.15"/>
<real name="m2" value="0.043"/>
<real name="m3" value="0.02"/>
<real name="r1" value="1."/>
<real name="r2" value="0.094"/>
<real name="r3" value="0.17"/>
</bsdf>
*/

RT_REGISTER_TYPE(CarPaint, BSDF)