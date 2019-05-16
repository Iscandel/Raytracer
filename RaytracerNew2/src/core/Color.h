#ifndef H__COLOR_170520151941__H
#define H__COLOR_170520151941__H

#include "core/BasicMath.h"
//#include <algorithm>
#include <cmath>
//#include <limits.h>
//#include <iostream>
#include <vector>

#include "core/Geometry.h"
#include "tools/Logger.h"

//#define SPECTRAL
#ifdef SPECTRAL
#ifndef NB_SPECTRUM_SAMPLES
	#define NB_SPECTRUM_SAMPLES 15
#endif
#else
#define NB_SPECTRUM_SAMPLES 3
#endif 

#ifdef USE_ALIGN
#define ALIGNED_SAMPLES NB_SPECTRUM_SAMPLES + 1
#else
#define ALIGNED_SAMPLES NB_SPECTRUM_SAMPLES;
#endif

//using ray::real;

//class DiscreteSpectrum
//{
//public:
//	static constexpr int NB_SAMPLES = NB_SPECTRUM_SAMPLES;
//};


template<class T>
class TplColor : /*public DiscreteSpectrum,*/ public Eigen::Array<T, ALIGNED_SAMPLES, 1>
{
public:
	typedef Eigen::Array<T, ALIGNED_SAMPLES, 1> BaseArray;

public:
	static constexpr int NB_SAMPLES = NB_SPECTRUM_SAMPLES;

public:
	static void init()
	{
#if NB_SPECTRUM_SAMPLES != 3
		for (int i = 0; i < NB_SAMPLES; i++)
		{
			priv::real lambda0 = math::interp1(priv::real(i) / priv::real(NB_SAMPLES),
				(T)CIEConstants::LAMBDA_START, (T)CIEConstants::LAMBDA_END);
			priv::real lambda1 = math::interp1(priv::real(i + 1) / priv::real(NB_SAMPLES),
				(T)CIEConstants::LAMBDA_START, (T)CIEConstants::LAMBDA_END);
			X(i) = averageSpectrumSamples(CIEConstants::CIE_lambda,
				CIEConstants::CIE_X, (int)CIEConstants::CIE_SAMPLE_NUMBER, lambda0, lambda1);
			Y(i) = averageSpectrumSamples(CIEConstants::CIE_lambda,
				CIEConstants::CIE_Y, (int)CIEConstants::CIE_SAMPLE_NUMBER, lambda0, lambda1);
			Z(i) = averageSpectrumSamples(CIEConstants::CIE_lambda,
				CIEConstants::CIE_Z, (int)CIEConstants::CIE_SAMPLE_NUMBER, lambda0, lambda1);
		}
#endif
	}

public:
	explicit TplColor(T val = 0.)
		//:r(coeffRef(0))
		//, g(coeffRef(1))
		//, b(coeffRef(2))
	{
		BaseArray::setConstant(val);
	}

	explicit TplColor(T spectrum[NB_SPECTRUM_SAMPLES])
		:BaseArray(spectrum)
	{		
	}

#if NB_SPECTRUM_SAMPLES == 3
#ifdef USE_ALIGN

	TplColor(T x, T y, T z, T w = (T)0)
		: BaseArray(x, y, z, w)
		//, r(coeffRef(0))
		//, g(coeffRef(1))
		//, b(coeffRef(2))
	{
	}

#else
	TplColor(T x, T y, T z)
		: Eigen::Array<T, ALIGNED_SAMPLES, 1>(x, y, z)
		, r(coeffRef(0))
		, g(coeffRef(1))
		, b(coeffRef(2))
	{
	}
#endif
#endif

	//TplColor(const Color3<T>& c)
	//	:r(coeffRef(0))
	//	, g(coeffRef(1))
	//	, b(coeffRef(2))
	//{
	//	r = c.r;
	//	g = c.g;
	//	b = c.b;
	//}

	TplColor& operator = (const TplColor<T>& c) {
		for (int i = 0; i < NB_SAMPLES; i++)
			coeffRef(i) = c.coeff(i);

		return *this;
	}

	template <typename Derived> TplColor(const Eigen::ArrayBase<Derived>& p)
		: BaseArray(p)
		//, r(coeffRef(0))
		//, g(coeffRef(1))
		//, b(coeffRef(2))
	{ }

	template <typename Derived> TplColor &operator=(const Eigen::ArrayBase<Derived>& p) {
		this->Base::operator=(p);
		return *this;
	}

	static TplColor<T> fromBlackbody(priv::real temperature)
	{
		/* Convert inputs to meters and kelvins */
		const double c = 299792458;      /* Speed of light */
		const double k = 1.3806488e-23;  /* Boltzmann constant */
		const double h = 6.62606957e-34; /* Planck constant */

		const int SAMPLE_NUMBER = CIEConstants::SAMPLE_NUMBER;
		const int LAMBDA_START = CIEConstants::LAMBDA_START;
		const int LAMBDA_END = CIEConstants::LAMBDA_END;

		std::vector<priv::real> lambdaVect;
		std::vector<priv::real> spectrum;

		for (int i = 0; i < SAMPLE_NUMBER; i++)
		{
			priv::real l = math::interp1(priv::real(i) / priv::real(SAMPLE_NUMBER),
				LAMBDA_START, LAMBDA_END);
			const double lambda = l * 1e-9;  /* Wavelength in meters */
											 /* Watts per unit surface area (m^-2) per unit wavelength (nm^-1) per
											 steradian (sr^-1) */

			const double I = (2 * h*c*c) * std::pow(lambda, -5.0)
				/ ((std::exp((h / k)*c / (lambda*temperature)) - 1.0) * 1e9);
			lambdaVect.push_back(l);
			spectrum.push_back(I);
		}

		TplColor<T> res;
		res.setFromSampledSpectrum(lambdaVect, spectrum);
		return res;

		//CIEConstants convert(lambdaVect, spectrum);
		//priv::real rgb[3];
		//convert.toRGB(rgb);
		//return TplColor<T>(rgb[0], rgb[1], rgb[2]);
	}

	priv::real average() const
	{
		T sum = (T)0;
		for (int i = 0; i < NB_SAMPLES; i++)
			sum += coeff(i);

		return sum / (priv::real)NB_SAMPLES;//coeff(0) + coeff(1) + coeff(2)) / (T)3.;
	}

