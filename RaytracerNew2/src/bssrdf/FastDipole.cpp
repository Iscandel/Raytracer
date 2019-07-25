#include "FastDipole.h"

#include "tools/Common.h"
#include "core/Fresnel.h"
#include "integrator/Integrator.h"
#include "core/Intersection.h"
#include "core/Math.h"
#include "core/Mapping.h"
#include "factory/ObjectFactoryManager.h"
#include "tools/Rng.h"
#include "core/Scene.h"
#include "medium/MediumPreset.h"

#include "sampler/RandomSampler.h"

struct IrradianceSample
{
	IrradianceSample() {}
	IrradianceSample(const Point3d& _pos, const Color& irr, real _area)
		:pos(_pos)
		, irradiance(irr)
		, area(_area)
	{}

	Point3d pos;
	Color irradiance;
	real area;
};

struct OctreeNode
{
	struct OctreeUnion
	{
		std::unique_ptr<OctreeNode> children[8];
		std::vector<IrradianceSample> samples;

		OctreeUnion() {}
		~OctreeUnion() {}
	};

	OctreeNode(const BoundingBox& box)
		:bbox(box)
		, isLeaf(false)
		, Ev(Color(0.f))
		, Av(0.f)
		, Pv(Point3d(0.f))
	{}

	OctreeNode(const BoundingBox& box, const std::vector<IrradianceSample>& points)
		:bbox(box)
		, isLeaf(true)
		, Ev(Color(0.f))
		, Av(0.f)
		, Pv(Point3d(0.f))
	{
		this->active.samples = points;

		real weight = 0.f;
		for (const IrradianceSample& s : points)
		{
			Ev += s.irradiance;
			Av += s.area;
			real w = s.irradiance.luminance() * s.area;
			Pv += s.pos * w;
			weight += w;
		}
		if (weight > 0.f)
			Pv /= weight;
		if (points.size() > 0)
			Ev /= (real)points.size();

		if (std::isinf(Pv(0)) || std::isinf(Pv(1)) || std::isinf(Pv(2))) {
			Pv = Point3d();
			Ev = Color();
		}
	}

	Color Ev; //total irradiance in voxel
	real Av; //Total area
	Point3d Pv; //Average location
	BoundingBox bbox;
	bool isLeaf;
	OctreeUnion active;
};

struct Octree
{
public:
	Octree() { myMaxDepth = 24; }
	void build(const std::vector<IrradianceSample>& points)
	{
		rootNode = std::make_unique<OctreeNode>(getBbox(points));
		recurseBuild(*rootNode, points, 0);
	}

