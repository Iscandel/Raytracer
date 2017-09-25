#pragma once

#include "Parameters.h"
#include "Ray.h"
#include "Screen.h"
#include "Transform.h"

#include <Eigen/Geometry>

Point3d squareToDiskConcentric(const Point3d &sample);

///////////////////////////////////////////////////////////////////////////////
/// \brief 
///////////////////////////////////////////////////////////////////////////////
class Camera
{
public:
	typedef std::shared_ptr<Camera> ptr;

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor.
	///////////////////////////////////////////////////////////////////////////
	Camera(const Parameters& params);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Destructor
	///////////////////////////////////////////////////////////////////////////
	virtual ~Camera(void);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Affects the focal length.
	///
	/// \param f The focal length.
	///////////////////////////////////////////////////////////////////////////
	//void setFocal(double f) {myFocal = f;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Affects the pixel size.
	///
	/// \param meter the pixel length (assuming square pixels).
	///////////////////////////////////////////////////////////////////////////
	//void setMeterPerPixel(double meter) {myMPerPixel = meter;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Affects the screen size.
	///////////////////////////////////////////////////////////////////////////
	void setSize(int px, int py) {mySizeX = px, mySizeY = py; myScreen = Screen::ptr(new Screen(px, py));}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Defines the type of camera.
	///////////////////////////////////////////////////////////////////////////
	//void setType(CameraType type) {myCameraType = type;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Returns the screen size X.
	///////////////////////////////////////////////////////////////////////////
	int getSizeX() const {return mySizeX;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Returns the screen size Y.
	///////////////////////////////////////////////////////////////////////////
	int getSizeY() const {return mySizeY;}

	///////////////////////////////////////////////////////////////////////////
	/// 
	///	  z | /y
	///		|/____ x
	///
	///////////////////////////////////////////////////////////////////////////
	virtual Ray getRay(double px, double py, const Point2d& apertureSample) = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Affects the camera world transform.
	///////////////////////////////////////////////////////////////////////////
	void setWorldTransform(const Transform::ptr t) {
		//Eigen::Vector3d origin(-65.6055, 47.5762, 24.3583);
		//Eigen::Vector3d target(-64.8161, 47.2211, 23.8576);
		//Eigen::Vector3d up(0.299858, 0.934836, -0.190177);
		//
		//Eigen::Vector3d dir = (target - origin).normalized();

		//Eigen::Vector3d left = up.normalized().cross(dir).normalized();
		//Eigen::Vector3d newUp = dir.cross(left).normalized();
		//
		//Eigen::Matrix4d trafo;
		//trafo << left, newUp, dir, origin,
		//0, 0, 0, 1;
		//
		//Eigen::Affine3d truc(trafo);
		//myWorldTransform = Transform::ptr(new Transform(truc.matrix()));

		myWorldTransform = t;
	}
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Affects the image reconstruction filter.
	///////////////////////////////////////////////////////////////////////////
	void setReconstructionFilter(ReconstructionFilter::ptr filter) {myScreen->setFilter(filter);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Returns the screen object.
	///////////////////////////////////////////////////////////////////////////
	Screen& getScreen() {return *myScreen;}

	bool needsDoFSample() const { return myNeedDoFSample; }

protected:
	int mySizeX;
	int mySizeY;

	std::shared_ptr<Screen> myScreen;

	Transform::ptr myWorldTransform;

	bool myNeedDoFSample;

	double myLensRadius;
	double myFocalPlane;
};



//Eigen::Vector3d origin(-65.6055, 47.5762, 24.3583);
//Eigen::Vector3d target(-64.8161, 47.2211, 23.8576);
//Eigen::Vector3d up(0.299858, 0.934836, -0.190177);
//
//Vector3d dir = (target - origin).normalized();
//Vector3d left = up.normalized().cross(dir).normalized();
//Vector3d newUp = dir.cross(left).normalized();
//
//Eigen::Matrix4d trafo;
//trafo << left, newUp, dir, origin,
//0, 0, 0, 1;
//
//Eigen::Affine3d truc(trafo);
//myWorldTransform = Transform(truc.matrix());