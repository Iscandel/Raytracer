#include "OpenVDBVolume.h"

#include "factory/ObjectFactoryManager.h"

#include <openvdb\tools\Statistics.h>
#include <openvdb\tools\Interpolation.h>

OpenVDBVolume::OpenVDBVolume(const Parameters& params)
:Volume(params)
,myIsEmissive(false)
,myMinGridTemperature((real)0.f)
,myMaxGridTemperature((real)0.f)
{
	static bool isInit = false;
	if (!isInit) {
		openvdb::initialize();
		isInit = true;
	}
		// Create a shared pointer to a newly-allocated grid of a built-in type:
		// in this case, a FloatGrid, which stores one single-precision floating point
		// value per voxel.  Other built-in grid types include BoolGrid, DoubleGrid,
		// Int32Grid and Vec3SGrid (see openvdb.h for the complete list).
		// The grid comprises a sparse tree representation of voxel data,
		// user-supplied metadata and a voxel space to world space transform,
		// which defaults to the identity transform.
		//openvdb::FloatGrid::Ptr grid =
		//	openvdb::FloatGrid::create(/*background value=*/2.0);
	
		std::string path = params.getString("path", "");//"./data/smoke.vdb"
		std::string gridDensityName = params.getString("gridDensityName", "density");//"./data/smoke.vdb"
		std::string gridTemperatureName = params.getString("gridTemperatureName", "temperature");
		myTemperatureScale = params.getReal("temperatureScale", 1.);
		myInterpOrder = params.getInt("interpOrder", 2);

		// Create a VDB file object.
		openvdb::io::File file(path);//"./data/smoke.vdb");
		// Open the file.  This reads the file header, but not any grids.
		file.open();
		// Loop over all grids in the file and retrieve a shared pointer
		// to the one named "LevelSetSphere".  (This can also be done
		// more simply by calling file.readGrid("LevelSetSphere").)
		openvdb::GridBase::Ptr baseGrid;
		for (openvdb::io::File::NameIterator nameIter = file.beginName();
			nameIter != file.endName(); ++nameIter)
		{
			// Read in only the grid we are interested in.
			if (nameIter.gridName() == gridDensityName) {
				baseGrid = file.readGrid(nameIter.gridName());
				myDensityGrid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
			}
			else if (nameIter.gridName() == gridTemperatureName) {
				myIsEmissive = true;
				baseGrid = file.readGrid(nameIter.gridName());
				myTemperatureGrid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
		
				auto extrema = openvdb::tools::extrema(myTemperatureGrid->beginValueAll());
				
				//myTemperatureGrid->evalMinMax(myMinGridTemperature, myMaxGridTemperature);
				myMinGridTemperature = extrema.min(), myMaxGridTemperature = extrema.max();
				//std::cout << "extrema " << extrema.min() << " " << myMinGridTemperature << " " << myMaxGridTemperature << " " << extrema.max() << std::endl;

				auto bbox = myTemperatureGrid->evalActiveVoxelBoundingBox();
				myTemperatureResolution = Point3i(bbox.getEnd().x() - bbox.getStart().x(), bbox.getEnd().y() - bbox.getStart().y(), bbox.getEnd().z() - bbox.getStart().z());

				std::cout << "Temperature grid " << std::endl;
				std::cout << bbox.getStart() << std::endl;
				std::cout << bbox.getEnd() << std::endl;
				std::cout << myTemperatureGrid->indexToWorld(myTemperatureGrid->evalActiveVoxelBoundingBox().getStart()) << std::endl;
				std::cout << myTemperatureGrid->indexToWorld(myTemperatureGrid->evalActiveVoxelBoundingBox().getEnd()) << std::endl;

				myTemperatureIndexSpaceBoundingBoxMin = Point3i(bbox.getStart().x(), bbox.getStart().y(), bbox.getStart().z());
				myTemperatureIndexSpaceBoundingBoxMax = Point3i(bbox.getEnd().x(), bbox.getEnd().y(), bbox.getEnd().z());

				myTempGrid = openvdb::Vec3SGrid::create();
				auto accessor = myTempGrid->getAccessor();

				// Update all active values
				//for (openvdb::FloatGrid::ValueOnIter iter = myTemperatureGrid->beginValueOn(); iter; ++iter) {
				//	Color c = Color::fromBlackbody(
				//		(iter.getValue() - myMinGridTemperature) / (myMaxGridTemperature - myMinGridTemperature) * myTemperatureScale);
				//	openvdb::Vec3s vec(c.r, c.g, c.b);
				//	accessor.setValueOn(iter.getCoord(), vec);
				//}
				//// Visit all of the grid's inactive tile and voxel values and update the values
				//// that correspond to the interior region.
				//for (openvdb::FloatGrid::ValueOffIter iter = myTemperatureGrid->beginValueOff(); iter; ++iter) {
				//	if (iter.getValue() < 0.0) {
				//		iter.setValue(1.0);
				//		iter.setValueOff();
				//	}
				//}
			}
			else {
				std::cout << "skipping grid " << nameIter.gridName() << std::endl;
			}
		}
		file.close();
		// From the example above, "LevelSetSphere" is known to be a FloatGrid,
		// so cast the generic grid pointer to a FloatGrid pointer.
		typedef openvdb::tree::Tree4<half, 5, 4, 3>::Type HalfTree;
		//using openvdb::Grid<HalfTree> HalfGrid;
	
		//if (baseGrid->isType<openvdb::FloatGrid>())
		//	std::cout << "FloatGrid" << std::endl;
		//using namespace openvdb;
		//if (baseGrid->template isType<BoolGrid>())        std::cout << "1" << std::endl;
		//else if (baseGrid->template isType<FloatGrid>())  std::cout << "2" << std::endl;
		//else if (baseGrid->template isType<DoubleGrid>()) std::cout << "3" << std::endl;
		//else if (baseGrid->template isType<Int32Grid>())  std::cout << "4" << std::endl;
		//else if (baseGrid->template isType<Int64Grid>())  std::cout << "5" << std::endl;
		//else if (baseGrid->template isType<Vec3IGrid>())  std::cout << "6" << std::endl;
		//else if (baseGrid->template isType<Vec3SGrid>())  std::cout << "7" << std::endl;
		//else if (baseGrid->template isType<Vec3DGrid>())  std::cout << "8" << std::endl;
		//else if (baseGrid->template isType<points::PointDataGrid>()) {
		//	std::cout << "FloatGrid" << std::endl;
		//}
		//myDensityGrid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
		//openvdb::Vec3SGrid::Ptr grid = openvdb::gridPtrCast<openvdb::Vec3SGrid>(baseGrid);
		//openvdb::points::PointDataGrid::Ptr grid2 =
		//	openvdb::gridPtrCast<openvdb::points::PointDataGrid>(baseGrid);
	
	
	
		//// Get an accessor for coordinate-based access to voxels.
		//openvdb::FloatGrid::Accessor accessor = grid->getAccessor();
		//// Define a coordinate with large signed indices.
		//openvdb::Coord xyz(1000, -200000000, 30000000);
		//// Set the voxel value at (1000, -200000000, 30000000) to 1.
		//accessor.setValue(xyz, 1.0);
		//// Verify that the voxel value at (1000, -200000000, 30000000) is 1.
		//std::cout << "Grid" << xyz << " = " << accessor.getValue(xyz) << std::endl;
		//// Reset the coordinates to those of a different voxel.
		//xyz.reset(1000, 200000000, -30000000);
		//// Verify that the voxel value at (1000, 200000000, -30000000) is
		//// the background value, 0.
		//std::cout << "Grid" << xyz << " = " << accessor.getValue(xyz) << std::endl;
	
		//On prend la grille entre [0, 0, 0] et bboxIndex.max
		auto bbox = myDensityGrid->evalActiveVoxelBoundingBox();
		std::cout << "Density grid " << bbox.getStart() << std::endl;
		std::cout << bbox.getEnd() << std::endl;
		std::cout << myDensityGrid->indexToWorld(bbox.getStart()) << std::endl;
		std::cout << myDensityGrid->indexToWorld(bbox.getEnd()) << std::endl;
		myDensityResolution = Point3i(bbox.getEnd().x() - bbox.getStart().x(), bbox.getEnd().y() - bbox.getStart().y(), bbox.getEnd().z() - bbox.getStart().z());
		
		myDensityIndexSpaceBoundingBoxMin = Point3i(bbox.getStart().x(), bbox.getStart().y(), bbox.getStart().z());
		myDensityIndexSpaceBoundingBoxMax = Point3i(bbox.getEnd().x(), bbox.getEnd().y(), bbox.getEnd().z());
	
		//Compute the world bbox
		computeBoundingBox();
		std::cout << myBoundingBox << std::endl;
		//myBoundingBox = BoundingBox(//Point3d(grid->indexToWorld(openvdb::Coord(0, 0, 0)).x(), 
		//							//		grid->indexToWorld(openvdb::Coord(0, 0, 0)).y(),
		//							//		grid->indexToWorld(openvdb::Coord(0, 0, 0)).z()),
		//							Point3d(myDensityGrid->indexToWorld(bbox.getStart()).x(),
		//									myDensityGrid->indexToWorld(bbox.getStart()).y(),
		//									myDensityGrid->indexToWorld(bbox.getStart()).z()),
		//							Point3d(myDensityGrid->indexToWorld(bbox.getEnd()).x(),
		//									myDensityGrid->indexToWorld(bbox.getEnd()).y(),
		//									myDensityGrid->indexToWorld(bbox.getEnd()).z()));
	//	xmin = grid->indexToWorld(openvdb::Coord(0,0,0)).x(); ymin = grid->indexToWorld(openvdb::Coord(0, 0, 0)).y();  zmin = grid->indexToWorld(openvdb::Coord(0, 0, 0)).z();
	//	xmax = grid->indexToWorld(bbox.getEnd()).x(); ymax = grid->indexToWorld(bbox.getEnd()).y(); zmax = grid->indexToWorld(bbox.getEnd()).z();
		//grid->setTransform(
		//	openvdb::math::Transform::createLinearTransform((/*voxel size=*/0.5));
	
		//OPENVDB_STATICLIB
		//OPENVDB_3_ABI_COMPATIBLE
}