	void recurseBuild(OctreeNode& node, const std::vector<IrradianceSample>& points, int depth)
	{
		//real weight = 0.f;
		//for (const IrradianceSample& s : points)
		//{
		//	node.Ev += s.irradiance;
		//	node.Av += s.area;
		//	real w = s.irradiance.luminance() * s.area;
		//	node.Pv += s.pos * w;
		//	weight += w;
		//}
		//if(weight > 0.f)
		//	node.Pv /= weight;
		//if(points.size() > 0)
		//	node.Ev /= (real) points.size();

		Point3d extent = node.bbox.getMax() - node.bbox.getMin();
		Point3d minParent = node.bbox.getMin();
		Point3d maxParent = node.bbox.getMax();
		Point3d center = node.bbox.getCentroid();
		//real midX = node.bbox.getMin().x() + extent.x() / 2.f;
		//real midY = node.bbox.getMin().y() + extent.y() / 2.f;
		//real midZ = node.bbox.getMin().z() + extent.z() / 2.f;

		//y up
		//
		//up part       lower part
		// ________      ________
		//|_1__|_2_|    |_5__|_6_|
		//|_0__|_3_|    |_4__|_7_|
		std::vector<IrradianceSample> p[8];
		BoundingBox bbox[8];
		bbox[0] = BoundingBox(Point3d(minParent.x(), maxParent.y(), maxParent.z()), center);
		bbox[1] = BoundingBox(Point3d(minParent.x(), maxParent.y(), minParent.z()), center);
		bbox[2] = BoundingBox(Point3d(maxParent.x(), maxParent.y(), minParent.z()), center);
		bbox[3] = BoundingBox(Point3d(maxParent.x(), maxParent.y(), maxParent.z()), center);
		bbox[4] = BoundingBox(Point3d(minParent.x(), minParent.y(), maxParent.z()), center);
		bbox[5] = BoundingBox(Point3d(minParent.x(), minParent.y(), minParent.z()), center);
		bbox[6] = BoundingBox(Point3d(maxParent.x(), minParent.y(), minParent.z()), center);
		bbox[7] = BoundingBox(Point3d(maxParent.x(), minParent.y(), maxParent.z()), center);
		//for (const IrradianceSample& s : points)
		//{
		//	if (s.pos.x() < midX && s.pos.z() >= midZ && s.pos.y() >= midY)
		//		p[0].push_back(s);
		//	else if (s.pos.x() < midX  && s.pos.z() >= midZ && s.pos.y() < midY)
		//		p[4].push_back(s);
		//	else if (s.pos.x() >= midX && s.pos.z() >= midZ && s.pos.y() >= midY)
		//		p[3].push_back(s);
		//	else if (s.pos.x() >= midX  && s.pos.z() >= midZ && s.pos.y() < midY)
		//		p[7].push_back(s);
		//	else if (s.pos.x() < midX && s.pos.z() < midZ && s.pos.y() >= midY)
		//		p[1].push_back(s);
		//	else if (s.pos.x() < midX && s.pos.z() < midZ && s.pos.y() < midY)
		//		p[5].push_back(s);
		//	else if (s.pos.x() >= midX && s.pos.z() < midZ && s.pos.y() >= midY)
		//		p[2].push_back(s);
		//	else if (s.pos.x() >= midX && s.pos.z() < midZ  && s.pos.y() < midY)
		//		p[6].push_back(s);
		//	else
		//		std::cout << s.pos << std::endl;
		//}

		for (const IrradianceSample& s : points)
			for (int i = 0; i < 8; i++)
				if (bbox[i].contains(s.pos)) {
					p[i].push_back(s);
					break;
				}

		real weight = 0.f;

		for (int i = 0; i < 8; i++)
		{
			//Collect points
			if (p[i].size() < 8 || depth > myMaxDepth)
			{
				//for (const IrradianceSample& s : p[i])
				//{
				//	node.Ev += s.irradiance;
				//	node.Av += s.area;
				//	node.Pv += s.pos;
				//}
				//node.Pv /= node.Ev.average();
				if (p[i].size() > 0)
					node.active.children[i] = std::make_unique<OctreeNode>(bbox[i], p[i]);
			}
			else
			{
				if (p[i].size() > 0)
				{
					node.active.children[i] = std::make_unique<OctreeNode>(bbox[i]);
					//for (const IrradianceSample& s : p[i])
					//{
					//	node.Ev += s.irradiance;
					//	node.Av += s.area;
					//	node.Pv += s.pos * s.irradiance.average();
					//}
					//node.Pv /= node.Ev.average();
					recurseBuild(*node.active.children[i], p[i], depth + 1);
				}
			}
			
			if (node.active.children[i])
			{
				node.Ev += node.active.children[i]->Ev * node.active.children[i]->Av;
				node.Av += node.active.children[i]->Av;
				real w = node.active.children[i]->Ev.luminance() * node.active.children[i]->Av;
				node.Pv += node.active.children[i]->Pv * w;
				weight += w;
			}
		}

		if (weight > 0.f)// && (node.Pv.array() > 0.).any())
			node.Pv /= weight;
		if (node.Av > 0.f)
			node.Ev /= (real)node.Av;

		if (std::isinf(node.Pv(0)) || std::isinf(node.Pv(1)) || std::isinf(node.Pv(2)))
			node.Pv = Point3d();;
	}

