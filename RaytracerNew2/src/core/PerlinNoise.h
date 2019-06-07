#pragma once

#include "core/Geometry.h"

class PerlinNoise
{
public:
	PerlinNoise(real persistence=0.f, int numberOfOctaves=0.f, real testScale=1);
	~PerlinNoise();

	//real eval(const Point2d& uv);

	//real eval(real x, real y);

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

	real eval(real x, real y);//PerlinNoise_2D(real x, real y);
	real InterpolatedNoise_1(real x, real y);
	real SmoothedNoise_1(int x, int y);
	real Noise1(int x, int y);

protected:
	int p[512];

	real myPersistence;
	int myNumberOfOctaves;
	real testScale;
};