OpenVDBVolume::~OpenVDBVolume()
{
}

void OpenVDBVolume::computeBoundingBox()
{
	auto bboxDensity = myDensityGrid->evalActiveVoxelBoundingBox();
	myBoundingBox = BoundingBox(
		Point3d(myDensityGrid->indexToWorld(bboxDensity.getStart()).x(),
			myDensityGrid->indexToWorld(bboxDensity.getStart()).y(),
			myDensityGrid->indexToWorld(bboxDensity.getStart()).z()),
		Point3d(myDensityGrid->indexToWorld(bboxDensity.getEnd()).x(),
			myDensityGrid->indexToWorld(bboxDensity.getEnd()).y(),
			myDensityGrid->indexToWorld(bboxDensity.getEnd()).z()));

	if (myTemperatureGrid)
	{
		auto bboxEmission = myTemperatureGrid->evalActiveVoxelBoundingBox();
		BoundingBox worldBboxEmission(
			Point3d(myTemperatureGrid->indexToWorld(bboxEmission.getStart()).x(),
				myTemperatureGrid->indexToWorld(bboxEmission.getStart()).y(),
				myTemperatureGrid->indexToWorld(bboxEmission.getStart()).z()),
			Point3d(myTemperatureGrid->indexToWorld(bboxEmission.getEnd()).x(),
				myTemperatureGrid->indexToWorld(bboxEmission.getEnd()).y(),
				myTemperatureGrid->indexToWorld(bboxEmission.getEnd()).z()));

		myBoundingBox = BoundingBox::unionBox(myBoundingBox, worldBboxEmission);
	}
}

