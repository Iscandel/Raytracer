#include "Diffuse.h"

#include "DifferentialGeometry.h"
#include "Mapping.h"
#include "ObjectFactoryManager.h"
#include "Tools.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Diffuse::Diffuse(const Parameters& list)
{
	myAlbedo = list.getColor("albedo", Color(0.5, 0.5, 0.5));
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Diffuse::~Diffuse()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color Diffuse::eval(const BSDFSamplingInfos& infos)
{
	if (DifferentialGeometry::cosTheta(infos.wi) <= 0. ||
		DifferentialGeometry::cosTheta(infos.wo) <= 0.)
		return Color();

	return myAlbedo * tools::INV_PI;
}

//inline void sincosf(float theta, float *_sin, float *_cos) {
//	*_sin = sinf(theta);
//	*_cos = cosf(theta);
//}
//Point2d squareToUniformDiskConcentric(const Point2d &sample) {
//	float r1 = 2.0f*sample.x() - 1.0f;
//	float r2 = 2.0f*sample.y() - 1.0f;
//
//	Point2d coords;
//	if (r1 == 0 && r2 == 0) {
//		coords = Point2d(0, 0);
//	}
//	else if (r1 > -r2) { /* Regions 1/2 */
//		if (r1 > r2)
//			coords = Point2d(r1, (M_PI / 4.0f) * r2 / r1);
//		else
//			coords = Point2d(r2, (M_PI / 4.0f) * (2.0f - r1 / r2));
//	}
//	else { /* Regions 3/4 */
//		if (r1<r2)
//			coords = Point2d(-r1, (M_PI / 4.0f) * (4.0f + r2 / r1));
//		else
//			coords = Point2d(-r2, (M_PI / 4.0f) * (6.0f - r1 / r2));
//	}
//
//	Point2d result;
//	float f1, f2;
//	sincosf(coords.y(), &f1, &f2);
//	result.x() = f1; result.y() = f2;
//	return result*coords.x();
//}
//Vector3d squareToCosineHemisphere(const Point2d &sample) {
//	Point2d p = squareToUniformDiskConcentric(sample);
//	float z = std::sqrt(std::max((double)0,
//		1.0f - p.x()*p.x() - p.y()*p.y()));
//
//	return Vector3d(p.x(), p.y(), z);
//}




//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color Diffuse::sample(BSDFSamplingInfos& infos, const Point2d& sample) 
{
	if (DifferentialGeometry::cosTheta(infos.wi) <= 0.)
		return Color();

	infos.sampledType = BSDF::DIFFUSE;
	infos.wo = Mapping::squareToCosineWeightedHemisphere(sample);
	infos.pdf = pdf(infos);

	//eval() * cos theta / pdf = albedo	
	return myAlbedo;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
double Diffuse::pdf(const BSDFSamplingInfos& infos)
{
	if (DifferentialGeometry::cosTheta(infos.wi) <= 0. ||
		DifferentialGeometry::cosTheta(infos.wo) <= 0.)
		return 0.;

	double cosTheta = DifferentialGeometry::cosTheta(infos.wo);

	//Pdf associated to cosine weighted hemisphere sampling is cos theta / pi
	return tools::INV_PI * cosTheta;
}

RT_REGISTER_TYPE(Diffuse, BSDF)