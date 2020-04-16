#include "AlphaBSDF.h"

#include "core/Parameters.h"
#include "texture/ConstantTexture.h"
#include "factory/ObjectFactoryManager.h"

AlphaBSDF::AlphaBSDF(const Parameters& params)
:BSDF(params)
{
	if (params.hasTexture("alpha"))
		myAlphaMap = params.getTexture("alpha", nullptr);
	else
		myAlphaMap = Texture::ptr(new ConstantTexture(params.getColor("alpha", Color(1.))));


	myBSDF = params.getBSDF("bsdf", nullptr);
	if (myBSDF == nullptr)
		ILogger::log(ILogger::ERRORS) << "AlphaBSDF: bsdf is null\n";
}


AlphaBSDF::~AlphaBSDF()
{
}

Color AlphaBSDF::eval(const BSDFSamplingInfos& infos)
{
	//if (infos.measure == Measure::DISCRETE)
	//	return Color(1.);

	//return Color(0.);

	if (infos.measure == Measure::DISCRETE && std::abs(1.- infos.wi.dot(-infos.wo)) < math::EPSILON)
		return Color(1.) - myAlphaMap->eval(infos.uv);
	else if (infos.measure == Measure::SOLID_ANGLE)
		return myBSDF->eval(infos) * myAlphaMap->eval(infos.uv);
	else
		return Color(0.);
}

Color AlphaBSDF::sample(BSDFSamplingInfos& infos, const Point2d& _sample)
{
	//infos.wo = -infos.wi;
	//infos.pdf = 1.;//1. - probaBeingBlocked;
	//infos.sampledType = BSDFType::UNKNOWN;
	//return Color(1.);

	Point2d sample(_sample);
	real probaBeingBlocked = myAlphaMap->eval(infos.uv).luminance();
	if (sample.x() < probaBeingBlocked)
	{
		sample.x() /= probaBeingBlocked;
		real pdfAlpha = probaBeingBlocked;
		Color res = myBSDF->sample(infos, sample) * myAlphaMap->eval(infos.uv) / pdfAlpha;// / pdfAlpha;
		infos.pdf *= pdfAlpha;
		return res;
	}
	else
	{
		infos.wo = -infos.wi;
		infos.pdf = real(1. - probaBeingBlocked);
		infos.sampledType = BSDFType::UNKNOWN;
		return (Color(1.) - myAlphaMap->eval(infos.uv)) / infos.pdf;//1- (eval / pdf) 
	}
}

real AlphaBSDF::pdf(const BSDFSamplingInfos& infos)
{
	if (infos.measure == Measure::DISCRETE &&  std::abs(1. - infos.wi.dot(-infos.wo)) < math::EPSILON)
		return 1. - myAlphaMap->eval(infos.uv).luminance();
	else if (infos.measure == Measure::SOLID_ANGLE)
		return myBSDF->pdf(infos) * myAlphaMap->eval(infos.uv).luminance();
	else
		return (real)0;
	
	//return 0;
	//return 1. - myAlphaMap->eval(infos.uv).luminance();
	//return myBSDF->pdf(infos) * myAlphaMap->eval(infos.uv).average();
}

RT_REGISTER_TYPE(AlphaBSDF, BSDF)