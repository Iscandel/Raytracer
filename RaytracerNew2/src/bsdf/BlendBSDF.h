#pragma once
#include "BSDF.h"

#include "texture\Texture.h"

class BlendBSDF :
	public BSDF
{
public:
	BlendBSDF(const Parameters& params);
	~BlendBSDF();

	Color eval(const BSDFSamplingInfos & infos) override;
	Color sample(BSDFSamplingInfos & infos, const Point2d & sample) override;
	real pdf(const BSDFSamplingInfos & infos);

protected:
	Texture::ptr myBlendFactor;
	BSDF::ptr myBSDF1;
	BSDF::ptr myBSDF2;
};

