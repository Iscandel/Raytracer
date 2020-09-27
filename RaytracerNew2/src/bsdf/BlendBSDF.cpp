#include "BlendBSDF.h"

#include "bsdf/Diffuse.h"
#include "texture/ConstantTexture.h"
#include "factory/ObjectFactoryManager.h"

BlendBSDF::BlendBSDF(const Parameters& params)
	:BSDF(params)
{
	//Don't forget to put gamma == 1 in the scene file for the texture... !!!!!!!!!!!!!

	myBlendFactor = params.getTexture("blendFactor", Texture::ptr(new ConstantTexture(Color(0.))));
	myBSDF1 = params.getBSDF("bsdf", BSDF::ptr(new Diffuse(Parameters())));
	myBSDF2 = params.getBSDF("bsdf2", BSDF::ptr(new Diffuse(Parameters())));
}


BlendBSDF::~BlendBSDF()
{
}

Color BlendBSDF::eval(const BSDFSamplingInfos & infos)
{
	if (DifferentialGeometry::cosTheta(infos.wi) <= 0.f || DifferentialGeometry::cosTheta(infos.wo) <= 0.f)
		return Color(0.f);
	Color fac = myBlendFactor->eval(infos.uv);

	return fac * myBSDF1->eval(infos) + (Color(1.) - fac) * myBSDF2->eval(infos);
}


Color BlendBSDF::sample(BSDFSamplingInfos & infos, const Point2d & _sample)
{
	if (DifferentialGeometry::cosTheta(infos.wi) <= 0.f)
		return Color(0.f);

	Point2d sample(_sample);
	Color val;
	real fac = myBlendFactor->eval(infos.uv).luminance();
	if (sample.x() < fac)
	{
		sample.x() /= fac;
		val = myBSDF1->sample(infos, sample);
		infos.pdf *= fac;
		val /= fac;
	} 
	else
	{
		sample.x() = (sample.x() - fac) / (real(1.) - fac);
		val = myBSDF2->sample(infos, sample);
		infos.pdf *= real(1) - fac;
	}

	return val;
}

real BlendBSDF::pdf(const BSDFSamplingInfos & infos)
{
	if (DifferentialGeometry::cosTheta(infos.wi) < 0.f || DifferentialGeometry::cosTheta(infos.wo) <= 0.f)
		return real(0.f);
	real fac = myBlendFactor->eval(infos.uv).luminance();

	return fac * myBSDF1->pdf(infos) + (real(1.) - fac) * myBSDF2->pdf(infos);
}

RT_REGISTER_TYPE(BlendBSDF, BSDF)