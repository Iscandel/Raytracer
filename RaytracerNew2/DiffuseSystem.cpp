//#include "DiffuseSystem.h"
//
//#include "Scene.h"
//#include "Intersection.h"
//
//#include <vector>
//
//DiffuseSystem::DiffuseSystem(const std::string& name)
//:System(name)
//{
//}
//
//
//DiffuseSystem::~DiffuseSystem(void)
//{
//}
//
//Color DiffuseSystem::compute(Scene& scene, const GeometricShape& object, const Point3d& inter, const Ray& ray, int level)
//{
//	Color diffuse;
//	//const std::vector<DirectionalLight, Eigen::aligned_allocator<DirectionalLight>>& lights = scene.getDirectionalLights();
//	//for(unsigned int i = 0; i< lights.size(); i++)
//	//{
//	//	Vector3d interToLight(inter, lights[i].getPosition());
//	//	interToLight.normalize();
//	//	Vector3d normalObject = object.normal(inter);
//
//	//	double d;
//	//	Ray shadowRay(inter, interToLight);
//	//	Intersection inter;
//	//	if(!scene.computeIntersection(shadowRay, inter))//d))
//	//	{
//	//		//double dotLightNormal = Vector3d::dot(interToLight, normalObject);
//	//		double dotLightNormal = interToLight.dot(normalObject);
//
//	//		//diffuse += Obj.getRhoDiffusion() * Obj.getCouleur() * MesLampes[i].getCouleur() * psLampeNormale;
//	//		//ok
//	//		Color c = object.getColor() * lights[i].getColorIntensity() * object.getDiffuseCoeff() * dotLightNormal;
//
//	//		//double r = c.r * Obj.getRhoDiffusion() * psLampeNormale;
//	//		//double g = c.g * Obj.getRhoDiffusion() * psLampeNormale;
//	//		//double b = c.b * Obj.getRhoDiffusion() * psLampeNormale;
//
//	//		//r = r < 0 ? 0 : r > 255. ? 255. : r;
//	//		//g = g < 0 ? 0 : g > 255. ? 255. : g;
//	//		//b = b < 0 ? 0 : b > 255. ? 255. : b;
//
//	//		//c.r = r; c.g = g; c.b = b;
//
//	//		diffuse += c;
//	//	}
//	//}
//
//	return diffuse;
//}