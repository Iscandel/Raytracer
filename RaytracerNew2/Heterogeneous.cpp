#include "Heterogeneous.h"

#include "ObjectFactoryManager.h"
#include "Parameters.h"

#include <fstream>
//#undef min
//#undef max
#include <openvdb/openvdb.h>
#include <openvdb/tools/Interpolation.h>
//#include <openvdb/points/PointConversion.h>
//#include <openvdb/points/PointCount.h>


Heterogeneous::Heterogeneous(const Parameters& params)
	:Medium(params)
	,myWorldBoundingBox(Point3d(0,0,0), Point3d(1,1,1))
{
	//myData = NULL;
	// Denotes the scattering albedo
	myAlbedo = params.getColor("albedo", Color());

	myVolume = params.getVolume("volume", Volume::ptr());

	myIsCentered = params.getBool("centered", 1);

	// An (optional) transformation that converts between medium and world coordinates
	myWorldToWorldBox = Transform::ptr(new Transform(params.getTransform("toWorld", Transform::ptr(new Transform))->inv()));
	//myWorldToMedium = Transform::ptr(new Transform(params.getTransform("toWorld", Transform::ptr(new Transform))->inv()));
	myWorldToArray = Transform::ptr(new Transform);
	myWorldBoxToUnitBox = Transform::ptr(new Transform);;

	// Optional multiplicative factor that will be applied to all density values in the file
	myScale = params.getReal("scale", 1.);
	myScaleEmissivity = params.getReal("scaleEmissivity", 1.);

	std::string path = params.getString("path", "");

	myMaxSigmaT = myVolume->getMaxSigmaT() * myScale;

	const BoundingBox& bbox = myVolume->getBoundingBox();
	xmin = bbox.getMin().x(); ymin = bbox.getMin().y(); zmin = bbox.getMin().z();
	xmax = bbox.getMax().x(); ymax = bbox.getMax().y(); zmax = bbox.getMax().z();

//	std::ifstream file(path, std::ios::binary);
//
//	if (!file)
//		ILogger::log(ILogger::ERRORS) << "The file " << path << "does not exist!\n";
//
//	///* Parse the file header */
//	//file.open(QIODevice::ReadOnly);
//	//QDataStream stream(&file);
//	//stream.setByteOrder(QDataStream::LittleEndian);
//
//	int8_t header[3], version; int32_t type;
//	file.read(reinterpret_cast<char*>(&header[0]), sizeof(header[0]));
//	file.read(reinterpret_cast<char*>(&header[1]), sizeof(header[1]));
//	file.read(reinterpret_cast<char*>(&header[2]), sizeof(header[2])); 
//	file.read(reinterpret_cast<char*>(&version), sizeof(version));
//	file.read(reinterpret_cast<char*>(&type), sizeof(type));
//
//	if (memcmp(header, "VOL", 3) != 0 || version != 3)
//		ILogger::log(ILogger::ERRORS) << "This is not a valid volume data file!";
//
//	int resX, resY, resZ;
//	file.read(reinterpret_cast<char*>(&resX), sizeof(resX));
//	file.read(reinterpret_cast<char*>(&resY), sizeof(resY));
//	file.read(reinterpret_cast<char*>(&resZ), sizeof(resZ));
//	myResolution = Point3i(resX, resY, resZ);
//	int dataType;
//	file.read(reinterpret_cast<char*>(&dataType), sizeof(dataType));
//	//float xmin, ymin, zmin, xmax, ymax, zmax;
//	file.read(reinterpret_cast<char*>(&xmin), sizeof(xmin));
//	file.read(reinterpret_cast<char*>(&ymin), sizeof(ymin));
//	file.read(reinterpret_cast<char*>(&zmin), sizeof(zmin));
//	file.read(reinterpret_cast<char*>(&xmax), sizeof(xmax));
//	file.read(reinterpret_cast<char*>(&ymax), sizeof(ymax));
//	file.read(reinterpret_cast<char*>(&zmax), sizeof(zmax));
//	Point3d minBB(xmin, ymin, zmin);
//	Point3d maxBB(xmax, ymax, zmax);
//
//	std::cout << "Mapping \"" << path.data() << "\" (" << myResolution.x()
//		<< "x" << myResolution.y() << "x" << myResolution.z() << ") into memory ..bbox(" << minBB << " " << maxBB << std::endl;
//
//	//The volume box is set by default to 0-1
//	Transform mediumToArray;// = Transform::scale(Point3d(
//		//1. / (xmax - xmin),
//		//1. / (ymax - ymin),
//		//1. / (zmax - zmin))) * Transform::translate(Vector3d(-xmin, -ymin, -zmin));
//	//(*myWorldToArray) = mediumToArray * (*myWorldToMedium);
//
//	//First, a transform sets the medium into the container (the owner mesh). myWorldToWorldBox reverts it
//	//Next, the container is transformed to a unit cube [0, 1]^3
//	//Finally, the medium is scaled in the unit cube (origin, the (0, 0, 0)), according to its original scale
//	real maxExtent = (Point3d(xmax, ymax, zmax) - Point3d(xmin, ymin, zmin)).maxCoeff();
//	myUnitBoxToMedium = 
//		Transform::ptr(new Transform(Transform::scale(
//			Point3d(1.f / ((xmax - xmin) / maxExtent),
//					1.f / ((ymax - ymin) / maxExtent),
//					1.f / ((zmax - zmin) / maxExtent)))));
//	(*myWorldToArray) = *myWorldToWorldBox * *myWorldBoxToUnitBox * *myUnitBoxToMedium;
//
//	//
//	//file.close();
//	char a;
//	for (int i = 0; i < 4; i++)
//		file.read(&a, 1);
//
//	myData = new float[myResolution.x() * myResolution.y() * myResolution.z()];
//	file.read(reinterpret_cast<char*>(myData), myResolution.x() * myResolution.y() * myResolution.z() * sizeof(float));
//
//	file.seekg(std::ios::end);
//	myFileSize = (size_t)file.tellg();
//	std::cout << myFileSize << std::endl;
//
////#if defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
////	int fd = open(path.data(), O_RDONLY);
////	if (fd == -1)
////		throw NoriException(QString("Could not open \"%1\"!").arg(m_filename));
////	m_data = (float *)mmap(NULL, m_fileSize, PROT_READ, MAP_SHARED, fd, 0);
////	if (m_data == NULL)
////		throw NoriException("mmap(): failed.");
////	if (close(fd) != 0)
////		throw NoriException("close(): unable to close file descriptor!");
////#elif defined(WIN32)
////	myHfile = CreateFileA(path.data(), GENERIC_READ,
////		FILE_SHARE_READ, NULL, OPEN_EXISTING,
////		FILE_ATTRIBUTE_NORMAL, NULL);
////	if (myHfile == INVALID_HANDLE_VALUE)
////		ILogger::log(ILogger::ERRORS) << "Could not open " << path << "\n";;
////	myFileMapping = CreateFileMapping(myHfile, NULL, PAGE_READONLY, 0, 0, NULL);
////	if (myFileMapping == NULL)
////		ILogger::log(ILogger::ERRORS) << "CreateFileMapping(): failed.\n";
////	myData = (float *)MapViewOfFile(myFileMapping, FILE_MAP_READ, 0, 0, 0);
////	if (myData == NULL)
////		std::cout << "error" << std::endl;
////		//ILogger::log(ILogger::ERRORS) << "MapViewOfFile(): failed.";
////#endif
////
////	myData += 12; // Shift past the header
//
//	//myMaxSigmaT = myData[0];
//	//for (int i = 1; i < myResolution.x() * myResolution.y() * myResolution.z(); i++) {//myFileSize - 12; i++) {
//	//	//std::cout << i << std::endl;
//	//	//float v = myRng.nextFloat();
//	//	//myData[i] = v / 10.;
//	//	
//	//	//if(myData[i] > 0.)
//	//	//	std::cout << "i vaut " << i << " " << myData[i] << std::endl;
//	//	if (myData[i] > myMaxSigmaT) 
//	//		myMaxSigmaT = myData[i];
//	//}
//
//	//myMaxSigmaT *= myScale;
//	//std::cout << myMaxSigmaT << std::endl;
//
//	file.close();
//
//
//	openvdb::initialize();
//	// Create a shared pointer to a newly-allocated grid of a built-in type:
//	// in this case, a FloatGrid, which stores one single-precision floating point
//	// value per voxel.  Other built-in grid types include BoolGrid, DoubleGrid,
//	// Int32Grid and Vec3SGrid (see openvdb.h for the complete list).
//	// The grid comprises a sparse tree representation of voxel data,
//	// user-supplied metadata and a voxel space to world space transform,
//	// which defaults to the identity transform.
//	//openvdb::FloatGrid::Ptr grid =
//	//	openvdb::FloatGrid::create(/*background value=*/2.0);
//
//	// Create a VDB file object.
//	openvdb::io::File fileo("./data/smoke.vdb");
//	// Open the file.  This reads the file header, but not any grids.
//	fileo.open();
//	// Loop over all grids in the file and retrieve a shared pointer
//	// to the one named "LevelSetSphere".  (This can also be done
//	// more simply by calling file.readGrid("LevelSetSphere").)
//	openvdb::GridBase::Ptr baseGrid;
//	for (openvdb::io::File::NameIterator nameIter = fileo.beginName();
//		nameIter != fileo.endName(); ++nameIter)
//	{
//		// Read in only the grid we are interested in.
//		if (nameIter.gridName() == "density") {
//			baseGrid = fileo.readGrid(nameIter.gridName());
//		}
//		else {
//			std::cout << "skipping grid " << nameIter.gridName() << std::endl;
//		}
//	}
//	fileo.close();
//	// From the example above, "LevelSetSphere" is known to be a FloatGrid,
//	// so cast the generic grid pointer to a FloatGrid pointer.
//	typedef openvdb::tree::Tree4<half, 5, 4, 3>::Type HalfTree;
//	//using openvdb::Grid<HalfTree> HalfGrid;
//
//	//if (baseGrid->isType<openvdb::FloatGrid>())
//	//	std::cout << "FloatGrid" << std::endl;
//	//using namespace openvdb;
//	//if (baseGrid->template isType<BoolGrid>())        std::cout << "1" << std::endl;
//	//else if (baseGrid->template isType<FloatGrid>())  std::cout << "2" << std::endl;
//	//else if (baseGrid->template isType<DoubleGrid>()) std::cout << "3" << std::endl;
//	//else if (baseGrid->template isType<Int32Grid>())  std::cout << "4" << std::endl;
//	//else if (baseGrid->template isType<Int64Grid>())  std::cout << "5" << std::endl;
//	//else if (baseGrid->template isType<Vec3IGrid>())  std::cout << "6" << std::endl;
//	//else if (baseGrid->template isType<Vec3SGrid>())  std::cout << "7" << std::endl;
//	//else if (baseGrid->template isType<Vec3DGrid>())  std::cout << "8" << std::endl;
//	//else if (baseGrid->template isType<points::PointDataGrid>()) {
//	//	std::cout << "FloatGrid" << std::endl;
//	//}
//	grid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
//	//openvdb::Vec3SGrid::Ptr grid = openvdb::gridPtrCast<openvdb::Vec3SGrid>(baseGrid);
//	//openvdb::points::PointDataGrid::Ptr grid2 =
//	//	openvdb::gridPtrCast<openvdb::points::PointDataGrid>(baseGrid);
//
//
//
//	//// Get an accessor for coordinate-based access to voxels.
//	//openvdb::FloatGrid::Accessor accessor = grid->getAccessor();
//	//// Define a coordinate with large signed indices.
//	//openvdb::Coord xyz(1000, -200000000, 30000000);
//	//// Set the voxel value at (1000, -200000000, 30000000) to 1.
//	//accessor.setValue(xyz, 1.0);
//	//// Verify that the voxel value at (1000, -200000000, 30000000) is 1.
//	//std::cout << "Grid" << xyz << " = " << accessor.getValue(xyz) << std::endl;
//	//// Reset the coordinates to those of a different voxel.
//	//xyz.reset(1000, 200000000, -30000000);
//	//// Verify that the voxel value at (1000, 200000000, -30000000) is
//	//// the background value, 0.
//	//std::cout << "Grid" << xyz << " = " << accessor.getValue(xyz) << std::endl;
//
//	//On prend la grille entre [0, 0, 0] et bboxIndex.max
//	auto bbox = grid->evalActiveVoxelBoundingBox();
//	std::cout << "start " << bbox.getStart() << std::endl;
//	std::cout << bbox.getEnd() << std::endl;
//	std::cout << grid->indexToWorld(bbox.getStart()) << std::endl;
//	std::cout << grid->indexToWorld(bbox.getEnd()) << std::endl;
//	myResolution = Point3i(bbox.getEnd().x(), bbox.getEnd().y(), bbox.getEnd().z());
//	// Print all active ("on") voxels by means of an iterator.
//
////	xmin = grid->indexToWorld(openvdb::Coord(0,0,0)).x(); ymin = grid->indexToWorld(openvdb::Coord(0, 0, 0)).y();  zmin = grid->indexToWorld(openvdb::Coord(0, 0, 0)).z();
////	xmax = grid->indexToWorld(bbox.getEnd()).x(); ymax = grid->indexToWorld(bbox.getEnd()).y(); zmax = grid->indexToWorld(bbox.getEnd()).z();
//	//grid->setTransform(
//	//	openvdb::math::Transform::createLinearTransform((/*voxel size=*/0.5));
//	//openvdb::FloatGrid::Ptr grid =
//	//	openvdb::FloatGrid::create(/*background value=*/2.0);
//	openvdb::FloatGrid::ValueOnIter iter = grid->beginValueOn();
//	myMaxSigmaT = *iter;
//	for (; iter; ++iter) {
//		if (myMaxSigmaT < *iter)
//			myMaxSigmaT = *iter;
//	//	std::cout << "Grid" << iter.getCoord() << " = " << *iter << std::endl;
//	}
//	myMaxSigmaT *= myScale;
//
//	//OPENVDB_STATICLIB
//	//OPENVDB_3_ABI_COMPATIBLE
}