real OpenVDBVolume::getMaxSigmaT() const
{
	openvdb::FloatGrid::ValueOnCIter iter = myDensityGrid->cbeginValueOn();
	real maxSigmaT = *iter;
	for (; iter; ++iter) {
		if (maxSigmaT < *iter)
			maxSigmaT = *iter;
		//	std::cout << "Grid" << iter.getCoord() << " = " << *iter << std::endl;
	}
	//myMaxSigmaT *= myScale;

	return maxSigmaT;
}

real OpenVDBVolume::lookupSigmaT(const Point3d &_p) const
{
	//Linear interpolation
	//translate p to fit the min-max index values of the medium. _p must be in 
	//[0, 1] to pass through the medium (but of course, it can take other values.  
	//If so, the point will be discarded in the following). Multiply it by the 
	//resolution and add the min index
	Point3d p = _p.cwiseProduct(myDensityResolution.cast<real>()) + myDensityIndexSpaceBoundingBoxMin.cast<real>(),
		pf = Point3d(std::floor(p.x()), std::floor(p.y()), std::floor(p.z()));
	Point3i p0 = pf.cast<int>();

	//if ((p0.array() < 0).any() || (p0.array() >= myResolution.array() - 1).head<3>().any())
	//	return 0.0f;

	if ((p0.array() < myDensityIndexSpaceBoundingBoxMin.array()).any() || (p0.array() >= myDensityIndexSpaceBoundingBoxMax.array() - 1).head<3>().any())
		return 0.0f;

	// Choose fractional coordinates in index space.
	const openvdb::Vec3R ijk(p.x(), p.y(), p.z());

	openvdb::FloatGrid::ValueType v0;
	if (myInterpOrder == 0) {
		// Compute the value of the grid at ijk via nearest-neighbor (zero-order)
		// interpolation.
		v0 = openvdb::tools::PointSampler::sample(myDensityGrid->tree(), ijk);
	}
	else if (myInterpOrder == 1) {
		// Compute the value via trilinear (first-order) interpolation.
		v0 = openvdb::tools::BoxSampler::sample(myDensityGrid->tree(), ijk);
	} else {
		// Compute the value via triquadratic (second-order) interpolation.
		v0 = openvdb::tools::QuadraticSampler::sample(myDensityGrid->tree(), ijk);
	}

	if (v0 < 0.) {
		v0 = 0;
	}
	return v0;
}

