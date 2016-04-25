//#include "ReflectionSystem.h"
//
//#include "Scene.h"
//
//
//ReflectionSystem::ReflectionSystem(const std::string& name)
//:System(name)
//{
//}
//
//
//ReflectionSystem::~ReflectionSystem(void)
//{
//}
//
//Color ReflectionSystem::compute(Scene& scene, const GeometricShape& object, const Point3d& inter, const Ray& ray, int level)
//{
//	//Vector3d normalObject = object.normal(inter);
//	//Vector3d reflected;
//	//reflected = ray.direction() - 2 * normalObject.dot(ray.direction()) * normalObject;
//	////reflected.x() = ray.myDirection.x() - 2 * Vector3d::dot(normalObject, ray.myDirection) * normalObject.x();
//	////reflected.y() = ray.myDirection.y() - 2 * Vector3d::dot(normalObject, ray.myDirection) * normalObject.y();
//	////reflected.z() = ray.myDirection.z() - 2 * Vector3d::dot(normalObject, ray.myDirection) * normalObject.z();
//	//Ray reflectedRay(inter, reflected);
//
//	//return scene.castRay(reflectedRay, level + 1);
//}