Heterogeneous::~Heterogeneous()
{
//	if (myData) {
//		myData -= 12;
//
//		//cout << "Unmapping \"" << qPrintable(m_filename) << "\" from memory.." << endl;
//#if defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
//		int retval = munmap(m_data, m_fileSize);
//		if (retval != 0)
//			throw NoriException("munmap(): unable to unmap memory!");
//#elif defined(WIN32)
//		if (!UnmapViewOfFile(myData))
//			ILogger::log(ILogger::ERRORS) << "UnmapViewOfFile(): unable to unmap memory region\n";
//		if (!CloseHandle(myFileMapping))
//			ILogger::log(ILogger::ERRORS) << "CloseHandle(): unable to close file mapping!\n";
//		if (!CloseHandle(myHfile))
//			ILogger::log(ILogger::ERRORS) << "CloseHandle(): unable to close file\n";
//#endif
//	}
}

void Heterogeneous::setOwnerBBox(const BoundingBox& box)
{
	//Old implementation (partially wrong)
	//Ex : Bbox [4-6], medium [5-6] in this dimension which is 0.5 of bbox
	// Medium is [4-6] in the other 2 dimensions, which is entire bbox
	//----===
	//|  |  |
	//|__|__|
	//4  5  6
	//
	// Take for example point 5 (which is 0 in array space). Nb : array space is [0,1]^3
	// ws is "world space". Scale array is to respect medium ratio.
	// Array space / scale array (half bbox) / scale bbox (ws) / translate (ws)  / translate medium in bbox
	//   [0]	   x	0.5					x		2			x		+4					+1
	// = 5 !

	//New implementation
	//The medium tries to fit the bounding box as best as possible, while 
	//keeping its dimension ratio. By default it is put at the (xmin, ymin, zmin) 
	//of the bbox.
	//Ex : Medium is (0, 1) in first dim, (0, 2) in other dims.
	//Bbox [4-6] in all dims, we want to put medium in [5-6] in this dimension
	//
	//----===
	//|  |  |
	//|__|__|
	//4  5  6
	//Example, array to world:
	//Take for example point 5 (which is 0 in array space). Nb : array space is exactly [0,1]^3
	//ws is "world space". The scale array process respects medium ratio. The bounding box is
	//already in ws coords. First, scale array with its own world dimensions. Then, it is scaled 
	//to best fit the bbox with the min ratio for all dimensions (min(bbox extent / medium extent))
	//Ex: Bbox is (0, 1) in first dim and (0,2) in others. Medium is (0, 2) in first dim and (0,1) 
	//in others. Min ratio is so 1/2 (from first dim) and medium then occupies (0, 1) in first dim 
	//and (0, 0.5)
	// -- ==
	//|  |  |
	//|__|==
	//0  1  2
	//Then, the medium is translated by the bbox (xmin, ymin, zmin).
	//An additionnal transformation enabling to arbitrary scale, translation or rotate the 
	//the medium once correctly put at the origin of the bbox can be added optionally.
	//It is called the inv(myWorldToWorldBox) transform
	//with the previous example:
	//scale medium to world size: (1-0) , (2-0) , (2-0)  = 1, 2, 2
	//min ratio is: min(6-4 / 1-0, 6-4 / 2-0, 6-4 / 2-0) = min(2, 1, 1) = 1
	//translation: 4, 4, 4
	//additional transformation: translation (1, 0, 0)
	//Take for example point 6 in the first dim (which is xmax in the medium space, ie 1. in this dim

	// Array space / scale medium / min ratio / translate (ws)  / translate medium in bbox
	//   [1]	     x	1			x	1			+4					+1
	// = 6 !

	//following incorrect
	// Array space / scale medium / min ratio / translate (ws)  / translate medium in bbox
	//   [1]	     x	0.5			x	1			+4					+1
	// = 6 !
	real maxExtent = (Point3d(xmax, ymax, zmax) - Point3d(xmin, ymin, zmin)).maxCoeff();

	real ratio = std::min((box.getMax() - box.getMin()).x() / (xmax - xmin), (box.getMax() - box.getMin()).y() / (ymax - ymin));
	ratio = std::min(ratio, (box.getMax() - box.getMin()).z() / (zmax - zmin));
	myUnitBoxToMedium =
		Transform::ptr(new Transform(Transform::scale(
			Point3d(1.f / ((xmax - xmin) * /*maxExtent * */ ratio),
				1.f / ((ymax - ymin) * /*maxExtent * */ ratio),
				1.f / ((zmax - zmin) * /*maxExtent * */ ratio)))));

	if (myIsCentered)
	{
		Point3d newExtent = myUnitBoxToMedium->inv().transform(Point3d(1, 1, 1));//xmax - xmin, ymax - ymin, zmax - zmin));
		myWorldBoxToUnitBox = Transform::ptr(new Transform(
			Transform::translate(-(box.getMin() + ( (box.getMax()-box.getMin()) / 2.f - newExtent / 2.f) ) )));
		//Transform::translate(-(box.getMin() +  (box.getMax() - box.getMin()) / 2.f) ) ));
	}
	else
	{
		myWorldBoxToUnitBox = Transform::ptr(new Transform(Transform::translate(-box.getMin())));
	}
	//*myWorldBoxToUnitBox = Transform::scale(
	//	Point3d(1.f / (box.getMax().x() - box.getMin().x()),
	//		1.f / (box.getMax().y() - box.getMin().y()),
	//		1.f / (box.getMax().z() - box.getMin().z()))) * *myWorldBoxToUnitBox;

	//(*myWorldToArray) = *myWorldBoxToUnitBox * *myUnitBoxToMedium * *myWorldToWorldBox;
	(*myWorldToArray) =  *myUnitBoxToMedium * *myWorldBoxToUnitBox * *myWorldToWorldBox;

	myWorldBoundingBox = myWorldToArray->inv().transform(myWorldBoundingBox);
}

