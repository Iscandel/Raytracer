#include "EnvironmentLight.h"

#include "io/ImageLoader.h"
#include "core/Math.h"
#include "factory/ObjectFactoryManager.h"
#include "core/Scene.h"

EnvironmentLight::EnvironmentLight(const Parameters& params)
{
	std::string path = params.getString("path", "");
	myLightToWorld = params.getTransform("toWorld", Transform::ptr(new Transform));
	myWorldToLight = Transform::ptr(new Transform(myLightToWorld->inv()));
	myFactor = params.getReal("scale", 1.);
	myIsProbe = params.getBool("isProbe", false);
	real gamma = params.getReal("gamma", (real)0.);
	myOffsetU = params.getReal("offsetU", real(0.));
	myOffsetV = params.getReal("offsetV", real(0.));

	myArray = ImageLoader::load(path, ImageLoader::ALL, gamma);

	//Color res((real)0.);
	//for (int i = 0; i < myArray->getWidth(); i++)
	//	for (int j = 0; j < myArray->getHeight(); j++)
	//		res += (*myArray)(i, j);

	//res /= myArray->getWidth() * myArray->getHeight();
	//Color average = res;

	//res = Color(+std::numeric_limits<real>::infinity());
	//for (int i = 0; i < myArray->getWidth(); i++)
	//	for (int j = 0; j < myArray->getHeight(); j++)
	//		for (int k = 0; k < Color::NB_SAMPLES; k++)
	//			res(k) = std::min(res(k), (*myArray)(i, j)(k));

	//Color min = res;

	//res = Color(-std::numeric_limits<real>::infinity());
	//for (int i = 0; i < myArray->getWidth(); i++)
	//	for (int j = 0; j < myArray->getHeight(); j++)
	//		for (int k = 0; k < Color::NB_SAMPLES; k++)
	//			res(k) = std::max(res(k), ((*myArray)(i, j))(k));

	//Color max = res;

	//ILogger::log() << "Texture min: " << min << "Texture max: " << max << "Texture average: " << average;

	//if (myFactor != 1.)
	//{
	//	for (unsigned int i = 0; i < myArray->getHeight(); i++)
	//	{
	//		for (unsigned int j = 0; j < myArray->getWidth(); j++)
	//		{
	//			myArray(j, i) *= myFactor;
	//		}
	//	}
	//}

	for (unsigned int i = 0; i < myArray->getHeight(); i++)
	{
		real weight = std::sin(math::PI * (i + 0.5f) / myArray->getHeight());//.getSize().y);
		myRowCDFs.push_back(CDF());
		for (unsigned int j = 0; j < myArray->getWidth(); j++)
		{
			myRowCDFs[i].add(getValue(j, i).luminance());
		}
		
		myWeight.push_back(weight);
		myMarginalCDF.add(myRowCDFs[i].getSum() * weight);
		myRowCDFs[i].normalize();
	}

	myMarginalCDF.normalize();
}


EnvironmentLight::~EnvironmentLight()
{
}

Color EnvironmentLight::power() const
{
	return 4 * math::PI * mySphereRadius * mySphereRadius * Color(1.);
}