	bool isZero() const
	{
#ifdef DOUBLE_PRECISION
		const priv::real eps = 1e-9;
#else
		const priv::real eps = 1e-5f;
#endif
		return BaseArray::isZero(eps);
		//for (int i = 0; i < NB_SAMPLES; i++)
		//	if (coeff(i) > eps)
		//		return false;
		//return true;
	}

	bool isNan() const
	{
		//return hasNaN();
		for (int i = 0; i < NB_SAMPLES; i++)// : *this)
			if (std::isnan(coeff(i)))
				return true;
		return false;
		//if (std::isnan(coeff(0)) || std::isnan(coeff(1)) || std::isnan(coeff(2)))
		//	return true;
		//return false;
	}

	void XYZToRGB(const priv::real _xyz[3], priv::real rgb[3], bool clamp = true) const {
		rgb[0] = 3.240479f * _xyz[0] - 1.537150f * _xyz[1] - 0.498535f * _xyz[2];
		rgb[1] = -0.969256f * _xyz[0] + 1.875991f * _xyz[1] + 0.041556f * _xyz[2];
		rgb[2] = 0.055648f * _xyz[0] - 0.204043f * _xyz[1] + 1.057311f * _xyz[2];
		if (clamp) {
			rgb[0] = math::thresholding(rgb[0], (priv::real)0, rgb[0]);
			rgb[1] = math::thresholding(rgb[1], (priv::real)0, rgb[1]);
			rgb[2] = math::thresholding(rgb[2], (priv::real)0, rgb[2]);
		}
	}

	void RGBToXYZ(const priv::real rgb[3], priv::real _xyz[3]) const {

		xyz[0] = 0.412453f*rgb[0] + 0.357580f*rgb[1] + 0.180423f*rgb[2];
		xyz[1] = 0.212671f*rgb[0] + 0.715160f*rgb[1] + 0.072169f*rgb[2];
		xyz[2] = 0.019334f*rgb[0] + 0.119193f*rgb[1] + 0.950227f*rgb[2];
	}

	template <typename Predicate>
	int findInterval(int size, const Predicate &pred) {
		int first = 0, len = size;
		while (len > 0) {
			int half = len >> 1, middle = first + half;
			// Bisect range based on value of _pred_ at _middle_
			if (pred(middle)) {
				first = middle + 1;
				len -= half + 1;
			}
			else
				len = half;
		}
		return math::thresholding(first - 1, 0, size - 2);
	}

	priv::real interpolateSpectrumSamples(const priv::real *lambda, const priv::real *vals, int n,
		priv::real l) {
		//for (int i = 0; i < n - 1; ++i) CHECK_GT(lambda[i + 1], lambda[i]);
		if (l <= lambda[0]) return vals[0];
		if (l >= lambda[n - 1]) return vals[n - 1];
		int offset = findInterval(n, [&](int index) { return lambda[index] <= l; });
		//CHECK(l >= lambda[offset] && l <= lambda[offset + 1]);
		priv::real t = (l - lambda[offset]) / (lambda[offset + 1] - lambda[offset]);
		return math::interp1(t, vals[offset], vals[offset + 1]);
	}

	void clampNeg()
	{
		for (int i = 0; i < NB_SAMPLES; i++)
		{
			coeffRef(i) = std::max((T)0, coeff(i));
		}
	}

#if NB_SPECTRUM_SAMPLES == 3
	T luminance() const
	{
		return 0.2126f * coeff(0) + 0.7152f * coeff(1) + 0.0722f *coeff(2);
	}

	T max()const
	{
		return math::max(coeff(0), coeff(1), coeff(2));
		//return std::max(coeff(0), std::max(coeff(1), coeff(2)));
	}

	void toRGB(T& r, T& g, T& b) const
	{
		r = coeff(0);
		g = coeff(1);
		b = coeff(2);
	}

	void toSRGB(T& r, T& g, T& b) const
	{
		auto toSRGBVal = [&](T value) {
			if (value < 0.0031308)
				return 12.92f * value;
			return 1.055f * std::pow(value, 0.41666f) - 0.055f;
		};

		r = toSRGBVal(coeff(0));
		g = toSRGBVal(coeff(1));
		b = toSRGBVal(coeff(2));
	}

	static TplColor<T> fromRGB(T r, T g, T b)
	{
		return TplColor<T>(r, g, b);
	}

	void setfromRGB(T r, T g, T b)
	{
		coeffRef(0) = r;
		coeffRef(1) = g;
		coeffRef(2) = b;
	}

	void toXYZ(priv::real _xyz[3]) const {
		RGBToXYZ(data(), xyz);
	}



	void setFromSampledSpectrum(const std::vector<priv::real>& lambda, const std::vector<priv::real>& spectrum)
	{
		priv::real xyz[3] = { 0, 0, 0 };
		for (int i = 0; i < CIEConstants::CIE_SAMPLE_NUMBER; ++i) {
			priv::real val = interpolateSpectrumSamples(lambda.data(), spectrum.data(), spectrum.size(), CIEConstants::CIE_lambda[i]);
			xyz[0] += val * CIEConstants::CIE_X[i];
			xyz[1] += val * CIEConstants::CIE_Y[i];
			xyz[2] += val * CIEConstants::CIE_Z[i];
		}
		priv::real scale = priv::real(CIEConstants::CIE_lambda[CIEConstants::CIE_SAMPLE_NUMBER - 1] - CIEConstants::CIE_lambda[0]) /
			(CIEConstants::CIE_Y_INTEGRAL * CIEConstants::CIE_SAMPLE_NUMBER);
		xyz[0] *= scale;
		xyz[1] *= scale;
		xyz[2] *= scale;
		priv::real rgb[3];
		XYZToRGB(xyz, rgb);
		coeffRef(0) = rgb[0];
		coeffRef(1) = rgb[1];
		coeffRef(2) = rgb[2];

		//ColorSpaceConversion convert(lambda, spectrum);
		//T rgb[3];
		//convert.toRGB(rgb);
		//coeffRef(0) = rgb[0];
		//coeffRef(1) = rgb[1];
		//coeffRef(2) = rgb[2];
	}
#else
	void toXYZ(priv::real _xyz[3]) const {
		_xyz[0] = _xyz[1] = _xyz[2] = 0.f;
		for (int i = 0; i < NB_SAMPLES; ++i) {
			_xyz[0] += X(i) * coeff(i);
			_xyz[1] += Y(i) * coeff(i);
			_xyz[2] += Z(i) * coeff(i);
		}
		priv::real scale = priv::real(CIEConstants::LAMBDA_END - CIEConstants::LAMBDA_START) /
			priv::real(CIEConstants::CIE_Y_INTEGRAL * NB_SAMPLES);
		_xyz[0] *= scale;
		_xyz[1] *= scale;
		_xyz[2] *= scale;
	}