	BoundingBox getBbox(const std::vector<IrradianceSample>& points)
	{
		Point3d min(math::MAX_REAL, math::MAX_REAL, math::MAX_REAL);
		Point3d max(-math::MAX_REAL, -math::MAX_REAL, -math::MAX_REAL);

		for (const IrradianceSample& s : points)
		{
			if (s.pos.x() < min.x()) min.x() = s.pos.x();
			if (s.pos.y() < min.y()) min.y() = s.pos.y();
			if (s.pos.z() < min.z()) min.z() = s.pos.z();
			if (s.pos.x() > max.x()) max.x() = s.pos.x();
			if (s.pos.y() > max.y()) max.y() = s.pos.y();
			if (s.pos.z() > max.z()) max.z() = s.pos.z();
		}

		return BoundingBox(min, max);
	}

	Color eval(const Point3d& pos, real maxError, real Fdr, const Color& zr, const Color& zv, const Color& sigmaTr)
	{
		real Fdt = 1.f - Fdr;
		
		return eval(pos, maxError, Fdt, zr, zv, sigmaTr, *rootNode);
	}

	//Color eval(const Point3d& pos, real maxError, real eta, const Color& sigmaPrimeT, const Color& sigmaA, const OctreeNode& node)
	Color eval(const Point3d& pos, real maxError, real Fdt, const Color& zr, const Color& zv, const Color& sigmaTr, const OctreeNode& node)
	{
		Color MoVal;
		real error = node.Av / ((pos - node.Pv).squaredNorm());
		if (std::isnan(error)) {
			ILogger::log() << "[FastDipole] nan error\n";
			return MoVal;
		}

		if (!isInside(node, pos) && error < maxError)
		{
			MoVal += Mo(Fdt, zr, zv, sigmaTr, pos, node.Pv, node.Ev, node.Av);
		}
		else
		{
			if (node.isLeaf)
			{
				for (IrradianceSample s : node.active.samples)
					MoVal += Mo(Fdt, zr, zv, sigmaTr, pos, s.pos, s.irradiance, s.area);
				//MoVal += Mo(eta, sigmaPrimeT, sigmaA, pos, s.pos, s.irradiance, s.area);
			}
			else
			{
				for (int i = 0; i < 8; i++)
				{
					OctreeNode* child = node.active.children[i].get();
					if (child)
						MoVal += eval(pos, maxError, Fdt, zr, zv, sigmaTr, *child);
					//MoVal += eval(pos, maxError, eta, sigmaPrimeT, sigmaA, *child);
				}
			}
		}

		return MoVal;
	}

	Color Mo(real Fdt, const Color& zr, const Color& zv, const Color& sigmaTr, const Point3d& x0, const Point3d& xi, const Color& Ep, real Ap)
	{
		real r = (x0 - xi).norm();
		//r = r*r*r*r*r*r;
		Color r2 = Color((x0 - xi).squaredNorm());
		//Color dr = Color(std::sqrt(r * r + zr.r * zr.r), std::sqrt(r * r + zr.g * zr.g), std::sqrt(r * r + zr.b * zr.b));
		//Color dv = Color(std::sqrt(r * r + zv.r * zv.r), std::sqrt(r * r + zv.g * zv.g), std::sqrt(r * r + zv.b * zv.b));
		Color dr = math::fastSqrt(r2 + zr * zr);
		Color dv = math::fastSqrt(r2 + zv * zv);
		Color C1 = zr * (sigmaTr + Color(1.) / dr);
		Color C2 = zv * (sigmaTr + Color(1.) / dv);
		//Color term = math::INV_FOUR_PI * (C1 * Color(std::exp(-sigmaTr.r * dr.r), std::exp(-sigmaTr.g * dr.g), std::exp(-sigmaTr.b * dr.b)) / (dr * dr) +
		//	C2 * Color(std::exp(-sigmaTr.r * dv.r), std::exp(-sigmaTr.g * dv.g), std::exp(-sigmaTr.b * dv.b)) / (dv * dv));
		Color term = math::INV_FOUR_PI * (C1 * math::fastExp(-sigmaTr * dr) / (dr * dr) +
										   C2 * math::fastExp(-sigmaTr * dv) / (dv * dv));

		return Fdt * term * Ep * Ap;
	}

