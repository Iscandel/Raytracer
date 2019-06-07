#pragma once

#include "core/Parameters.h"
#include "tools/Common.h"

class MediumPreset
{
public:
	struct MediumData
	{
		//std::string name;
		real sigmaS[3];
		real sigmaA[3];
		real g[3];
		real eta;
	};

public:
	typedef std::map<std::string, MediumData, Ci_less> DataByName;

public:
	MediumPreset();
	~MediumPreset();

	static void fromParams(const Parameters & params, Color & sigmaA, Color & sigmaS, Color& g, real& etaInt)
	{
		static bool init = false;

		if (!init)
		{
			initMap();
			init = true;
		}

		std::string materialName = params.getString("preset", "");
		//std::transform(materialName.begin(), materialName.end(), materialName.begin(), ::tolower);
		std::map<std::string, MediumData>::iterator it = dataMap.find(materialName);

		if (it == dataMap.end() && materialName != "")
			ILogger::log() << "material " << materialName << " not found.\n";
		if (materialName == "" || it == dataMap.end())
		{
			sigmaA = params.getColor("sigmaA", Color(0.));
			sigmaS = params.getColor("sigmaS", Color(0.));
			if (params.hasColor("g"))
				g = params.getColor("g", Color());
			else
			{
				real gReal = params.getReal("g", 0.);
				g.setfromRGB(gReal, gReal, gReal);
			}		
			etaInt = params.getReal("etaInt", 1.);
		}
		else
		{
			sigmaA.setfromRGB(it->second.sigmaA[0], it->second.sigmaA[1], it->second.sigmaA[2]);
			sigmaS.setfromRGB(it->second.sigmaS[0], it->second.sigmaS[1], it->second.sigmaS[2]);
			sigmaA *= 100; //paper units are mm-1
			sigmaS *= 100;
			g.setfromRGB(it->second.g[0], it->second.g[1], it->second.g[2]);
			etaInt = it->second.eta;
		}
	}

protected:
	static void initMap();

protected:
	//static MediumData data[];
	static DataByName dataMap;

};