LightSamplingInfos EnvironmentLight::sample(const Point3d & pFrom, const Point2d& sample)
{
	LightSamplingInfos infos;

	real pdfRow, pdfCol;
	int row = myMarginalCDF.sample(sample.x(), pdfRow);
	int col = myRowCDFs[row].sample(sample.y(), pdfCol);

	//New
	real rcol = col + myOffsetU * myArray->getWidth();
	real rrow = row + myOffsetV * myArray->getHeight();
	checkXY(rcol, rrow);
	row = rrow;
	col = rcol;
	//

	real theta;
	real phi;
	if (myIsProbe)
	{
		//u and v in [-1,1]
		real v = (row / (real)myArray->getHeight()) * 2.f - 1;
		real u = (col / (real)myArray->getWidth()) * 2.f - 1;
		theta = std::atan2(v, u);
		phi = math::PI * math::fastSqrt(u*u + v*v);
	}
	else
	{
		theta = math::PI * row / (real) myArray->getHeight();
		phi = 2 * math::PI * col / (real)myArray->getWidth();
	}

	//real cosTheta = std::cos(theta);
	//real cosPhi = std::cos(phi);
	real sinTheta = std::sin(theta);
	//real sinPhi = std::sin(phi);
	infos.interToLight = cartesianFromSpherical(theta, phi);//Vector3d(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta);
	infos.interToLight = myLightToWorld->transform(infos.interToLight);
	infos.distance = mySphereRadius;
	infos.interToLight.normalize();
	infos.sampledPoint = Ray(pFrom, infos.interToLight).getPointAt(mySphereRadius);// Point3d(100000, 1000000, 100000);//infos.wi; //No !!!!!!!!!!!!!!!!!!!!!!!!!!!
	//if (std::abs(sinTheta) == 0)
	//{
	//	infos.pdf = 0.;
	//	infos.intensity = Color();
	//	//std::cout << "Theta nul\n";
	//}
	//else
	{		
#if NB_SPECTRUM_SAMPLES == 3
		infos.intensity = getValue(math::thresholding((int)col, 0, (int)myArray->getWidth() - 1), math::thresholding((int)row, 0, (int)myArray->getHeight() - 1));
		//infos.intensity = myArray(math::thresholding((int)col, 0, (int)myArray.getWidth() - 1), math::thresholding((int)row, 0, (int)myArray.getHeight() - 1));
#else
		Color3 res = myArray(math::thresholding((int)col, 0, (int)myArray.getWidth() - 1), math::thresholding((int)row, 0, (int)myArray.getHeight() - 1));
		infos.intensity = Color::fromRGB(res(0), res(1), res(2));
#endif
		infos.pdf = infos.intensity.luminance() * myWeight[row] / (myMarginalCDF.getSum() * (2 * math::PI / myArray->getWidth()) * (math::PI / myArray->getHeight()));//(pdfRow * pdfCol) / (2 * math::PI * math::PI * std::max(std::abs(sinTheta), math::EPSILON); //To change ?
		infos.pdf /= std::max(std::abs(sinTheta), math::EPSILON);
	}

	return infos;
}

real EnvironmentLight::pdf(const Point3d &, const LightSamplingInfos & infos)
{
	
	Vector3d dir = myWorldToLight->transform(infos.interToLight);
	dir.normalize();
	//if(isnan(dir.hasNaN()))
	//	std::cout << infos.interToLight << std::endl;
	//transform world to light
	real fromPhi;
	real fromTheta;
	if (myIsProbe)
	{
		//See Paul debevec website
		real r = (1 / math::PI) * std::acos(dir.z()) / std::sqrt(dir.x() * dir.x() + dir.y() * dir.y());
		//get u, v and rescale in [0,1]
		fromPhi = ((dir.x() * r + (real)1.) / (real)2) * myArray->getWidth();
		fromTheta = ((dir.y() * r + (real)1.) / (real)2) * myArray->getHeight();
	}
	else
	{
		fromPhi = (sphericalPhiFromCartesian(dir) / (2 * math::PI)) * myArray->getWidth();
		fromTheta = (sphericalThetaFromCartesian(dir) / math::PI) * myArray->getHeight();
	}

	//New
	fromPhi += myOffsetU * myArray->getWidth();
	fromTheta += myOffsetV * myArray->getHeight();
	checkXY(fromPhi, fromTheta);
	//

	//real phi = (sphericalPhiFromCartesian(dir) / (2 * math::PI)) * myArray.getWidth();
	//real theta = (sphericalThetaFromCartesian(dir) / math::PI) * myArray.getHeight();
	//if(isnan((float)theta))
	//	std::cout << infos.interToLight << " " << dir << std::endl;
	real sinTheta = std::sin(fromTheta);

	real normalizationFactor = myWeight[(int)fromTheta] / (myMarginalCDF.getSum() * (2 * math::PI / myArray->getWidth()) * (math::PI / myArray->getHeight()));
	real luminance = Color(math::interp2(Point2d(fromPhi, fromTheta), *myArray) * myFactor).luminance();
	real pdf = luminance * normalizationFactor;
	//real pdf = myArray((int)phi, (int)theta).luminance() / normalizationFactor;
	pdf /= std::max(std::abs(sinTheta), math::EPSILON);
	//myWeight[(int)theta] / (myMarginalCDF.getSum() * (2 * math::PI / myArray.getWidth()) * (math::PI / myArray.getHeight()));//(pdfRow * pdfCol) / (2 * math::PI * math::PI * sinTheta); //To change ?
	return pdf;
}