	void toRGB(T& r, T& g, T& b) const
	{
		priv::real xyz[3];
		toXYZ(xyz);
		
		priv::real rgb[3];
		XYZToRGB(xyz, rgb);
		r = rgb[0];
		g = rgb[1];
		b = rgb[2];
	}

	void toSRGB(T& r, T& g, T& b) const
	{
		T tmpR, tmpG, tmpB;
		toRGB(tmpR, tmpG, tmpB);

		auto toSRGBVal = [&](T value) {
			if (value < 0.0031308)
				return 12.92f * value;
			return 1.055f * std::pow(value, 0.41666f) - 0.055f;
		};

		r = toSRGBVal(tmpR);
		g = toSRGBVal(tmpG);
		b = toSRGBVal(tmpB);
	}

	static TplColor<T> fromRGB(T r, T g, T b)
	{
		TplColor<T> col;
		col.setFromRGB(r, g, b);
		return col;
	}

	void setFromRGB(T _r, T _g, T _b)
	{
		T rgb[3];
		rgb[0] = _r; rgb[1] = _g; rgb[2] = _b;
		TplColor<T> r;
		if (true) {//type == SPECTRUM_REFLECTANCE) {////////////////////////////////////////////////////////////////////////////////////
			// Convert reflectance spectrum to RGB
			if (rgb[0] <= rgb[1] && rgb[0] <= rgb[2]) {
				// Compute reflectance _SampledSpectrum_ with _rgb[0]_ as minimum
				r += rgb[0] * smitSpectrum::rgbRefl2SpectWhite;
				if (rgb[1] <= rgb[2]) {
					r += (rgb[1] - rgb[0]) * smitSpectrum::rgbRefl2SpectCyan;
					r += (rgb[2] - rgb[1]) * smitSpectrum::rgbRefl2SpectBlue;
				}
				else {
					r += (rgb[2] - rgb[0]) * smitSpectrum::rgbRefl2SpectCyan;
					r += (rgb[1] - rgb[2]) * smitSpectrum::rgbRefl2SpectGreen;
				}
			}
			else if (rgb[1] <= rgb[0] && rgb[1] <= rgb[2]) {
				// Compute reflectance _SampledSpectrum_ with _rgb[1]_ as minimum
				r += rgb[1] * smitSpectrum::rgbRefl2SpectWhite;
				if (rgb[0] <= rgb[2]) {
					r += (rgb[0] - rgb[1]) * smitSpectrum::rgbRefl2SpectMagenta;
					r += (rgb[2] - rgb[0]) * smitSpectrum::rgbRefl2SpectBlue;
				}
				else {
					r += (rgb[2] - rgb[1]) * smitSpectrum::rgbRefl2SpectMagenta;
					r += (rgb[0] - rgb[2]) * smitSpectrum::rgbRefl2SpectRed;
			}
		}
			else {
				// Compute reflectance _SampledSpectrum_ with _rgb[2]_ as minimum
				r += rgb[2] * smitSpectrum::rgbRefl2SpectWhite;
				if (rgb[0] <= rgb[1]) {
					r += (rgb[0] - rgb[2]) * smitSpectrum::rgbRefl2SpectYellow;
					r += (rgb[1] - rgb[0]) * smitSpectrum::rgbRefl2SpectGreen;
				}
				else {
					r += (rgb[1] - rgb[2]) * smitSpectrum::rgbRefl2SpectYellow;
					r += (rgb[0] - rgb[1]) * smitSpectrum::rgbRefl2SpectRed;
				}
			}
			r *= .94;
	}
		else {
			// Convert illuminant spectrum to RGB
			if (rgb[0] <= rgb[1] && rgb[0] <= rgb[2]) {
				// Compute illuminant _SampledSpectrum_ with _rgb[0]_ as minimum
				r += rgb[0] * smitSpectrum::rgbIllum2SpectWhite;
				if (rgb[1] <= rgb[2]) {
					r += (rgb[1] - rgb[0]) * smitSpectrum::rgbIllum2SpectCyan;
					r += (rgb[2] - rgb[1]) * smitSpectrum::rgbIllum2SpectBlue;
				}
				else {
					r += (rgb[2] - rgb[0]) * smitSpectrum::rgbIllum2SpectCyan;
					r += (rgb[1] - rgb[2]) * smitSpectrum::rgbIllum2SpectGreen;
				}
			}
			else if (rgb[1] <= rgb[0] && rgb[1] <= rgb[2]) {
				// Compute illuminant _SampledSpectrum_ with _rgb[1]_ as minimum
				r += rgb[1] * smitSpectrum::rgbIllum2SpectWhite;
				if (rgb[0] <= rgb[2]) {
					r += (rgb[0] - rgb[1]) * smitSpectrum::rgbIllum2SpectMagenta;
					r += (rgb[2] - rgb[0]) * smitSpectrum::rgbIllum2SpectBlue;
				}
				else {
					r += (rgb[2] - rgb[1]) * smitSpectrum::rgbIllum2SpectMagenta;
					r += (rgb[0] - rgb[2]) * smitSpectrum::rgbIllum2SpectRed;
				}
			}
			else {
				// Compute illuminant _SampledSpectrum_ with _rgb[2]_ as minimum
				r += rgb[2] * smitSpectrum::rgbIllum2SpectWhite;
				if (rgb[0] <= rgb[1]) {
					r += (rgb[0] - rgb[2]) * smitSpectrum::rgbIllum2SpectYellow;
					r += (rgb[1] - rgb[0]) * smitSpectrum::rgbIllum2SpectGreen;
				}
				else {
					r += (rgb[1] - rgb[2]) * smitSpectrum::rgbIllum2SpectYellow;
					r += (rgb[0] - rgb[1]) * smitSpectrum::rgbIllum2SpectRed;
				}
			}
			r *= .86445f;
		}

		r.clampNeg();
		*this = r;
	}

