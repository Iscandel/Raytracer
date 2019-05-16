//#include "SpecularSystem.h"
//
//#include "DirectionalLight.h"
//#include "Scene.h"
//
//#include "Intersection.h"
//
//#include <vector>
//
//SpecularSystem::SpecularSystem(const std::string& name)
//:System(name)
//{
//}
//
//
//SpecularSystem::~SpecularSystem(void)
//{
//}
//
//Color SpecularSystem::compute(Scene& scene, const GeometricShape& object, const Point3d& inter, const Ray& ray, int level)
//{
//	Color spec;
//	//const std::vector<DirectionalLight, Eigen::aligned_allocator<DirectionalLight>>& lights = scene.getDirectionalLights();
//	//for(unsigned int i = 0; i< lights.size(); i++)
//	//{
//	//	Vector3d dirRay(ray.myDirection);
//	//	Vector3d interToLight(lights[i].getPosition(), inter);
//	//	interToLight.normalize();
//
//	//	double d;
//	//	Ray shadowRay(inter, interToLight);
//	//	Intersection intersection;
//	//	if(!scene.computeIntersection(shadowRay, intersection))// d))
//	//	{
//	//		Vector3d C(interToLight.x() - dirRay.x(), interToLight.y() - dirRay.y() , interToLight.z() - dirRay.z() );
//	//		
//	//		C.normalize();
//	//		Vector3d normalObject = object.normal(inter);
//
//	//		Vector3d reflected(dirRay.x() - 2 * normalObject.dot(dirRay) * normalObject.x());
//	//		//reflected.x() = dirRay.x() - 2 * Vector3d::dot(normalObject, dirRay) * normalObject.x();
//	//		//reflected.y() = dirRay.y() - 2 * Vector3d::dot(normalObject, dirRay) * normalObject.y();
//	//		//reflected.z() = dirRay.z() - 2 * Vector3d::dot(normalObject, dirRay) * normalObject.z();
//
//	//		//reflechi.x = directionLampe.x - 2 * produitScalaire(normaleSphere2, directionLampe) * normaleSphere2.x;
//	//		//reflechi.y = directionLampe.y - 2 * produitScalaire(normaleSphere2, directionLampe) * normaleSphere2.y;
//	//		//reflechi.z = directionLampe.z - 2 * produitScalaire(normaleSphere2, directionLampe) * normaleSphere2.z;
//	//		double psCNormal = C.dot(normalObject);//reflechi, Vecteur3(-directionLampe.x, -directionLampe.y, -directionLampe.z));
//	//		//double psCNormale = utiles::produitScalaire(reflechi, interVersLampe);
//	//		//V - 2NV N
//	//		//double dCouleurSpec[3];
//	//		//double rhoS = 0.8;
//	//		//for(int i = 0; i < 3; i++)
//	//		//{
//	//		//	dCouleurSpec[i] =  Obj.getRhoSpec() * Obj.getCouleur() * MesLampes[i].getCouleur() * pow(std::max(0., psCNormale), 100);
//	//		//	dCouleurSpec[i]  = dCouleurSpec[i] < 0 ? 0 : dCouleurSpec[i] > 1. ? 1. : dCouleurSpec[i];
//	//		//}
//
//	//		//sf::Color c = Obj.getCouleur() * MesLampes[i].getCouleur();
//	//		//double r = c.r * Obj.getRhoSpec() * pow(std::max(0., psCNormale), (double)Obj.getBrillance());
//	//		//double g = c.g * Obj.getRhoSpec() * pow(std::max(0., psCNormale), (double)Obj.getBrillance());
//	//		//double b = c.b * Obj.getRhoSpec() * pow(std::max(0., psCNormale), (double)Obj.getBrillance());
//
//	//		//r = r < 0 ? 0 : r > 255. ? 255. : r;
//	//		//g = g < 0 ? 0 : g > 255. ? 255. : g;
//	//		//b = b < 0 ? 0 : b > 255. ? 255. : b;
//
//	//		//c.r = r; c.g = g; c.b = b;
//
//	//		Color c = object.getColor() * lights[i].getColorIntensity() * object.getSpecularCoeff() * 
//	//			pow(std::max(0., psCNormal), (double)object.getShininessCoeff());
//
//	//		spec += c;
//	//	}
//	//}
//
//	return spec;
//}