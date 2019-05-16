#include "PerlinNoiseTexture.h"

#include "ObjectFactoryManager.h"

static int permutation[] = { 151,160,137,91,90,15,
131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

PerlinNoiseTexture::PerlinNoiseTexture(const Parameters& params)
{
	for (int i = 0; i < 256; i++) {
		p[256 + i] = p[i] = permutation[i];
	}

	myColor1 = Color::fromRGB(0.4f, 0.2f, 0.8f);
	myColor2 = Color::fromRGB(0.1f, 0.7f, 0.3f);


	persistence = 0.5f;
	NumberOfOctaves = 4;
}


PerlinNoiseTexture::~PerlinNoiseTexture()
{
}

Color PerlinNoiseTexture::eval(const Point2d & uv)
{
	//real noiseCoef = 0;
	//real fac = 0.05;
	//for (int level = 1; level < 10; level++)
	//{
	//	noiseCoef += (1.0f / level)
	//		* fabsf(float(noise(level * 0.05 * uv.x(),
	//			level * 0.05 * uv.y(),
	//			level * 0.05 * 0.4)));//((int)(uv.x() * 256) & 21 + (int)(uv.y() * 256) & 21))));
	//}
	//	
	//return noiseCoef * myColor1 + (1. - noiseCoef) * myColor2;

	real coeff = PerlinNoise_2D(uv.x() * 100.f, uv.y() * 100.f);
	return coeff * myColor1 + (1.f - coeff) * myColor2;
}

real PerlinNoiseTexture::Noise1(int x, int y) //int
{
	int n = x + y * 57;
	n = (n << 13) ^ n;
	return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

real PerlinNoiseTexture::SmoothedNoise_1(int x, int y) {
	real corners = (Noise1(x - 1, y - 1) + Noise1(x + 1, y - 1) + Noise1(x - 1, y + 1) + Noise1(x + 1, y + 1)) / 16;
	real sides = (Noise1(x - 1, y) + Noise1(x + 1, y) + Noise1(x, y - 1) + Noise1(x, y + 1)) / 8;
	real center = Noise1(x, y) / 4;
	return corners + sides + center;
}

real PerlinNoiseTexture::InterpolatedNoise_1(real x, real y)
{
	int integer_X = int(x);
	real fractional_X = x - integer_X;

	int integer_Y = int(y);
	real fractional_Y = y - integer_Y;

	real v1 = SmoothedNoise_1(integer_X, integer_Y);
	real v2 = SmoothedNoise_1(integer_X + 1, integer_Y);
	real v3 = SmoothedNoise_1(integer_X, integer_Y + 1);
	real v4 = SmoothedNoise_1(integer_X + 1, integer_Y + 1);

	real i1 = lerp(fractional_X, v1, v2);
	real i2 = lerp(fractional_X, v3, v4);

	return lerp(fractional_Y, i1, i2);
}


real PerlinNoiseTexture::PerlinNoise_2D(real x, real y)
{
	real total = 0;
	int n = NumberOfOctaves - 1;

	for (unsigned int i = 0; i < (unsigned int) n; i++)
	{
		real frequency = (real)std::pow(2, i);
		real amplitude = (real)std::pow(persistence, i);

		total = total + InterpolatedNoise_1(x * frequency, y * frequency) * amplitude;
	}

	return total;
}

RT_REGISTER_TYPE(PerlinNoiseTexture, Texture)



//public final class ImprovedNoise {
//	static public real noise(real x, real y, real z) {
//		int X = (int)Math.floor(x) & 255,                  // FIND UNIT CUBE THAT
//			Y = (int)Math.floor(y) & 255,                  // CONTAINS POINT.
//			Z = (int)Math.floor(z) & 255;
//		x -= Math.floor(x);                                // FIND RELATIVE X,Y,Z
//		y -= Math.floor(y);                                // OF POINT IN CUBE.
//		z -= Math.floor(z);
//		real u = fade(x),                                // COMPUTE FADE CURVES
//			v = fade(y),                                // FOR EACH OF X,Y,Z.
//			w = fade(z);
//		int A = p[X] + Y, AA = p[A] + Z, AB = p[A + 1] + Z,      // HASH COORDINATES OF
//			B = p[X + 1] + Y, BA = p[B] + Z, BB = p[B + 1] + Z;      // THE 8 CUBE CORNERS,
//
//		return lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z),  // AND ADD
//			grad(p[BA], x - 1, y, z)), // BLENDED
//			lerp(u, grad(p[AB], x, y - 1, z),  // RESULTS
//				grad(p[BB], x - 1, y - 1, z))),// FROM  8
//			lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1),  // CORNERS
//				grad(p[BA + 1], x - 1, y, z - 1)), // OF CUBE
//				lerp(u, grad(p[AB + 1], x, y - 1, z - 1),
//					grad(p[BB + 1], x - 1, y - 1, z - 1))));
//	}
//	static real fade(real t) { return t * t * t * (t * (t * 6 - 15) + 10); }
//	static real lerp(real t, real a, real b) { return a + t * (b - a); }
//	static real grad(int hash, real x, real y, real z) {
//		int h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
//		real u = h<8 ? x : y,                 // INTO 12 GRADIENT DIRECTIONS.
//			v = h<4 ? y : h == 12 || h == 14 ? x : z;
//		return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
//	}
//	static final int p[] = new int[512], permutation[] = { 151,160,137,91,90,15,
//		131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
//		190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
//		88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
//		77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
//		102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
//		135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
//		5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
//		223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
//		129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
//		251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
//		49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
//		138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
//	};
//	static { for (int i = 0; i < 256; i++) p[256 + i] = p[i] = permutation[i]; }
//}