	T luminance() const
	{
		T res = (T)0.f;
		for (int i = 0; i < NB_SAMPLES; i++)
			res += Y(i) * coeff(i);
	
		return res * (CIEConstants::LAMBDA_END - CIEConstants::LAMBDA_START) /
					  T(CIEConstants::CIE_Y_INTEGRAL * NB_SAMPLES);
	}

	T max() const
	{
		return maxCoeff();
	}

	static TplColor<T> fromSampledSpectrum(const priv::real* lambda, const priv::real* spectrum, int nbSamples)
	{
		Color res;
		for (int i = 0; i < NB_SAMPLES; i++)
		{
			priv::real lambda0 = math::interp1(priv::real(i) / priv::real(NB_SAMPLES),
				(T)CIEConstants::LAMBDA_START, (T) CIEConstants::LAMBDA_END);
			priv::real lambda1 = math::interp1(priv::real(i + 1) / priv::real(NB_SAMPLES),
				(T) CIEConstants::LAMBDA_START, (T) CIEConstants::LAMBDA_END);
			T value = averageSpectrumSamples(lambda, spectrum, nbSamples, lambda0, lambda1);
			res(i) = (T)value;
		}

		return res;
	}

	void setFromSampledSpectrum(const std::vector<priv::real>& lambda, const std::vector<priv::real>& spectrum)
	{
		for (int i = 0; i < NB_SAMPLES; i++)
		{
			priv::real lambda0 = math::interp1(priv::real(i) / priv::real(NB_SAMPLES),
				CIEConstants::LAMBDA_START, CIEConstants::LAMBDA_END);
			priv::real lambda1 = math::interp1(priv::real(i + 1) / priv::real(NB_SAMPLES),
				CIEConstants::LAMBDA_START, CIEConstants::LAMBDA_END);
			T value = averageSpectrumSamples(lambda.data(), spectrum.data(), (int)spectrum.size(), lambda0, lambda1);
			coeffRef(i) = (T)value;
		}
	}
#endif

	static priv::real averageSpectrumSamples(const priv::real *lambda, const priv::real *vals,
		int n, priv::real lambdaStart, priv::real lambdaEnd) {
		//for (int i = 0; i < n - 1; ++i) assert(lambda[i + 1] > lambda[i]);
		//assert(lambdaStart < lambdaEnd);
		// Handle cases with out-of-bounds range or single sample only
		if (lambdaEnd <= lambda[0])   return vals[0];
		if (lambdaStart >= lambda[n - 1]) return vals[n - 1];
		if (n == 1) return vals[0];
		priv::real sum = 0.f;
		// Add contributions of constant segments before/after samples
		if (lambdaStart < lambda[0])
			sum += vals[0] * (lambda[0] - lambdaStart);
		if (lambdaEnd > lambda[n - 1])
			sum += vals[n - 1] * (lambdaEnd - lambda[n - 1]);

		// Advance to first relevant wavelength segment
		int i = 0;
		while (lambdaStart > lambda[i + 1]) ++i;
		//assert(i + 1 < n);

		// Loop over wavelength sample segments and add contributions
#define INTERP(w, i) \
        math::interp1(((w) - lambda[i]) / (lambda[(i)+1] - lambda[i]), \
             vals[i], vals[(i)+1])
#define SEG_AVG(wl0, wl1, i) (0.5f * (INTERP(wl0, i) + INTERP(wl1, i)))
		for (; i + 1 < n && lambdaEnd >= lambda[i]; ++i) {
			priv::real segStart = std::max(lambdaStart, lambda[i]);
			priv::real segEnd = std::min(lambdaEnd, lambda[i + 1]);
			sum += SEG_AVG(segStart, segEnd, i) * (segEnd - segStart);
		}
#undef INTERP
#undef SEG_AVG
		return sum / (lambdaEnd - lambdaStart);
	}

	
#if NB_SPECTRUM_SAMPLES == 3
	T r() const {
		return coeff(0);
	}

	T& r() {
		return coeffRef(0);
	}

	T g() const {
		return coeff(1);
	}

	T& g() {
		return coeffRef(1);
	}

	T b() const {
		return coeff(2);
	}

	T& b() {
		return coeffRef(2);
	}
#else
protected:
	static TplColor<T> X, Y, Z;
#endif

};

#if NB_SPECTRUM_SAMPLES != 3
template<class T>
TplColor<T> TplColor<T>::X;
template<class T>
TplColor<T> TplColor<T>::Y;
template<class T>
TplColor<T> TplColor<T>::Z;
#endif

#ifdef _MSC_VER
// Disable the double to float truncation warning
# pragma warning( disable : 4305)
# pragma warning( disable : 4838 )
#endif

template<class T>
inline std::ostream& operator << (std::ostream& o, const TplColor<T>& color)
{
	o << "[";
	for (int i = 0; i < 3; i++)
	{
		o << color(i);
		if (i != 2)
			o << " ";
	}
	o << "]" << std::endl;

	return o;
}

typedef TplColor<real> Color;
typedef TplColor<real> Color3f;

#if NB_SPECTRUM_SAMPLES != 3
namespace smitSpectrum
{
	const int NB_RGB_2_SPECT_SAMPLES = 32;

		const real RGB2SpectLambda[NB_RGB_2_SPECT_SAMPLES] = {
			380.000000, 390.967743, 401.935486, 412.903229, 423.870972, 434.838715,
			445.806458, 456.774200, 467.741943, 478.709686, 489.677429, 500.645172,
			511.612915, 522.580627, 533.548340, 544.516052, 555.483765, 566.451477,
			577.419189, 588.386902, 599.354614, 610.322327, 621.290039, 632.257751,
			643.225464, 654.193176, 665.160889, 676.128601, 687.096313, 698.064026,
			709.031738, 720.000000
		};