float Heterogeneous::lookupSigmaT(const Point3d &_p) const {

	return myVolume->lookupSigmaT(_p) * myScale;

	//Point3d p = _p.cwiseProduct(myResolution.cast<real>()),
	//	pf = Point3d(std::floor(p.x()), std::floor(p.y()), std::floor(p.z()));
	//Point3i p0 = pf.cast<int>();


	//if ((p0.array() < 0).any() || (p0.array() >= myResolution.array() - 1).head<3>().any())
	//	return 0.0f;

	//// Choose fractional coordinates in index space.
	//const openvdb::Vec3R ijk(p.x(), p.y(), p.z());
	//// Compute the value of the grid at ijk via nearest-neighbor (zero-order)
	//// interpolation.
	//openvdb::FloatGrid::ValueType v0 = openvdb::tools::PointSampler::sample(grid->tree(), ijk);
	//return v0;

	//size_t row = myResolution.x(),
	//	slab = row * myResolution.y(),
	//	offset = p0.z()*slab + p0.y()*row + p0.x();

	//const float
	//	d000 = myData[offset],
	//	d001 = myData[offset + 1],
	//	d010 = myData[offset + row],
	//	d011 = myData[offset + row + 1],
	//	d100 = myData[offset + slab],
	//	d101 = myData[offset + slab + 1],
	//	d110 = myData[offset + slab + row],
	//	d111 = myData[offset + slab + row + 1];

	//Vector3d w1 = p - pf, w0 = (1 - w1.array()).matrix();

	////std::cout << offset << " " << d000 << " " << d001 << " " << w0 << " " << w1 << std::endl;
	///* Trilinearly interpolate */
	//return (((d000 * w0.x() + d001 * w1.x()) * w0.y() +
	//	(d010 * w0.x() + d011 * w1.x()) * w1.y()) * w0.z() +
	//	((d100 * w0.x() + d101 * w1.x()) * w0.y() +
	//	(d110 * w0.x() + d111 * w1.x()) * w1.y()) * w1.z()) * (float)myScale;
}