	//Color Mo(real eta, const Color& sigmaPrimeT, const Color& sigmaA, const Point3d& x0, const Point3d& xi, const Color& Ep, real Ap)
	//{
	//	real Fdr = -1.440f / (eta * eta) + 0.710f / eta + 0.668f + 0.0636f * eta;
	//	real Fdt = 1.f - Fdr;
	//	Color lu = 1.f / sigmaPrimeT;
	//	real A = (1 + Fdr) / (1 - Fdr);
	//	Color zr = lu;
	//	Color zv = lu * (1 + 4.f / 3 * A);
	//	Color sigmaTr = Color(std::sqrt(3 * sigmaA.r * sigmaPrimeT.r), std::sqrt(3 * sigmaA.g * sigmaPrimeT.g), std::sqrt(3 * sigmaA.b * sigmaPrimeT.b));
	//	real r = (x0 - xi).norm();
	//	Color dr = Color(std::sqrt(r * r + zr.r * zr.r), std::sqrt(r * r + zr.g * zr.g), std::sqrt(r * r + zr.b * zr.b));
	//	Color dv = Color(std::sqrt(r * r + zv.r * zv.r), std::sqrt(r * r + zv.g * zv.g), std::sqrt(r * r + zv.b * zv.b));
	//	Color C1 = zr * (sigmaTr + 1. / dr);
	//	Color C2 = zv * (sigmaTr + 1. / dv);
	//	Color term = math::INV_FOUR_PI * (C1 * Color(std::exp(-sigmaTr.r * dr.r), std::exp(-sigmaTr.g * dr.g), std::exp(-sigmaTr.b * dr.b)) / (dr * dr) + 
	//								 C2 * Color(std::exp(-sigmaTr.r * dv.r), std::exp(-sigmaTr.g * dv.g), std::exp(-sigmaTr.b * dv.b)) / (dv * dv));

	//	return Fdt * term * Ep * Ap;
	//}

	bool isInside(const OctreeNode& node, const Point3d& pos)
	{
		return node.bbox.contains(pos);
		//Point3d min = node.bbox.getMin();
		//Point3d max = node.bbox.getMax();

		//if (pos.x() > min.x() && pos.y() > min.y() && pos.z() > min.z() &&
		//	pos.x() < max.x() && pos.y() < max.y() && pos.z() < max.z())
		//	return true;

		//return false;
	}

	std::unique_ptr<OctreeNode> rootNode;
	int myMaxDepth;
};

FastDipole::FastDipole(const Parameters& params)
{
	myNumberOfPoints = params.getInt("numberPoints", 100);
	myEtaExt = params.getReal("etaExt", 1.);
	//myEtaInt = params.getReal("etaInt", 1.5);
	myMaxError = params.getReal("error", 0.05f);
	//Color sigmaPrimeS = params.getColor("sigmaS", Color());
	//mySigmaA = params.getColor("sigmaA", Color());
	Color sigmaS;
	Color g;
	MediumPreset::fromParams(params, mySigmaA, sigmaS, g, myEtaInt);
	real scale = params.getReal("scale", 1.f);
	myNbSamplesLight = params.getInt("lightSamples", 16);
	myIsMultithreadedOctreeInit = params.getBool("multithreadedInit", true);
	myIsIndirectIrradiance = params.getBool("indirectIrradiance", true);

	mySqueezeZeroIrradiance = params.getBool("squeezeZeroIrradiance", false);
	myScaleResult = params.getReal("scaleResult", 1.); //hack in case the light is close the bssrdf, Mo exceeds float max value

	mySigmaA *= scale;
	sigmaS *= scale;

	Color sigmaPrimeS = sigmaS * (Color(1.f) - g);

	//mySigmaA *= scale;
	//sigmaPrimeS *= scale;

	mySigmaPrimeT = sigmaPrimeS + mySigmaA;

	myFdr = fresnel::estimateDiffuseReflectance(myEtaInt / myEtaExt);


	Color lu = Color(1.f) / mySigmaPrimeT;
	real A = (1 + myFdr) / (1 - myFdr);
	myZr = lu;
	myZv = lu * (1 + 4.f / 3 * A);
	//mySigmaTr = Color(std::sqrt(3 * mySigmaA.r * mySigmaPrimeT.r), std::sqrt(3 * mySigmaA.g * mySigmaPrimeT.g), std::sqrt(3 * mySigmaA.b * mySigmaPrimeT.b));
	mySigmaTr = math::fastSqrt(3 * mySigmaA * mySigmaPrimeT);

	myIsInitialized = false;
}