		const real RGBRefl2SpectWhite[NB_RGB_2_SPECT_SAMPLES] = {
			1.0618958571272863e+00,   1.0615019980348779e+00,
			1.0614335379927147e+00,   1.0622711654692485e+00,
			1.0622036218416742e+00,   1.0625059965187085e+00,
			1.0623938486985884e+00,   1.0624706448043137e+00,
			1.0625048144827762e+00,   1.0624366131308856e+00,
			1.0620694238892607e+00,   1.0613167586932164e+00,
			1.0610334029377020e+00,   1.0613868564828413e+00,
			1.0614215366116762e+00,   1.0620336151299086e+00,
			1.0625497454805051e+00,   1.0624317487992085e+00,
			1.0625249140554480e+00,   1.0624277664486914e+00,
			1.0624749854090769e+00,   1.0625538581025402e+00,
			1.0625326910104864e+00,   1.0623922312225325e+00,
			1.0623650980354129e+00,   1.0625256476715284e+00,
			1.0612277619533155e+00,   1.0594262608698046e+00,
			1.0599810758292072e+00,   1.0602547314449409e+00,
			1.0601263046243634e+00,   1.0606565756823634e+00 };

		const real RGBRefl2SpectCyan[NB_RGB_2_SPECT_SAMPLES] = {
			1.0414628021426751e+00,   1.0328661533771188e+00,
			1.0126146228964314e+00,   1.0350460524836209e+00,
			1.0078661447098567e+00,   1.0422280385081280e+00,
			1.0442596738499825e+00,   1.0535238290294409e+00,
			1.0180776226938120e+00,   1.0442729908727713e+00,
			1.0529362541920750e+00,   1.0537034271160244e+00,
			1.0533901869215969e+00,   1.0537782700979574e+00,
			1.0527093770467102e+00,   1.0530449040446797e+00,
			1.0550554640191208e+00,   1.0553673610724821e+00,
			1.0454306634683976e+00,   6.2348950639230805e-01,
			1.8038071613188977e-01,  -7.6303759201984539e-03,
			-1.5217847035781367e-04,  -7.5102257347258311e-03,
			-2.1708639328491472e-03,   6.5919466602369636e-04,
			1.2278815318539780e-02,  -4.4669775637208031e-03,
			1.7119799082865147e-02,   4.9211089759759801e-03,
			5.8762925143334985e-03,   2.5259399415550079e-02 };

		const real RGBRefl2SpectMagenta[NB_RGB_2_SPECT_SAMPLES] = {
			9.9422138151236850e-01,   9.8986937122975682e-01,
			9.8293658286116958e-01,   9.9627868399859310e-01,
			1.0198955019000133e+00,   1.0166395501210359e+00,
			1.0220913178757398e+00,   9.9651666040682441e-01,
			1.0097766178917882e+00,   1.0215422470827016e+00,
			6.4031953387790963e-01,   2.5012379477078184e-03,
			6.5339939555769944e-03,   2.8334080462675826e-03,
			-5.1209675389074505e-11,  -9.0592291646646381e-03,
			3.3936718323331200e-03,  -3.0638741121828406e-03,
			2.2203936168286292e-01,   6.3141140024811970e-01,
			9.7480985576500956e-01,   9.7209562333590571e-01,
			1.0173770302868150e+00,   9.9875194322734129e-01,
			9.4701725739602238e-01,   8.5258623154354796e-01,
			9.4897798581660842e-01,   9.4751876096521492e-01,
			9.9598944191059791e-01,   8.6301351503809076e-01,
			8.9150987853523145e-01,   8.4866492652845082e-01 };

		const real RGBRefl2SpectYellow[NB_RGB_2_SPECT_SAMPLES] = {
			5.5740622924920873e-03,  -4.7982831631446787e-03,
			-5.2536564298613798e-03,  -6.4571480044499710e-03,
			-5.9693514658007013e-03,  -2.1836716037686721e-03,
			1.6781120601055327e-02,   9.6096355429062641e-02,
			2.1217357081986446e-01,   3.6169133290685068e-01,
			5.3961011543232529e-01,   7.4408810492171507e-01,
			9.2209571148394054e-01,   1.0460304298411225e+00,
			1.0513824989063714e+00,   1.0511991822135085e+00,
			1.0510530911991052e+00,   1.0517397230360510e+00,
			1.0516043086790485e+00,   1.0511944032061460e+00,
			1.0511590325868068e+00,   1.0516612465483031e+00,
			1.0514038526836869e+00,   1.0515941029228475e+00,
			1.0511460436960840e+00,   1.0515123758830476e+00,
			1.0508871369510702e+00,   1.0508923708102380e+00,
			1.0477492815668303e+00,   1.0493272144017338e+00,
			1.0435963333422726e+00,   1.0392280772051465e+00 };

		const real RGBRefl2SpectRed[NB_RGB_2_SPECT_SAMPLES] = {
			1.6575604867086180e-01,   1.1846442802747797e-01,
			1.2408293329637447e-01,   1.1371272058349924e-01,
			7.8992434518899132e-02,   3.2205603593106549e-02,
			-1.0798365407877875e-02,   1.8051975516730392e-02,
			5.3407196598730527e-03,   1.3654918729501336e-02,
			-5.9564213545642841e-03,  -1.8444365067353252e-03,
			-1.0571884361529504e-02,  -2.9375521078000011e-03,
			-1.0790476271835936e-02,  -8.0224306697503633e-03,
			-2.2669167702495940e-03,   7.0200240494706634e-03,
			-8.1528469000299308e-03,   6.0772866969252792e-01,
			9.8831560865432400e-01,   9.9391691044078823e-01,
			1.0039338994753197e+00,   9.9234499861167125e-01,
			9.9926530858855522e-01,   1.0084621557617270e+00,
			9.8358296827441216e-01,   1.0085023660099048e+00,
			9.7451138326568698e-01,   9.8543269570059944e-01,
			9.3495763980962043e-01,   9.8713907792319400e-01 };

		const real RGBRefl2SpectGreen[NB_RGB_2_SPECT_SAMPLES] = {
			2.6494153587602255e-03,  -5.0175013429732242e-03,
			-1.2547236272489583e-02,  -9.4554964308388671e-03,
			-1.2526086181600525e-02,  -7.9170697760437767e-03,
			-7.9955735204175690e-03,  -9.3559433444469070e-03,
			6.5468611982999303e-02,   3.9572875517634137e-01,
			7.5244022299886659e-01,   9.6376478690218559e-01,
			9.9854433855162328e-01,   9.9992977025287921e-01,
			9.9939086751140449e-01,   9.9994372267071396e-01,
			9.9939121813418674e-01,   9.9911237310424483e-01,
			9.6019584878271580e-01,   6.3186279338432438e-01,
			2.5797401028763473e-01,   9.4014888527335638e-03,
			-3.0798345608649747e-03,  -4.5230367033685034e-03,
			-6.8933410388274038e-03,  -9.0352195539015398e-03,
			-8.5913667165340209e-03,  -8.3690869120289398e-03,
			-7.8685832338754313e-03,  -8.3657578711085132e-06,
			5.4301225442817177e-03,  -2.7745589759259194e-03 };

