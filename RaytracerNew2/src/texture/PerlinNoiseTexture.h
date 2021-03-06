#pragma once
#include "Texture.h"
#include "core/Color.h"
#include "core/PerlinNoise.h"

#include <map>
#include <string>

class Parameters;

//Macro to associate strings and enum
#define MYLIST(x)       \
x(NORMAL_MAP, "normalMap") \
x(BLEND, "blend") \
x(END, "null") \

#define USE_FIRST_ELEMENT(x, y)  x,
#define USE_SECOND_ELEMENT(x, y) y,

class PerlinNoiseTexture :
	public Texture
{
public:
	enum TextureType
	{
		MYLIST(USE_FIRST_ELEMENT)
	};
public:
	PerlinNoiseTexture(const Parameters& params);
	~PerlinNoiseTexture();

	Color eval(const Point2d& uv) override;

	real noise(real x, real y, real z) {
		int X = (int)floor(x) & 255,                  // FIND UNIT CUBE THAT
			Y = (int)floor(y) & 255,                  // CONTAINS POINT.
			Z = (int)floor(z) & 255;
		x -= floor(x);                                // FIND RELATIVE X,Y,Z
		y -= floor(y);                                // OF POINT IN CUBE.
		z -= floor(z);
		real u = fade(x),                                // COMPUTE FADE CURVES
			v = fade(y),                                // FOR EACH OF X,Y,Z.
			w = fade(z);
		int A = p[X] + Y, AA = p[A] + Z, AB = p[A + 1] + Z,      // HASH COORDINATES OF
			B = p[X + 1] + Y, BA = p[B] + Z, BB = p[B + 1] + Z;      // THE 8 CUBE CORNERS,

		return lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z),  // AND ADD
			grad(p[BA], x - 1, y, z)), // BLENDED
			lerp(u, grad(p[AB], x, y - 1, z),  // RESULTS
				grad(p[BB], x - 1, y - 1, z))),// FROM  8
			lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1),  // CORNERS
				grad(p[BA + 1], x - 1, y, z - 1)), // OF CUBE
				lerp(u, grad(p[AB + 1], x, y - 1, z - 1),
					grad(p[BB + 1], x - 1, y - 1, z - 1))));
	}
	real fade(real t) { return t * t * t * (t * (t * 6 - 15) + 10); }
	real lerp(real t, real a, real b) { return a + t * (b - a); }
	real grad(int hash, real x, real y, real z) {
		int h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
		real u = h<8 ? x : y,                 // INTO 12 GRADIENT DIRECTIONS.
			v = h<4 ? y : h == 12 || h == 14 ? x : z;
		return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
	}

	Color getAverage() const override { throw std::runtime_error("not implemented yet"); }
	Color getMin() const override { throw std::runtime_error("not implemented yet"); }
	Color getMax() const override { throw std::runtime_error("not implemented yet"); }


	real PerlinNoise_2D(real x, real y);
	real InterpolatedNoise_1(real x, real y);
	real SmoothedNoise_1(int x, int y);
	real Noise1(int x, int y);

	int p[512];
	Color myColor1;
	Color myColor2;

	PerlinNoise myNoise;


	PerlinNoise noise2;

	//Array2D<Color> myArray;

	std::map<std::string, TextureType> myTypeByName;
	TextureType myType;
	//real myPersistence;
	//int myNumberOfOctaves;
	//real testScale;
};

namespace rtPriv
{
	static const char *STRING[] =
	{
		MYLIST(USE_SECOND_ELEMENT)
	};
};

#undef MYLIST
#undef USE_FIRST_ELEMENT
#undef USE_SECOND_ELEMENT