Color OpenVDBVolume::lookupEmissivity(const Point3d &_p) const
{
	//return Color::fromBlackbody(2700);

	if (myTemperatureGrid == nullptr)
		return Color();

	Point3d p = _p.cwiseProduct(myTemperatureResolution.cast<real>()) + myTemperatureIndexSpaceBoundingBoxMin.cast<real>(),
		pf = Point3d(std::floor(p.x()), std::floor(p.y()), std::floor(p.z()));
	Point3i p0 = pf.cast<int>();

	//if ((p0.array() < 0).any() || (p0.array() >= myResolution.array() - 1).head<3>().any())
	//	return 0.0f;

	if ((p0.array() < myTemperatureIndexSpaceBoundingBoxMin.array()).any() || 
		(p0.array() >= myTemperatureIndexSpaceBoundingBoxMax.array() - 1).head<3>().any())
		return Color(0.0f);

	// Choose fractional coordinates in index space.
	const openvdb::Vec3R ijk(p.x(), p.y(), p.z());

	openvdb::FloatGrid::ValueType v0;
	if (myInterpOrder == 0) {
		// Compute the value of the grid at ijk via nearest-neighbor (zero-order)
		// interpolation.
		v0 = openvdb::tools::PointSampler::sample(myTemperatureGrid->tree(), ijk);
	}
	else if (myInterpOrder == 1) {
		// Compute the value via trilinear (first-order) interpolation.
		v0 = openvdb::tools::BoxSampler::sample(myTemperatureGrid->tree(), ijk);
	}
	else {
		// Compute the value via triquadratic (second-order) interpolation.
		v0 = openvdb::tools::QuadraticSampler::sample(myTemperatureGrid->tree(), ijk);
	}
	v0 = v0 < (real)0 ? (real)0 : v0;
	Color c = Color::fromBlackbody((v0 - myMinGridTemperature) / (myMaxGridTemperature - myMinGridTemperature) * myTemperatureScale);
	return c;
	//if (c.r < 0 || c.g < 0 || c.b < 0)
	//{
	//	std::cout << c << " " << v0 << ijk << std::endl;
	//	Color::fromBlackbody(v0 * myTemperatureScale);
	//}
}

const BoundingBox& OpenVDBVolume::getBoundingBox() const
{
	return myBoundingBox;
}

Point3d OpenVDBVolume::sample(const Point2d& samplePoint)
{
	//res in [0, 1]^3
	real u = samplePoint.x() * myDensityResolution.x() * myDensityResolution.z();
	real v = samplePoint.y() * myDensityResolution.y();

	Point3d res;
	res.x() = ((u - (int) u) + (int)u % myDensityResolution.x()) / myDensityResolution.x();
	res.y() = v / myDensityResolution.y();
	res.z() =  ((int)u / myDensityResolution.x()) / (real)myDensityResolution.z();

	return res;
}

//Very rough approximation...
Color OpenVDBVolume::getAverageEmission() const
{
	return Color::fromBlackbody(myTemperatureScale) / (real)2.f;
}

RT_REGISTER_TYPE(OpenVDBVolume, Volume)