Color Heterogeneous::lookupEmissivity(const Point3d &_p) const 
{
	return myVolume->lookupEmissivity(_p) * myScaleEmissivity;
}

Color Heterogeneous::transmittance(const Ray& _ray, Sampler::ptr sampler)
{
	Ray ray = myWorldToArray->transform(_ray);

	real tMax = ray.myMaxT - ray.myMinT;
	real tr = 1.;
	real sampledT = 0;

	int breaker = 0;

	real nearT, farT;
	BoundingBox box(Point3d(0., 0., 0.), (1., 1., 1.));

	if (!box.intersection(ray, nearT, farT))
		return Color(1.f);

	tMax = std::min(tMax, farT);
	sampledT = std::max(nearT, ray.myMinT);

	//real result = 0;
	//for (int i = 0; i < 2; i++)
	//{
	//	sampledT = std::max(nearT, ray.myMinT);
	//	while (true)
	//	{
	//		sampledT -= std::log(1.f - sampler->getNextSample1D()) / myMaxSigmaT;
	//		if (sampledT >= tMax)
	//		{
	//			result += 1;
	//			break;
	//		}

	//		real proba = lookupSigmaT(myWorldToArray->transform(_ray.getPointAt(sampledT)));
	//		if (proba / myMaxSigmaT > sampler->getNextSample1D())
	//			break;
	//	}
	//}

	//return Color(result / 2.f);

	while (true)
	{
		sampledT += -std::log(1.f - sampler->getNextSample1D()) / myMaxSigmaT;

		real proba = lookupSigmaT(ray.getPointAt(sampledT));// / myMaxSigmaT;
		//if (myRng.nextFloat() < proba)
		//	break;
		//if (sampledT > tMax)
		//	return Color(1.);

		if (sampledT >= tMax) 
			return Color(tr);

		tr *= 1 - std::max((real)0., proba / myMaxSigmaT);

		const real threshold = .1f;
		if (tr < threshold) {
			real q = std::max((real).05, 1 - tr);
			if (sampler->getNextSample1D() < q)
				return Color(0.);
			tr /= 1 - q;
		}

		if(breaker > 1024)
			return Color(0.);

		breaker++;
	}

	//return Color();
}

