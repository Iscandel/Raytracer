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
	return myBSDF->eval(infos) * myAlphaMap->eval(infos.uv).average();
}

Color AlphaBSDF::sample(BSDFSamplingInfos& infos, const Point2d& sample)
{
	real probaPassThrough = myAlphaMap->eval(infos.uv).average();
	if (sample.x() < probaPassThrough)
	{
		infos.wo = -infos.wi;
		infos.pdf = probaPassThrough;
		return Color(probaPassThrough);
	}
	else
	{
		real pdfAlpha = 1. - probaPassThrough;
		Color res = myBSDF->sample(infos, sample) / pdfAlpha;
		infos.pdf *= pdfAlpha;

		return res;
	}
}

real AlphaBSDF::pdf(const BSDFSamplingInfos& infos)
{
	return myAlphaMap->eval(infos.uv).average();
}

RT_REGISTER_TYPE(AlphaBSDF, BSDF)