		const real RGBRefl2SpectBlue[NB_RGB_2_SPECT_SAMPLES] = {
			9.9209771469720676e-01,   9.8876426059369127e-01,
			9.9539040744505636e-01,   9.9529317353008218e-01,
			9.9181447411633950e-01,   1.0002584039673432e+00,
			9.9968478437342512e-01,   9.9988120766657174e-01,
			9.8504012146370434e-01,   7.9029849053031276e-01,
			5.6082198617463974e-01,   3.3133458513996528e-01,
			1.3692410840839175e-01,   1.8914906559664151e-02,
			-5.1129770932550889e-06,  -4.2395493167891873e-04,
			-4.1934593101534273e-04,   1.7473028136486615e-03,
			3.7999160177631316e-03,  -5.5101474906588642e-04,
			-4.3716662898480967e-05,   7.5874501748732798e-03,
			2.5795650780554021e-02,   3.8168376532500548e-02,
			4.9489586408030833e-02,   4.9595992290102905e-02,
			4.9814819505812249e-02,   3.9840911064978023e-02,
			3.0501024937233868e-02,   2.1243054765241080e-02,
			6.9596532104356399e-03,   4.1733649330980525e-03 };

		const real RGBIllum2SpectWhite[NB_RGB_2_SPECT_SAMPLES] = {
			1.1565232050369776e+00,   1.1567225000119139e+00,
			1.1566203150243823e+00,   1.1555782088080084e+00,
			1.1562175509215700e+00,   1.1567674012207332e+00,
			1.1568023194808630e+00,   1.1567677445485520e+00,
			1.1563563182952830e+00,   1.1567054702510189e+00,
			1.1565134139372772e+00,   1.1564336176499312e+00,
			1.1568023181530034e+00,   1.1473147688514642e+00,
			1.1339317140561065e+00,   1.1293876490671435e+00,
			1.1290515328639648e+00,   1.0504864823782283e+00,
			1.0459696042230884e+00,   9.9366687168595691e-01,
			9.5601669265393940e-01,   9.2467482033511805e-01,
			9.1499944702051761e-01,   8.9939467658453465e-01,
			8.9542520751331112e-01,   8.8870566693814745e-01,
			8.8222843814228114e-01,   8.7998311373826676e-01,
			8.7635244612244578e-01,   8.8000368331709111e-01,
			8.8065665428441120e-01,   8.8304706460276905e-01 };

		const real RGBIllum2SpectCyan[NB_RGB_2_SPECT_SAMPLES] = {
			1.1334479663682135e+00,   1.1266762330194116e+00,
			1.1346827504710164e+00,   1.1357395805744794e+00,
			1.1356371830149636e+00,   1.1361152989346193e+00,
			1.1362179057706772e+00,   1.1364819652587022e+00,
			1.1355107110714324e+00,   1.1364060941199556e+00,
			1.1360363621722465e+00,   1.1360122641141395e+00,
			1.1354266882467030e+00,   1.1363099407179136e+00,
			1.1355450412632506e+00,   1.1353732327376378e+00,
			1.1349496420726002e+00,   1.1111113947168556e+00,
			9.0598740429727143e-01,   6.1160780787465330e-01,
			2.9539752170999634e-01,   9.5954200671150097e-02,
			-1.1650792030826267e-02,  -1.2144633073395025e-02,
			-1.1148167569748318e-02,  -1.1997606668458151e-02,
			-5.0506855475394852e-03,  -7.9982745819542154e-03,
			-9.4722817708236418e-03,  -5.5329541006658815e-03,
			-4.5428914028274488e-03,  -1.2541015360921132e-02 };

		const real RGBIllum2SpectMagenta[NB_RGB_2_SPECT_SAMPLES] = {
			1.0371892935878366e+00,   1.0587542891035364e+00,
			1.0767271213688903e+00,   1.0762706844110288e+00,
			1.0795289105258212e+00,   1.0743644742950074e+00,
			1.0727028691194342e+00,   1.0732447452056488e+00,
			1.0823760816041414e+00,   1.0840545681409282e+00,
			9.5607567526306658e-01,   5.5197896855064665e-01,
			8.4191094887247575e-02,   8.7940070557041006e-05,
			-2.3086408335071251e-03,  -1.1248136628651192e-03,
			-7.7297612754989586e-11,  -2.7270769006770834e-04,
			1.4466473094035592e-02,   2.5883116027169478e-01,
			5.2907999827566732e-01,   9.0966624097105164e-01,
			1.0690571327307956e+00,   1.0887326064796272e+00,
			1.0637622289511852e+00,   1.0201812918094260e+00,
			1.0262196688979945e+00,   1.0783085560613190e+00,
			9.8333849623218872e-01,   1.0707246342802621e+00,
			1.0634247770423768e+00,   1.0150875475729566e+00 };

		const real RGBIllum2SpectYellow[NB_RGB_2_SPECT_SAMPLES] = {
			2.7756958965811972e-03,   3.9673820990646612e-03,
			-1.4606936788606750e-04,   3.6198394557748065e-04,
			-2.5819258699309733e-04,  -5.0133191628082274e-05,
			-2.4437242866157116e-04,  -7.8061419948038946e-05,
			4.9690301207540921e-02,   4.8515973574763166e-01,
			1.0295725854360589e+00,   1.0333210878457741e+00,
			1.0368102644026933e+00,   1.0364884018886333e+00,
			1.0365427939411784e+00,   1.0368595402854539e+00,
			1.0365645405660555e+00,   1.0363938240707142e+00,
			1.0367205578770746e+00,   1.0365239329446050e+00,
			1.0361531226427443e+00,   1.0348785007827348e+00,
			1.0042729660717318e+00,   8.4218486432354278e-01,
			7.3759394894801567e-01,   6.5853154500294642e-01,
			6.0531682444066282e-01,   5.9549794132420741e-01,
			5.9419261278443136e-01,   5.6517682326634266e-01,
			5.6061186014968556e-01,   5.8228610381018719e-01 };