FastDipole::~FastDipole()
{
}

//void FastDipole::computeIrradiance(Scene& scene)
//{
//
//}

void FastDipole::initialize(Scene & scene)
{
	if (myIsInitialized)
		return;

	//Rng rng;
	std::shared_ptr<ISampledShape> shape = myShape.lock();

	real area = shape->surfaceArea() / myNumberOfPoints;

	Integrator* integrator = scene.getIntegrator().get();
	std::vector<IrradianceSample> irrVector;

	irrVector.resize(myNumberOfPoints);
	auto computeIrradiance = [&](int start, int end, Sampler::ptr sampler)
	{
		const Scene::LightVector& lights = scene.getLights();
		//for (int i = 0; i < myNumberOfPoints; i++)
		for (int i = 0; i < (end - start) + 1; i++)
		{
			//if (i % 10000 == 0)
			//	std::cout << i << std::endl;
			Color irradiance;
			//Point2d sample(rng.nextReal(), rng.nextReal());
			Point2d sample = sampler->getNextSample2D();
			Point3d sampled;
			Normal3d normal;
			shape->sample(sample, sampled, normal);
			DifferentialGeometry geom(normal);
			Intersection inter;
			inter.myPoint = sampled;
			inter.myShadingGeometry = inter.myTrueGeometry = geom;

			Medium::ptr medium = scene.getCameraMedium(); //to modify !!!!!!!!!!!!!!! Use also primitive->getExteriorMedium() (some changes to do)
			irradiance = integrator->irradiance(scene, myNbSamplesLight, inter, medium, sampler, myIsIndirectIrradiance);

			//for (int j = 0; j < lights.size(); j++)
			//{
			//	

			//	Color irrLight;
			//	for (int k = 0; k < myNbSamplesLight; k++)
			//	{
			//		//sample = Point2d(rng.nextReal(), rng.nextReal());
			//		sample = sampler->getNextSample2D();
			//		LightSamplingInfos infos = lights[j]->sample(sampled, sample);
			//		Ray shadowRay(sampled, infos.interToLight, math::EPSILON, infos.distance - math::EPSILON);
			//		Intersection tmp;

			//		if (!scene.computeIntersection(shadowRay, tmp, true))
			//		{
			//			real cosTheta = infos.interToLight.dot(normal);
			//			if (cosTheta > 0.)
			//			{
			//				Medium::ptr medium = scene.getCameraMedium();
			//				//if(!medium)
			//				//	medium = 
			//				//Medium::ptr = scene.getMedium(sampled);

			//				Color tr = scene.getIntegrator()->evalTransmittance(scene, shadowRay, medium, scene.getSampler());
			//				irrLight += tr * infos.intensity * infos.interToLight.dot(normal) / infos.pdf;
			//			}
			//		}

			//		if (myIsIndirectIrradiance)
			//		{
			//			Vector3d dir = inter.toWorld(Mapping::squareToCosineWeightedHemisphere(sampler->getNextSample2D()));
			//			Integrator::RadianceType::ERadianceType type = Integrator::RadianceType::INDIRECT_RADIANCE;
			//			Ray ray(sampled, dir);
			//			//Indirect irradiance is E = integral(Li (n.w) dw) = Li / pdf * (n.w)
			//			//pdf is a weighted cosine -> cos / pi
			//			//so E = Li * pi
			//			irrLight += scene.getIntegrator()->li(scene, sampler, ray, type) * math::PI;
			//		}
			//	}

			//	irrLight /= (real)myNbSamplesLight;
			//	irradiance += irrLight;
			//}
			//if (!irradiance.isZero())
			{
				IrradianceSample irrSample(sampled, irradiance, area);
				irrVector[start + i] = irrSample;
				//irrVector.push_back(irrSample);
			}
		}
	};

	if (myIsMultithreadedOctreeInit)
	{
		ILogger::log() << "Acquiring irradiance (multithreaded)\n";
		int nbThreads = getCoreNumber();
		std::vector<std::unique_ptr<std::thread>> threads;
		threads.resize(nbThreads);
		int nbPointsThread = myNumberOfPoints / nbThreads;
		if (myNumberOfPoints < nbThreads)
		{
			nbPointsThread = myNumberOfPoints;
			nbThreads = 1;
		}
		Sampler::ptr sampler(new RandomSampler(scene.getSampler()->getSampleNumber()));
		std::vector<Sampler::ptr> samplers(nbThreads);
		int start = 0; int end = nbPointsThread - 1;
		for (auto i = 0; i < nbThreads; i++)
		{
			samplers[i] = sampler->clone();
			samplers[i]->seed(i * nbPointsThread, myNumberOfPoints - (i * nbPointsThread));
			threads[i] = std::unique_ptr<std::thread>(new std::thread(computeIrradiance, start, end, samplers[i]));
			//if (i == nbThreads - 1)
			//	threads[i] = std::unique_ptr<std::thread>(new std::thread(computeIrradiance, (myNumberOfPoints - (nbThreads - 1) * nbPointsThread) - 1, i, samplers[i]));
			//else
			//	threads[i] = std::unique_ptr<std::thread>(new std::thread(computeIrradiance, nbPointsThread, i, samplers[i]));
			start = end + 1;
			end = std::min(myNumberOfPoints, end + nbPointsThread);
		}

		for (auto i = 0; i < nbThreads; i++)
		{
			threads[i]->join();
		}
	}
	else
	{
		ILogger::log() << "Acquiring irradiance\n";
		Sampler::ptr sampler(new RandomSampler(scene.getSampler()->getSampleNumber()));
		sampler->seed(myNumberOfPoints, 1);
		computeIrradiance(0, myNumberOfPoints - 1, scene.getSampler());
	}

	if (mySqueezeZeroIrradiance)
	{
		std::vector<IrradianceSample>::iterator it = irrVector.begin();
		irrVector.erase(std::remove_if(irrVector.begin(), irrVector.end(), [](const IrradianceSample& s) {return s.irradiance.average() < 1e-10f;}));
	}

	std::cout << irrVector.size() << std::endl;
	real min = std::numeric_limits<real>::max();
	real max = std::numeric_limits<real>::min();
	for (IrradianceSample& s : irrVector) {
		s.area = shape->surfaceArea() / irrVector.size();
		if (s.irradiance.average() < min)
			min = s.irradiance.average();
		if (s.irradiance.average() > max)
			max = s.irradiance.average();

		s.irradiance *= myScaleResult;
	}
	std::cout << min << std::endl;
	std::cout << max << std::endl;
	myOctree = std::make_unique<Octree>();
	myOctree->build(irrVector);

	myIsInitialized = true;
}

