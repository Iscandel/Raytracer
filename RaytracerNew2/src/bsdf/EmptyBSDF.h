#pragma once
#include "BSDF.h"
class EmptyBSDF :
	public BSDF
{
public:
	EmptyBSDF(const Parameters&);
	~EmptyBSDF();
};