		const real RGBIllum2SpectRed[NB_RGB_2_SPECT_SAMPLES] = {
			5.4711187157291841e-02,   5.5609066498303397e-02,
			6.0755873790918236e-02,   5.6232948615962369e-02,
			4.6169940535708678e-02,   3.8012808167818095e-02,
			2.4424225756670338e-02,   3.8983580581592181e-03,
			-5.6082252172734437e-04,   9.6493871255194652e-04,
			3.7341198051510371e-04,  -4.3367389093135200e-04,
			-9.3533962256892034e-05,  -1.2354967412842033e-04,
			-1.4524548081687461e-04,  -2.0047691915543731e-04,
			-4.9938587694693670e-04,   2.7255083540032476e-02,
			1.6067405906297061e-01,   3.5069788873150953e-01,
			5.7357465538418961e-01,   7.6392091890718949e-01,
			8.9144466740381523e-01,   9.6394609909574891e-01,
			9.8879464276016282e-01,   9.9897449966227203e-01,
			9.8605140403564162e-01,   9.9532502805345202e-01,
			9.7433478377305371e-01,   9.9134364616871407e-01,
			9.8866287772174755e-01,   9.9713856089735531e-01 };

		const real RGBIllum2SpectGreen[NB_RGB_2_SPECT_SAMPLES] = {
			2.5168388755514630e-02,   3.9427438169423720e-02,
			6.2059571596425793e-03,   7.1120859807429554e-03,
			2.1760044649139429e-04,   7.3271839984290210e-12,
			-2.1623066217181700e-02,   1.5670209409407512e-02,
			2.8019603188636222e-03,   3.2494773799897647e-01,
			1.0164917292316602e+00,   1.0329476657890369e+00,
			1.0321586962991549e+00,   1.0358667411948619e+00,
			1.0151235476834941e+00,   1.0338076690093119e+00,
			1.0371372378155013e+00,   1.0361377027692558e+00,
			1.0229822432557210e+00,   9.6910327335652324e-01,
			-5.1785923899878572e-03,   1.1131261971061429e-03,
			6.6675503033011771e-03,   7.4024315686001957e-04,
			2.1591567633473925e-02,   5.1481620056217231e-03,
			1.4561928645728216e-03,   1.6414511045291513e-04,
			-6.4630764968453287e-03,   1.0250854718507939e-02,
			4.2387394733956134e-02,   2.1252716926861620e-02 };

		const real RGBIllum2SpectBlue[NB_RGB_2_SPECT_SAMPLES] = {
			1.0570490759328752e+00,   1.0538466912851301e+00,
			1.0550494258140670e+00,   1.0530407754701832e+00,
			1.0579930596460185e+00,   1.0578439494812371e+00,
			1.0583132387180239e+00,   1.0579712943137616e+00,
			1.0561884233578465e+00,   1.0571399285426490e+00,
			1.0425795187752152e+00,   3.2603084374056102e-01,
			-1.9255628442412243e-03,  -1.2959221137046478e-03,
			-1.4357356276938696e-03,  -1.2963697250337886e-03,
			-1.9227081162373899e-03,   1.2621152526221778e-03,
			-1.6095249003578276e-03,  -1.3029983817879568e-03,
			-1.7666600873954916e-03,  -1.2325281140280050e-03,
			1.0316809673254932e-02,   3.1284512648354357e-02,
			8.8773879881746481e-02,   1.3873621740236541e-01,
			1.5535067531939065e-01,   1.4878477178237029e-01,
			1.6624255403475907e-01,   1.6997613960634927e-01,
			1.5769743995852967e-01, 1.9069090525482305e-01 };


	static Color rgbRefl2SpectWhite(Color::fromSampledSpectrum(RGB2SpectLambda, RGBRefl2SpectWhite, NB_RGB_2_SPECT_SAMPLES));
	static Color rgbRefl2SpectCyan(Color::fromSampledSpectrum(RGB2SpectLambda, RGBRefl2SpectCyan, NB_RGB_2_SPECT_SAMPLES));
	static Color rgbRefl2SpectMagenta(Color::fromSampledSpectrum(RGB2SpectLambda, RGBRefl2SpectMagenta, NB_RGB_2_SPECT_SAMPLES));
	static Color rgbRefl2SpectYellow(Color::fromSampledSpectrum(RGB2SpectLambda, RGBRefl2SpectYellow, NB_RGB_2_SPECT_SAMPLES));
	static Color rgbRefl2SpectRed(Color::fromSampledSpectrum(RGB2SpectLambda, RGBRefl2SpectRed, NB_RGB_2_SPECT_SAMPLES));
	static Color rgbRefl2SpectGreen(Color::fromSampledSpectrum(RGB2SpectLambda, RGBRefl2SpectGreen, NB_RGB_2_SPECT_SAMPLES));
	static Color rgbRefl2SpectBlue(Color::fromSampledSpectrum(RGB2SpectLambda, RGBRefl2SpectBlue, NB_RGB_2_SPECT_SAMPLES));
	static Color rgbIllum2SpectWhite(Color::fromSampledSpectrum(RGB2SpectLambda, RGBIllum2SpectWhite, NB_RGB_2_SPECT_SAMPLES));
	static Color rgbIllum2SpectCyan(Color::fromSampledSpectrum(RGB2SpectLambda, RGBIllum2SpectCyan, NB_RGB_2_SPECT_SAMPLES));
	static Color rgbIllum2SpectMagenta(Color::fromSampledSpectrum(RGB2SpectLambda, RGBIllum2SpectMagenta, NB_RGB_2_SPECT_SAMPLES));
	static Color rgbIllum2SpectYellow(Color::fromSampledSpectrum(RGB2SpectLambda, RGBIllum2SpectYellow, NB_RGB_2_SPECT_SAMPLES));
	static Color rgbIllum2SpectRed(Color::fromSampledSpectrum(RGB2SpectLambda, RGBIllum2SpectRed, NB_RGB_2_SPECT_SAMPLES));
	static Color rgbIllum2SpectGreen(Color::fromSampledSpectrum(RGB2SpectLambda, RGBIllum2SpectGreen, NB_RGB_2_SPECT_SAMPLES));
	static Color rgbIllum2SpectBlue(Color::fromSampledSpectrum(RGB2SpectLambda, RGBIllum2SpectBlue, NB_RGB_2_SPECT_SAMPLES));
}