Color EnvironmentLight::le(const Vector3d & direction, const Point3d&, const Normal3d &) const
{
	Vector3d dir = myWorldToLight->transform(direction);
	dir.normalize();
	//transform world to light
	real fromPhi;
	real fromTheta;
	if (myIsProbe)
	{
		//See Paul debevec website
		real r = (1 / math::PI) * std::acos(dir.z()) / math::fastSqrt(dir.x() * dir.x() + dir.y() * dir.y());
		//get u, v and rescale in [0,1]
		fromPhi = ((dir.x() * r + (real)1.) / (real)2) * myArray->getWidth();
		fromTheta = ((dir.y() * r + (real)1.) / (real)2) * myArray->getHeight();
	}
	else
	{
		fromPhi = (sphericalPhiFromCartesian(dir) / (2 * math::PI)) * myArray->getWidth();
		fromTheta = (sphericalThetaFromCartesian(dir) / math::PI) * myArray->getHeight();
	}

	//New
	fromPhi += myOffsetU * myArray->getWidth();
	fromTheta += myOffsetV * myArray->getHeight();
	checkXY(fromPhi, fromTheta);
	//
	
#if NB_SPECTRUM_SAMPLES == 3
		return math::interp2(Point2d(fromPhi, fromTheta), *myArray) * myFactor;
#else
		Color3 res = math::interp2(Point2d(fromPhi, fromTheta), myArray) * myFactor;
		return Color::fromRGB(res(0), res(1), res(2));
#endif
	//real advanceX = phi - (int)phi;
	//real advanceY = theta - (int)theta;
	//int xMin = (int)phi;
	//int yMin = (int)theta;
	//int xMax = xMin + 1 < myArray.getWidth() ? xMin + 1 : xMin;
	//int yMax = yMin + 1 < myArray.getHeight() ? yMin + 1 : yMin;

	//return (1 - advanceX) * (1 - advanceY) * myArray(xMin, yMin) +
	//	   (1 - advanceX) * advanceY * myArray(xMin, yMax) + 
	//	   advanceX * (1 - advanceY) * myArray(xMax, yMin) + 
	//	   advanceX * advanceY * myArray(xMax, yMax);
}

//Color EnvironmentLight::interp2(const Point2d& xy) const
//{
//	real advanceX = xy.x() - (int)xy.x();
//	real advanceY = xy.y() - (int)xy.y();
//	int xMin = math::thresholding((int)xy.x(), 0, (int)myArray.getWidth());
//	int yMin = math::thresholding((int)xy.y(), 0, (int)myArray.getHeight());
//	int xMax = xMin + 1 < myArray.getWidth() ? xMin + 1 : xMin;
//	xMax = math::thresholding(xMax, 0, (int)myArray.getWidth() - 1);
//	int yMax = yMin + 1 < myArray.getHeight() ? yMin + 1 : yMin;
//	yMax = math::thresholding(yMax, 0, (int)myArray.getHeight() - 1);
//
//	return (1 - advanceX) * (1 - advanceY) * myArray(xMin, yMin) +
//		(1 - advanceX) * advanceY * myArray(xMin, yMax) +
//		advanceX * (1 - advanceY) * myArray(xMax, yMin) +
//		advanceX * advanceY * myArray(xMax, yMax);
//}

void EnvironmentLight::initialize(Scene & scene)
{
	BoundingBox box = scene.getBoundingBox();
	myCenter = box.getCentroid();
	//We double the radius just to be sure
	mySphereRadius = 2 * std::max((box.getMin() - myCenter).norm(), (box.getMax() - myCenter).norm());
}

//Repeat mode
void EnvironmentLight::checkXY(real& x, real& y) const
{
	if (x >= myArray->getWidth())
		x = (int)x % myArray->getWidth() + (x - (int)x); //modulo + decimal part
	else if (x < 0)
		x = myArray->getWidth() - std::abs((int)x) % myArray->getWidth() + (x - (int)x);
	if (y >= myArray->getHeight())
		y = (int)y % myArray->getHeight() + (y - (int)y);
	else if (y < 0)
		y = myArray->getHeight() - std::abs((int)y) % myArray->getHeight() + (y - (int)y);
}

RT_REGISTER_TYPE(EnvironmentLight, Light)