//static MediumPreset::MediumData data[] = {
//	/* Fitted data from "A Practical Model for Subsurface scattering" (Jensen et al.). No anisotropy data available. */
//	{ "Apple",{ 2.29f, 2.39f, 1.97f },{ 0.0030f, 0.0034f, 0.046f },{ 0.0f, 0.0f, 0.0f }, 1.3f },
//	{ "Chicken1",{ 0.15f, 0.21f, 0.38f },{ 0.0015f, 0.077f,  0.19f },{ 0.0f, 0.0f, 0.0f }, 1.3f },
//	{ "Chicken2",{ 0.19f, 0.25f, 0.32f },{ 0.0018f, 0.088f,  0.20f },{ 0.0f, 0.0f, 0.0f }, 1.3f },
//	{ "Cream",{ 7.38f, 5.47f, 3.15f },{ 0.0002f, 0.0028f, 0.0163f },{ 0.0f, 0.0f, 0.0f }, 1.3f },
//	{ "Ketchup",{ 0.18f, 0.07f, 0.03f },{ 0.061f,  0.97f,   1.45f },{ 0.0f, 0.0f, 0.0f }, 1.3f },
//	{ "Marble",{ 2.19f, 2.62f, 3.00f },{ 0.0021f, 0.0041f, 0.0071f },{ 0.0f, 0.0f, 0.0f }, 1.5f },
//	{ "Potato",{ 0.68f, 0.70f, 0.55f },{ 0.0024f, 0.0090f, 0.12f },{ 0.0f, 0.0f, 0.0f }, 1.3f },
//	{ "Skimmilk",{ 0.70f, 1.22f, 1.90f },{ 0.0014f, 0.0025f, 0.0142f },{ 0.0f, 0.0f, 0.0f }, 1.3f },
//	{ "Skin1",{ 0.74f, 0.88f, 1.01f },{ 0.032f,  0.17f,   0.48f },{ 0.0f, 0.0f, 0.0f }, 1.3f },
//	{ "Skin2",{ 1.09f, 1.59f, 1.79f },{ 0.013f,  0.070f,  0.145f },{ 0.0f, 0.0f, 0.0f }, 1.3f },
//	{ "Spectralon",{ 11.6f, 20.4f, 14.9f },{ 0.00f,   0.00f,   0.00f },{ 0.0f, 0.0f, 0.0f }, 1.3f },
//	{ "Wholemilk",{ 2.55f, 3.21f, 3.77f },{ 0.0011f, 0.0024f, 0.014f },{ 0.0f, 0.0f, 0.0f }, 1.3f },
//
//	/* From "Acquiring Scattering Properties of Participating Media by Dilution"
//	by Narasimhan, Gupta, Donner, Ramamoorthi, Nayar, Jensen (SIGGRAPH 2006) */
//	{ "Lowfat Milk",{ 13.1157f, 15.4445f, 17.9572f },{ 0.00287f, 0.00575f, 0.01150f },{ 0.93200f, 0.90200f, 0.85900f }, 1.33f },
//	{ "Reduced Milk",{ 13.7335f, 15.6003f, 17.8007f },{ 0.00256f, 0.00511f, 0.01278f },{ 0.81900f, 0.79700f, 0.74600f }, 1.33f },
//	{ "Regular Milk",{ 18.2052f, 20.3826f, 22.3698f },{ 0.00153f, 0.00460f, 0.01993f },{ 0.75000f, 0.71400f, 0.68100f }, 1.33f },
//	{ "Espresso",{ 7.78262f, 8.13050f, 8.53875f },{ 4.79838f, 6.57512f, 8.84925f },{ 0.90700f, 0.89600f, 0.88000f }, 1.33f },
//	{ "Mint Mocha Coffee",{ 3.51133f, 4.14383f, 5.59667f },{ 3.77200f, 5.82283f, 7.82000f },{ 0.91000f, 0.90700f, 0.91400f }, 1.33f },
//	{ "Lowfat Soy Milk",{ 2.03838f, 2.32875f, 3.90281f },{ 0.00144f, 0.00719f, 0.03594f },{ 0.85000f, 0.85300f, 0.84200f }, 1.33f },
//	{ "Regular Soy Milk",{ 4.66325f, 5.20183f, 8.74575f },{ 0.00192f, 0.00958f, 0.06517f },{ 0.87300f, 0.85800f, 0.83200f }, 1.33f },
//	{ "Lowfat Chocolate Milk",{ 9.83710f, 11.4954f, 13.1629f },{ 0.01150f, 0.03680f, 0.15640f },{ 0.93400f, 0.92700f, 0.91600f }, 1.33f },
//	{ "Regular Chocolate Milk",{ 10.5685f, 13.1416f, 15.2202f },{ 0.01006f, 0.04313f, 0.14375f },{ 0.86200f, 0.83800f, 0.80600f }, 1.33f },
//	{ "Coke",{ 0.00254f, 0.00299f, 0.00000f },{ 0.10014f, 0.16503f, 0.24680f },{ 0.96500f, 0.97200f, 0.00000f }, 1.33f },
//	{ "Pepsi",{ 0.00083f, 0.00203f, 0.00000f },{ 0.09164f, 0.14158f, 0.20729f },{ 0.92600f, 0.97900f, 0.00000f }, 1.33f },
//	{ "Sprite",{ 0.00011f, 0.00014f, 0.00014f },{ 0.00189f, 0.00183f, 0.00200f },{ 0.94300f, 0.95300f, 0.95200f }, 1.33f },
//	{ "Gatorade",{ 0.03668f, 0.04488f, 0.05742f },{ 0.02479f, 0.01929f, 0.00888f },{ 0.93300f, 0.93300f, 0.93500f }, 1.33f },
//	{ "Chardonnay",{ 0.00021f, 0.00033f, 0.00048f },{ 0.01078f, 0.01186f, 0.02400f },{ 0.91400f, 0.95800f, 0.97500f }, 1.33f },
//	{ "White Zinfandel",{ 0.00022f, 0.00033f, 0.00046f },{ 0.01207f, 0.01618f, 0.01984f },{ 0.91900f, 0.94300f, 0.97200f }, 1.33f },
//	{ "Merlot",{ 0.00081f, 0.00000f, 0.00000f },{ 0.11632f, 0.25191f, 0.29434f },{ 0.97400f, 0.00000f, 0.00000f }, 1.33f },
//	{ "Budweiser Beder",{ 0.00029f, 0.00055f, 0.00059f },{ 0.01149f, 0.02491f, 0.05779f },{ 0.91700f, 0.95600f, 0.98200f }, 1.33f },
//	{ "Coors Light Beer",{ 0.00062f, 0.00127f, 0.00000f },{ 0.00616f, 0.01398f, 0.03498f },{ 0.91800f, 0.96600f, 0.00000f }, 1.33f },
//	{ "Clorox",{ 0.02731f, 0.03302f, 0.03695f },{ 0.00335f, 0.01489f, 0.02630f },{ 0.91200f, 0.90500f, 0.89200f }, 1.33f },
//	{ "Apple Juice",{ 0.00257f, 0.00311f, 0.00413f },{ 0.01296f, 0.02374f, 0.05218f },{ 0.94700f, 0.94900f, 0.94500f }, 1.33f },
//	{ "Cranberry Juice",{ 0.00196f, 0.00238f, 0.00301f },{ 0.03944f, 0.09422f, 0.12426f },{ 0.94700f, 0.95100f, 0.97400f }, 1.33f },
//	{ "Grape Juice",{ 0.00138f, 0.00000f, 0.00000f },{ 0.10404f, 0.23958f, 0.29325f },{ 0.96100f, 0.00000f, 0.00000f }, 1.33f },
//	{ "Ruby Grapefruit Juice",{ 0.15496f, 0.15391f, 0.15995f },{ 0.08587f, 0.18314f, 0.25262f },{ 0.92900f, 0.92900f, 0.93100f }, 1.33f },
//	{ "White Grapefruit Juice",{ 0.50499f, 0.52742f, 0.75282f },{ 0.01380f, 0.01883f, 0.05678f },{ 0.54800f, 0.54500f, 0.56500f }, 1.33f },
//	{ "Shampoo",{ 0.00797f, 0.00874f, 0.01127f },{ 0.01411f, 0.04569f, 0.06172f },{ 0.91000f, 0.90500f, 0.92000f }, 1.33f },
//	{ "Strawberry Shampoo",{ 0.00215f, 0.00245f, 0.00253f },{ 0.01449f, 0.05796f, 0.07582f },{ 0.92700f, 0.93500f, 0.99400f }, 1.33f },
//	{ "Head & Shoulders Shampoo",{ 0.26747f, 0.27696f, 0.29574f },{ 0.08462f, 0.15688f, 0.20365f },{ 0.91100f, 0.89600f, 0.88400f }, 1.33f },
//	{ "Lemon Tea Powder",{ 0.74489f, 0.83823f, 1.00158f },{ 2.42881f, 4.57573f, 7.21270f },{ 0.94600f, 0.94600f, 0.94900f }, 1.33f },
//	{ "Orange Juice Powder",{ 0.00193f, 0.00213f, 0.00226f },{ 0.00145f, 0.00344f, 0.00786f },{ 0.91900f, 0.91800f, 0.92200f }, 1.33f },
//	{ "Pink Lemonade Powder",{ 0.00123f, 0.00133f, 0.00131f },{ 0.00116f, 0.00237f, 0.00320f },{ 0.90200f, 0.90200f, 0.90400f }, 1.33f },
//	{ "Cappuccino Powder",{ 12.2094f, 16.4659f, 29.2727f },{ 35.8441f, 49.5470f, 61.0844f },{ 0.84900f, 0.84300f, 0.92600f }, 1.33f },
//	{ "Salt Powder",{ 0.13805f, 0.15677f, 0.17865f },{ 0.28415f, 0.32570f, 0.34148f },{ 0.80200f, 0.79300f, 0.82100f }, 1.33f },
//	{ "Sugar Powder",{ 0.00282f, 0.00315f, 0.00393f },{ 0.01264f, 0.03105f, 0.05012f },{ 0.92100f, 0.91900f, 0.93100f }, 1.33f },
//	{ "Suisse Mocha Powder",{ 30.0848f, 33.4452f, 38.7191f },{ 17.5020f, 27.0044f, 35.4334f },{ 0.90700f, 0.89400f, 0.88800f }, 1.33f },
//	{ "Pacific Ocean Surface Water",{ 0.00180f, 0.00183f, 0.00228f },{ 0.03184f, 0.03132f, 0.03015f },{ 0.90200f, 0.82500f, 0.91400f }, 1.33f },
//
//	{ "",{ 0.0f,  0.0f,   0.0f },{ 0.0f,   0.0f,   0.0f },{ 0.0f,   0.0f,  0.0f }, 0.0f }
//};