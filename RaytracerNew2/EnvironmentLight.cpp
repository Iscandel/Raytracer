#include "EnvironmentLight.h"

#include "ObjectFactoryManager.h"
#include "Scene.h"
#include "Tools.h"

#include <SFML\Graphics.hpp>

EnvironmentLight::EnvironmentLight(const Parameters& params)
{
	std::string path = params.getString("path", "");
	myLightToWorld = params.getTransform("toWorld", Transform::ptr(new Transform));
	myWorldToLight = Transform::ptr(new Transform(myLightToWorld->inv()));

	sf::Image im;
	im.loadFromFile(path);
	myArray.setSize(im.getSize().x, im.getSize().y);
	for (unsigned int y = 0; y < im.getSize().y; y++)
	{
		for (unsigned int x = 0; x <im.getSize().x; x++)
		{
			sf::Color col = im.getPixel(x, y);
			myArray(x, y) = Color(col.r / 255., col.g / 255., col.b / 255.) / 1.5;
		}
	}

	for (unsigned int i = 0; i < myArray.getHeight(); i++)
	{
		double weight = std::sin(tools::PI * (i + 0.5) / im.getSize().y);
		myRowCDFs.push_back(CDF());
		for (unsigned int j = 0; j < myArray.getWidth(); j++)
		{
			myRowCDFs[i].add(myArray(j, i).luminance());
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
	return 4 * tools::PI * mySphereRadius * mySphereRadius * Color(1.);
}

LightSamplingInfos EnvironmentLight::sample(const Point3d & pFrom, const Point2d& sample)
{
	LightSamplingInfos infos;

	double pdfRow, pdfCol;
	int row = myMarginalCDF.sample(sample.x(), pdfRow);
	int col = myRowCDFs[row].sample(sample.y(), pdfCol);

	double theta = tools::PI * row / (double) myArray.getHeight();
	double phi = 2 * tools::PI * col / (double)myArray.getWidth();
	double cosTheta = std::cos(theta);
	double cosPhi = std::cos(phi);
	double sinTheta = std::sin(theta);
	double sinPhi = std::sin(phi);
	infos.interToLight = Vector3d(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta);
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
		infos.intensity = myArray(tools::thresholding((int)col, 0, (int)myArray.getWidth() - 1), tools::thresholding((int)row, 0, (int)myArray.getHeight() - 1));
		infos.pdf = infos.intensity.luminance() * myWeight[row] / (myMarginalCDF.getSum() * (2 * tools::PI / myArray.getWidth()) * (tools::PI / myArray.getHeight()));//(pdfRow * pdfCol) / (2 * tools::PI * tools::PI * std::max(std::abs(sinTheta), tools::EPSILON); //To change ?
		infos.pdf /= std::max(std::abs(sinTheta), tools::EPSILON);
	}

	return infos;
}

double EnvironmentLight::pdf(const Point3d & pFrom, const LightSamplingInfos & infos)
{
	Vector3d dir = myWorldToLight->transform(infos.interToLight);
	//transform world to light
	double phi = (sphericalPhiFromCartesian(dir) / (2 * tools::PI)) * myArray.getWidth();
	double theta = (sphericalThetaFromCartesian(dir) / tools::PI) * myArray.getHeight();
	double sinTheta = std::sin(theta);
	double normalizationFactor = myWeight[(int)theta] / (myMarginalCDF.getSum() * (2 * tools::PI / myArray.getWidth()) * (tools::PI / myArray.getHeight()));
	double luminance = interp2(Point2d(phi, theta)).luminance();
	double pdf = luminance * normalizationFactor;
	//double pdf = myArray((int)phi, (int)theta).luminance() / normalizationFactor;
	pdf /= std::max(std::abs(sinTheta), tools::EPSILON);
	//myWeight[(int)theta] / (myMarginalCDF.getSum() * (2 * tools::PI / myArray.getWidth()) * (tools::PI / myArray.getHeight()));//(pdfRow * pdfCol) / (2 * tools::PI * tools::PI * sinTheta); //To change ?
	return pdf;
}

Color EnvironmentLight::le(const Vector3d & direction, const Normal3d & normal) const
{
	Vector3d dir = myWorldToLight->transform(direction);
	//transform world to light
	double phi = (sphericalPhiFromCartesian(dir) / (2 * tools::PI)) * myArray.getWidth();
	double theta = (sphericalThetaFromCartesian(dir) / tools::PI) * myArray.getHeight();
	
	return interp2(Point2d(phi, theta));
	//double advanceX = phi - (int)phi;
	//double advanceY = theta - (int)theta;
	//int xMin = (int)phi;
	//int yMin = (int)theta;
	//int xMax = xMin + 1 < myArray.getWidth() ? xMin + 1 : xMin;
	//int yMax = yMin + 1 < myArray.getHeight() ? yMin + 1 : yMin;

	//return (1 - advanceX) * (1 - advanceY) * myArray(xMin, yMin) +
	//	   (1 - advanceX) * advanceY * myArray(xMin, yMax) + 
	//	   advanceX * (1 - advanceY) * myArray(xMax, yMin) + 
	//	   advanceX * advanceY * myArray(xMax, yMax);
}

Color EnvironmentLight::interp2(const Point2d& xy) const
{
	double advanceX = xy.x() - (int)xy.x();
	double advanceY = xy.y() - (int)xy.y();
	int xMin = tools::thresholding((int)xy.x(), 0, (int)myArray.getWidth());
	int yMin = tools::thresholding((int)xy.y(), 0, (int)myArray.getHeight());
	int xMax = xMin + 1 < myArray.getWidth() ? xMin + 1 : xMin;
	xMax = tools::thresholding(xMax, 0, (int)myArray.getWidth() - 1);
	int yMax = yMin + 1 < myArray.getHeight() ? yMin + 1 : yMin;
	yMax = tools::thresholding(yMax, 0, (int)myArray.getHeight() - 1);

	return (1 - advanceX) * (1 - advanceY) * myArray(xMin, yMin) +
		(1 - advanceX) * advanceY * myArray(xMin, yMax) +
		advanceX * (1 - advanceY) * myArray(xMax, yMin) +
		advanceX * advanceY * myArray(xMax, yMax);
}

void EnvironmentLight::initialize(const Scene & scene)
{
	BoundingBox box = scene.getBoundingBox();
	myCenter = box.getCentroid();
	//We double the radius just to be sure
	mySphereRadius = 2 * std::max((box.getMin() - myCenter).norm(), (box.getMax() - myCenter).norm());
}

RT_REGISTER_TYPE(EnvironmentLight, Light)
