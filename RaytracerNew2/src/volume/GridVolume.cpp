#include "GridVolume.h"

#include "tools/Common.h"
#include "factory/ObjectFactoryManager.h"


GridVolume::GridVolume(const Parameters& params)
:Volume(params)
{
	std::string path = ::getGlobalFileSystem().resolve(params.getString("path", "")).string();

	std::ifstream file(path, std::ios::binary);

	if (!file)
		ILogger::log(ILogger::ERRORS) << "The file " << path << "does not exist!\n";

	///* Parse the file header */
	//file.open(QIODevice::ReadOnly);
	//QDataStream stream(&file);
	//stream.setByteOrder(QDataStream::LittleEndian);

	int8_t header[3], version; int32_t type;
	file.read(reinterpret_cast<char*>(&header[0]), sizeof(header[0]));
	file.read(reinterpret_cast<char*>(&header[1]), sizeof(header[1]));
	file.read(reinterpret_cast<char*>(&header[2]), sizeof(header[2]));
	file.read(reinterpret_cast<char*>(&version), sizeof(version));
	file.read(reinterpret_cast<char*>(&type), sizeof(type));

	if (memcmp(header, "VOL", 3) != 0 || version != 3)
		ILogger::log(ILogger::ERRORS) << "This is not a valid volume data file!";

	int resX, resY, resZ;
	file.read(reinterpret_cast<char*>(&resX), sizeof(resX));
	file.read(reinterpret_cast<char*>(&resY), sizeof(resY));
	file.read(reinterpret_cast<char*>(&resZ), sizeof(resZ));
	myResolution = Point3i(resX, resY, resZ);
	int dataType;
	file.read(reinterpret_cast<char*>(&dataType), sizeof(dataType));
	
	float xmin, ymin, zmin, xmax, ymax, zmax;
	file.read(reinterpret_cast<char*>(&xmin), sizeof(float));
	file.read(reinterpret_cast<char*>(&ymin), sizeof(float));
	file.read(reinterpret_cast<char*>(&zmin), sizeof(float));
	file.read(reinterpret_cast<char*>(&xmax), sizeof(float));
	file.read(reinterpret_cast<char*>(&ymax), sizeof(float));
	file.read(reinterpret_cast<char*>(&zmax), sizeof(float));
	Point3d minBB(xmin, ymin, zmin);
	Point3d maxBB(xmax, ymax, zmax);

	myBoundingBox = BoundingBox(minBB, maxBB);

	//
	//file.close();
	char a;
	for (int i = 0; i < 4; i++)
		file.read(&a, 1);

	myData = new float[myResolution.x() * myResolution.y() * myResolution.z()];
	file.read(reinterpret_cast<char*>(myData), myResolution.x() * myResolution.y() * myResolution.z() * sizeof(float));

	file.seekg(std::ios::end);
	size_t fileSize = (size_t)file.tellg();
	//std::cout << fileSize << std::endl;

	std::cout << "Mapping \"" << path.data() << "\" (" << myResolution.x()
		<< "x" << myResolution.y() << "x" << myResolution.z() << ") into memory ..bbox(" << minBB << " " << maxBB << std::endl;

	//#if defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
	//	int fd = open(path.data(), O_RDONLY);
	//	if (fd == -1)
	//		throw NoriException(QString("Could not open \"%1\"!").arg(m_filename));
	//	m_data = (float *)mmap(NULL, m_fileSize, PROT_READ, MAP_SHARED, fd, 0);
	//	if (m_data == NULL)
	//		throw NoriException("mmap(): failed.");
	//	if (close(fd) != 0)
	//		throw NoriException("close(): unable to close file descriptor!");
	//#elif defined(WIN32)
	//	myHfile = CreateFileA(path.data(), GENERIC_READ,
	//		FILE_SHARE_READ, NULL, OPEN_EXISTING,
	//		FILE_ATTRIBUTE_NORMAL, NULL);
	//	if (myHfile == INVALID_HANDLE_VALUE)
	//		ILogger::log(ILogger::ERRORS) << "Could not open " << path << "\n";;
	//	myFileMapping = CreateFileMapping(myHfile, NULL, PAGE_READONLY, 0, 0, NULL);
	//	if (myFileMapping == NULL)
	//		ILogger::log(ILogger::ERRORS) << "CreateFileMapping(): failed.\n";
	//	myData = (float *)MapViewOfFile(myFileMapping, FILE_MAP_READ, 0, 0, 0);
	//	if (myData == NULL)
	//		std::cout << "error" << std::endl;
	//		//ILogger::log(ILogger::ERRORS) << "MapViewOfFile(): failed.";
	//#endif
	//
	//	myData += 12; // Shift past the header
}


GridVolume::~GridVolume()
{
	delete[] myData;
}

const BoundingBox& GridVolume::getBoundingBox() const
{
	return myBoundingBox;
}

real GridVolume::getMaxSigmaT() const
{
	float maxSigmaT = myData[0];
	for (int i = 1; i < myResolution.x() * myResolution.y() * myResolution.z(); i++) 
	{
		if (myData[i] > maxSigmaT)
			maxSigmaT = myData[i];
	}

	return (real)maxSigmaT;
}

real GridVolume::lookupSigmaT(const Point3d &_p) const
{
	Point3d p = _p.cwiseProduct(myResolution.cast<real>()),
		pf = Point3d(std::floor(p.x()), std::floor(p.y()), std::floor(p.z()));
	Point3i p0 = pf.cast<int>();

	if ((p0.array() < 0).any() || (p0.array() >= myResolution.array() - 1).head<3>().any())
		return 0.0f;

	size_t row = myResolution.x(),
		slab = row * myResolution.y(),
		offset = p0.z()*slab + p0.y()*row + p0.x();

	const float
		d000 = myData[offset],
		d001 = myData[offset + 1],
		d010 = myData[offset + row],
		d011 = myData[offset + row + 1],
		d100 = myData[offset + slab],
		d101 = myData[offset + slab + 1],
		d110 = myData[offset + slab + row],
		d111 = myData[offset + slab + row + 1];

	Vector3d w1 = p - pf, w0 = (1 - w1.array()).matrix();

	//std::cout << offset << " " << d000 << " " << d001 << " " << w0 << " " << w1 << std::endl;
	/* Trilinearly interpolate */
	return (real)(((d000 * w0.x() + d001 * w1.x()) * w0.y() +
		(d010 * w0.x() + d011 * w1.x()) * w1.y()) * w0.z() +
		((d100 * w0.x() + d101 * w1.x()) * w0.y() +
		(d110 * w0.x() + d111 * w1.x()) * w1.y()) * w1.z());
}

RT_REGISTER_TYPE(GridVolume, Volume)