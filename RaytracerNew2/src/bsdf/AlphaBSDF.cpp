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
	return myBSDF->eval(infos) * myAlphaMap->eval(infos.uv);
}

Color AlphaBSDF::sample(BSDFSamplingInfos& infos, const Point2d& _sample)
{
	Point2d sample(_sample);
	real probaBeingBlocked = myAlphaMap->eval(infos.uv).average();
	if (sample.x() < probaBeingBlocked)
	{
		sample.x() /= probaBeingBlocked;
		real pdfAlpha = probaBeingBlocked;
		Color res = myBSDF->sample(infos, sample);// / pdfAlpha;
		infos.pdf *= pdfAlpha;
		return res;
	}
	else
	{
		infos.wo = -infos.wi;
		infos.pdf = 1. - probaBeingBlocked;
		return Color(1.) - myBSDF->eval(infos);//1- (eval / pdf) 
	}
}

real AlphaBSDF::pdf(const BSDFSamplingInfos& infos)
{
	return myBSDF->pdf(infos) * myAlphaMap->eval(infos.uv).average();
}

RT_REGISTER_TYPE(AlphaBSDF, BSDF)