Color FastDipole::eval(const Point3d& pos, const Intersection& inter, const Vector3d& dir)
{
	if (!myIsInitialized || dir.dot(inter.myShadingGeometry.myN) < 0.)
		return Color();

	Color ft = Color(1.f) - fresnel::estimateDielectric(myEtaExt, myEtaInt, dir.dot(inter.myShadingGeometry.myN));
	Color Mo = myOctree->eval(pos, myMaxError, myFdr, myZr, myZv, mySigmaTr);
	//if ((Mo * math::INV_PI * ft / myFdr).isZero())
	//	std::cout << "nop" << std::endl;
	return Mo * math::INV_PI * ft / myFdr;
	//real Fdr = 1.440 / (myEta * myEta) + 0.710 / myEta + 0.0636 * myEta;
	//real fDt = 1. - Fdr;
	//real lu = 1. / mySigmaPrimeT;
	//real A = (1 + Fdr) / (1 - Fdr);
	//real lv = lu * (1 + 4. / 3 * A);
	//real sigmaTr = std::sqrt(3 * mySigmaA * mySigmaPrimeT);
	//real C1 = 
	//real term = 4 * math::PI * (C1 * std::exp(-sigmaTr * dr) / (dr * dr) + C2 * std::exp(-sigmaTr * dv) / (dv * dv))
}

RT_REGISTER_TYPE(FastDipole, BSSRDF)