#ifdef _MSC_VER
// Enable the double to float truncation warning
#pragma warning(default:4305)
#pragma warning(default:4838)
#endif

#endif


//class Color
//{
//public:
//	explicit Color(real r, real g, real b);
//	
//	explicit Color(real val = 0.);
//	//explicit Color(int r, int g, int b);
//
//	static Color fromBlackbody(real temperature);
//
//	void validate() 
//	{
//		r = r > 1.f ? 1.f : r < 0. ? 0.f : r;
//		g = g > 1.f ? 1.f : g < 0. ? 0.f : g;
//		b = b > 1.f ? 1.f : b < 0. ? 0.f : b;
//	}
//
//	real average() const
//	{
//		return (r + b + g) / (real)3.;
//	}
//
//	real max()
//	{
//		return std::max(r, std::max(g, b));
//	}
//
//	real luminance() const
//	{
//		return 0.2126f * r + 0.7152f * g + 0.0722f *b;
//	}
//
//	bool isZero() const
//	{
//#ifdef DOUBLE_PRECISION
//		const real eps = 1e-9;
//#else
//		const real eps = 1e-5f;
//#endif
//		return r < eps && g < eps && b < eps;
//	}
//
//	bool isNan() const
//	{
//		if (std::isnan(r) || std::isnan(g) || std::isnan(b))
//			return true;
//		return false;
//	}
//
//public:
//	real r;
//	real g;
//	real b;
//};

//class ColorSpaceConversion 
class CIEConstants
{
public:
	void precompute();

	std::vector<real> X;
	std::vector<real> Y;
	std::vector<real> Z;

	real xyz[3];

	//static bool computed;

	static const int LAMBDA_START = 400; 
	static const int LAMBDA_END = 700;
	static const int CIE_SAMPLE_NUMBER = 471;
	static const real CIE_Y_INTEGRAL;// = 106.856895f;

	static const int SAMPLE_NUMBER = 30;

	static const real CIE_X[CIE_SAMPLE_NUMBER];
	static const real CIE_Y[CIE_SAMPLE_NUMBER];
	static const real CIE_Z[CIE_SAMPLE_NUMBER];
	static const real CIE_lambda[CIE_SAMPLE_NUMBER];

	CIEConstants(const std::vector<real>& lambda, const std::vector<real>& spectrum);

	void toRGB(real rgb[3]) const;

	void toXYZ(real _xyz[3]) const;

	void XYZToRGB(const real _xyz[3], real rgb[3]) const;

	static real averageSpectrumSamples(const real *lambda, const real *vals,
		int n, real lambdaStart, real lambdaEnd);

protected:
	real c[SAMPLE_NUMBER];

};

//inline std::ostream& operator << (std::ostream& o, const Color& color)
//{
//	o << "[" << color.r << " " << color.g << " " << color.b << "]" << std::endl;
//
//	return o;
//}
//
//bool operator == (const Color& c1, const Color& c2);
//
//bool operator != (const Color& c1, const Color& c2);
//
//Color operator + (const Color& c1, const Color& c2);
//
//Color& operator += (Color& c1, const Color& c2);
//
//Color operator - (const Color& c1);
//
//Color operator - (const Color& c1, const Color& c2);
//
//Color& operator -= (Color& c1, const Color& c2);
//
//Color operator * (const Color& c1, const Color& c2);
//
//Color& operator *= (Color& c1, const Color& c2);
//
//Color& operator *= (Color& c1, real val);
//
//Color operator *(real value, const Color& other);
//
//Color operator *(const Color& other, real value);
//
////Color operator /(real value, const Color& other);
//
//Color operator /(const Color& other, real value);
//
//Color& operator /=(Color& other, real value);
//
//Color operator /(const Color& col, const Color& other);
//
//Color operator /= (Color& col, const Color& other);
////Color operator *(Color& col, real value);
//
//Color operator + (const Color& c1, real value);
//
//Color operator + (real value, const Color& c1);
//
//Color& operator += (Color& c1, real value);


















//class Color : public Eigen::Array<priv::real, 3, 1>
//{
//public:
//	explicit Color(priv::real r, priv::real g, priv::real b);
//
//	Color(const Color& c)
//		:r(coeffRef(0))
//		, g(coeffRef(1))
//		, b(coeffRef(2))
//	{
//		r = c.r;
//		g = c.g;
//		b = c.b;
//	}
//
//	Color& operator = (const Color& c) {
//		coeffRef(0) = c.coeff(0);
//		coeffRef(1) = c.coeff(1);
//		coeffRef(2) = c.coeff(2);
//
//		return *this;
//	}
//
//	explicit Color(priv::real val = 0.);
//	//explicit Color(int r, int g, int b);
//
//	template<typename OtherDerived>
//	Color(const Eigen::MatrixBase<OtherDerived>& other)
//		: Eigen::Matrix<priv::real, 3, 1>(other)
//		, r(coeffRef(0))
//		, g(coeffRef(1))
//		, b(coeffRef(2))
//	{ }
//
//	template <typename Derived> Color &operator=(const Eigen::MatrixBase<Derived>& other) {
//		this->Base::operator=(other);
//		return *this;
//	}
//
//	static Color fromBlackbody(priv::real temperature);
//
//	void validate()
//	{
//		r = r > 1.f ? 1.f : r < 0. ? 0.f : r;
//		g = g > 1.f ? 1.f : g < 0. ? 0.f : g;
//		b = b > 1.f ? 1.f : b < 0. ? 0.f : b;
//	}
//
//	priv::real average() const
//	{
//		return (r + b + g) / (priv::real)3.;
//	}
//
//	priv::real max()
//	{
//		return std::max(r, std::max(g, b));
//	}
//
//	priv::real luminance() const
//	{
//		return 0.2126f * r + 0.7152f * g + 0.0722f *b;
//	}
//
//	bool isZero() const
//	{
//#ifdef DOUBLE_PRECISION
//		const real eps = 1e-9;
//#else
//		const priv::real eps = 1e-5f;
//#endif
//		return r < eps && g < eps && b < eps;
//	}
//
//	bool isNan() const
//	{
//		if (std::isnan(r) || std::isnan(g) || std::isnan(b))
//			return true;
//		return false;
//	}
//
//public:
//	priv::real& r;
//	priv::real& g;
//	priv::real& b;
//};


#endif
