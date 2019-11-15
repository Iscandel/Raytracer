#pragma once

#include "tools/Array2D.h"
#include "core/Color.h"
#include "texture/Texture.h"

#include <functional>
#include <map>

class ImageLoader
{
public:
	enum Channel
	{
		ALL,
		R,
		G,
		B,
		A
	};

public:
	typedef std::map<std::size_t, std::shared_ptr<Array2D<Color3>>> CacheMap;

public:
	ImageLoader();
	~ImageLoader();

	static std::shared_ptr<Array2D<Color3>> load(const std::string & path, Channel channel, real gammaFrom, real gammaDest = (real)1.);
	static bool isHDRFile(const std::string& path);
	static Channel channelToInt(const std::string& channel);

protected:
	
	static Color3 spaceColorCorrection(real gammaFrom, real invGammaDest, real r, real g, real b);
	static Color3 setPixel(real gammaFrom, real invGammaDest, Channel channel, real r, real g, real b, real a);

protected:
	/// Hash function
	//struct ImageLoaderHash : std::unary_function<ImageLoaderHash, size_t> {
	//	std::size_t operator()(const ImageLoader& im) {
	static std::size_t hash(const std::string& path, Channel channel) {
		size_t hash = std::hash<std::string>()(path);
		hash = hash * 37 + std::hash<uint32_t>()(channel);
		return hash;
	}


protected:
	static CacheMap myCache;
	//Array2D<Color> myArray;
};