bool Heterogeneous::sampleDistance(const Ray &_ray, Sampler::ptr sampler, real &t, Color &weight, Color& emissivity)
{
	Ray ray = myWorldToArray->transform(_ray);

	t = ray.myMinT;
	float currentSigmaT = 0;

	real nearT, farT;
	if (!myWorldBoundingBox.intersection(_ray, nearT, farT))
	{
		weight = Color(1.);
		emissivity = Color(0.);
		t = ray.myMaxT;
		return false;
	}

	real tMax = std::min(ray.myMaxT, farT);
	//t = std::max(nearT, ray.myMinT);

	while (true)
	{
		t += -std::log(1.f - sampler->getNextSample1D()) / myMaxSigmaT;

		if (t > tMax)
		{
			weight = Color(1.);
			emissivity = Color();//lookupEmissivity(myWorldToArray->transform(_ray.getPointAt(tMax))) / myMaxSigmaT;
			t = tMax;
			return false;
		}
		currentSigmaT = lookupSigmaT(myWorldToArray->transform(_ray.getPointAt(t)));
		//std::cout << currentSigmaT << std::endl;
		real proba = currentSigmaT / myMaxSigmaT;
		if (sampler->getNextSample1D() < proba)
		{
			//std::cout << "a plus" << std::endl;
			break;
		}
			
	}

	

	//sigmaS * tr / pdf ?
	if (currentSigmaT != 0.f) {
		weight = myAlbedo;// *currentSigmaT;// *transmittance(_ray);
		emissivity = lookupEmissivity(myWorldToArray->transform(_ray.getPointAt(t))) / currentSigmaT;
		//weight += emissivity;
	}
	else {
		weight = Color(0.f);
		emissivity = Color(0.f);
	}

	return true;
}

LightSamplingInfos Heterogeneous::sample(const Point3d& pFrom, const Point2d& sample)
{
	LightSamplingInfos infos;

	//std::shared_ptr<ISampledShape> shape = myShape.lock();
	//shape->sample(sample, infos.sampledPoint, infos.normal);
	Point3d sampled = myVolume->sample(sample);

	infos.sampledPoint = myWorldToArray->inv().transform(sampled);
	infos.interToLight = infos.sampledPoint - pFrom;
	infos.distance = infos.interToLight.norm();
	infos.interToLight /= infos.distance;
	//real cosine = infos.normal.dot(-infos.interToLight);

	infos.intensity = lookupEmissivity(sampled);
	infos.pdf = pdf(pFrom, infos);

	return infos;
}

bool Heterogeneous::isEmissive() const 
{ 
	return myVolume->isEmissive(); 
}


RT_REGISTER_TYPE(Heterogeneous, Medium)