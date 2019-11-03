#include <regex>
#include <iostream>

//inline std::vector<std::string> regexSplit(const std::string& s, const std::string& regex)
//{
//	std::vector<std::string> res;
//	std::regex words_regex(regex);//"[^\\s.,:;!?]+");
//	auto words_begin = std::sregex_iterator(s.begin(), s.end(), words_regex);
//	auto words_end = std::sregex_iterator();
//
//	for (std::sregex_iterator i = words_begin; i != words_end; ++i)
//		res.push_back((*i).str());
//	return res;
//}
//
//int main(int argc, char* argv[])
//{
//	//auto vec = regexSplit("200.57", "(\\.[0-9]+)");
//	//auto vec = regexSplit("200:3, 300:5,350:4 400:4, 450:7, 500: 6 550 : 7 600 :  8", "([0-9]+\\s*:\\s*[0-9]+)");
//	auto vec = regexSplit("200:3, 300:5,350:4 400:4, 450:7, 500: 6 550 : 7 600 :  8", "([0-9]+\\.?[0-9]*\\s*:\\s*[0-9]+\\.?[0-9]*)|([^\\s,]+)");
//	for (auto tmp : vec)
//		std::cout << tmp << std::endl;
//	std::cout << "New\n" << std::endl;
//	vec = regexSplit("3 5 7", "([0-9]+\\.?[0-9]*\\s*:\\s*[0-9]+\\.?[0-9]*)|([^\\s,]+)");
//	for (auto tmp : vec)
//		std::cout << tmp << std::endl;
//	std::cout << "New\n" << std::endl;
//	vec = regexSplit("3, 8, 2 7 9", "([0-9]+\\.?[0-9]*\\s*:\\s*[0-9]+\\.?[0-9]*)|([^\\s,]+)");
//	//auto vec = tools::regexSplit("200:3, 300:5,350:4 400:4, 450:7, 500: 6 550 : 7 ", "[^\\s,]+");
//	//auto vec = tools::regexSplit("Hello, everyone! This is: COSC-1436, SP18", "[^\\s.,:;!?]+");
//	for (auto tmp : vec)
//		std::cout << tmp << std::endl;
//	std::cout << "New2\n" << std::endl;
//	vec = regexSplit("200:3, 300:5,350:4 400:4, 450:7, 500: 6 550 : 7 600 :  8", "([^\\s,]+\\.*[^,]+\\s*:\\s*[^,]+\\.*[^\\s,]+)|([^,]+)");
//	for (auto tmp : vec)
//		std::cout << tmp << std::endl;
//	std::cout << "New2\n" << std::endl;
//	vec = regexSplit("3, 8, 2 7 9", "([^\\s,]+\\s*:\\s*[^\\s,]+)|([^\\s,]+)");
//	for (auto tmp : vec)
//		std::cout << tmp << std::endl;
//	std::cout << "New2\n" << std::endl;
//	vec = regexSplit("2003, 300.5.1:5,350.5f:4 400:4.9de, 450:7, 500: 6 550 : 7 600 :  8", "([^\\s,]+\\s*:\\s*[^\\s,]+)|([^\\s,]+)");
//	for (auto tmp : vec)
//		std::cout << tmp << std::endl;
//	getchar();
//	return 0;
//}

#include <SFML/Graphics.hpp>
#include "tools/Logger.h"
#include "core/Math.h"
#include "core/Scene.h"
#include "tools/Timer.h"
#include "tools/Tools.h"

//#ifdef _DEBUG 
//#pragma comment(lib,"sfml-graphics-d.lib")
//#pragma comment(lib,"sfml-window-d.lib")
//#pragma comment(lib,"sfml-system-d.lib")
//#pragma comment(lib,"sfml-main-d.lib")
////#pragma comment(lib,"sfml-network-s-d.lib")
////#pragma comment(lib,"sfml-audio-s-d.lib")
////#pragma comment(lib,"tinyxmld_STL.lib")
//#else
//#pragma comment(lib,"sfml-graphics.lib")
//#pragma comment(lib,"sfml-window.lib")
//#pragma comment(lib,"sfml-system.lib")
////#pragma comment(lib,"sfml-network-s.lib")
////#pragma comment(lib,"sfml-audio-s.lib")
////#pragma comment(lib,"tinyxml_STL.lib")
//#pragma comment(lib,"sfml-main.lib")
////#pragma comment(lib,"opengl32.lib")
//#endif

//
#include <ImfRgbaFile.h>
#include <ImfRgba.h>

void run(int argc, char* argv[]);

//#include "RandomSampler.h"

	//Gamma tonemapper
void shToneMapper()
{
	sf::Shader shader;
	std::string vertexShader =
		"#version 330\n"
		"in vec2 position;\n"
		"out vec2 uv;\n"
		"void main() {\n"
		"    gl_Position = vec4(position.x*2-1, position.y*2-1, 0.0, 1.0);\n"
		"    uv = vec2(position.x, 1-position.y);\n"
		"}";

	std::string fragmentShader =
		"#version 330\n"
		"uniform sampler2D source;\n"
		"uniform float gamma;\n"
		"in vec2 uv;\n"
		"out vec4 out_color;\n"
		"float toSRGB(float value) {\n"
		"    if (value < 0.0031308)\n"
		"        return 12.92 * value;\n"
		"    return 1.055 * pow(value, 0.41666) - 0.055;\n"
		"}\n"
		"void main() {\n"
		"    vec4 color = texture(source, uv);\n"
		"    color *= gamma / color.w;\n"
		"    out_color = vec4(toSRGB(gl_color.r), toSRGB(gl_color.g), toSRGB(gl_color.r.b), 1);\n"
		"}";

	//Eigen::MatrixXi indices(3, 2); /* Draw 2 triangles */
	//indices.col(0) << 0, 1, 2;
	//indices.col(1) << 2, 3, 0;

	//Eigen::MatrixXf positions(2, 4);
	//positions.col(0) << 0, 0;
	//positions.col(1) << 1, 0;
	//positions.col(2) << 1, 1;
	//positions.col(3) << 0, 1;

	//shader.setParameter("position", positions);
	shader.setParameter("source", 0);
	shader.setParameter("gamma", std::pow(2.f, (0.5f - 0.5f) * 20));
}

// Tone mapper based on sRGB and a scale factor for adjust f-stops
void toneMapper(Screen& film, sf::Image& out, real exposure, real gamma)
{
	auto toSRGB = [&](real value) {
		if (value < 0.0031308)
			return 12.92f * value;
		return 1.055f * pow(value, 0.41666f) - 0.055f;
	};

	exposure = std::pow(2., (exposure - 0.5)* 20);

	for (unsigned int y = 0; y < out.getSize().y; y++)
	{
		for (unsigned int x = 0; x < out.getSize().x; x++)
		{
			Color col = film(x, y) * exposure;
			real r, g, b;
			
			if (gamma == -1.)
				col.toSRGB(r, g, b);
			else if (gamma != 1.) {
				col.toRGB(r, g, b);
				real invGamma = 1. / gamma;
				r = std::pow(r, invGamma);
				g = std::pow(g, invGamma);
				b = std::pow(b, invGamma);
			} else
				col.toRGB(r, g, b);

			//col.r() = toSRGB(col.r()); col.g() = toSRGB(col.g()); col.b() = toSRGB(col.b());
			out.setPixel(x, y, sf::Color((sf::Uint8)math::thresholding(r * 255, (real) 0., (real) 255.),
										 (sf::Uint8)math::thresholding(g * 255, (real) 0., (real) 255.),
										 (sf::Uint8)math::thresholding(b * 255, (real) 0., (real) 255.)));
		}
	}
}

void EXRToRGB(Screen& film, sf::Image& out)
{
	for (unsigned int y = 0; y < out.getSize().y; y++)
	{
		for (unsigned int x = 0; x < out.getSize().x; x++)
		{
			Color col = film(x, y);
			real r, g, b;
			col.toRGB(r, g, b);
			out.setPixel(x, y, sf::Color(math::thresholding(r * 255, (real) 0., (real) 255.),
										 math::thresholding(g * 255, (real) 0., (real) 255.),
										 math::thresholding(b * 255, (real) 0., (real) 255.)));
		}
	}
}

void applyProcessing(bool toneMap, Screen& filmOrig, sf::Image& image, real exposure)
{
	if (toneMap)
	{
		toneMapper(filmOrig, image, exposure, -1.);
	}
	else
	{
		EXRToRGB(filmOrig, image);
	}
}

void writeEXR(const std::string& path, const Screen& film) //const Array2D<Pixel>& array)
{
	int width = film.getSizeX();//.getWidth();
	int height = film.getSizeY();//array.getHeight();
	Imf_2_2::Rgba* pixels = new Imf_2_2::Rgba[width * height];

	if (Color::NB_SAMPLES != 3)
		ILogger::log() << "Spectral EXR not implemented yet. Switch to RGB\n";
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			Color col = film(x, y);
			
			real r, g, b;
			col.toRGB(r, g, b);
			Imf_2_2::Rgba tmp((half)r, (half)g, (half)b);
			*(pixels + x + y * width) = tmp;
		}
	}

	Imf_2_2::RgbaOutputFile file(path.c_str(), width, height, Imf_2_2::WRITE_RGBA);
	file.setFrameBuffer(pixels, 1, width);
	file.writePixels(height);

	delete[] pixels;
}

class DisplayableText
{
public:
	DisplayableText(float x, float y)
	{
		myIsShown = false;
		myFont.loadFromFile("./PORKYS_.TTF");
		myText.setFont(myFont);
		myText.setPosition(x, y);
		myText.setColor(sf::Color::White);
		
	}

	//DisplayableText(const std::string& text)
	//{
	//	setText(text);
	//}

	void setText(const std::string& text)
	{
		myIsShown = true;
		myText.setString(text);
		myTimer.reset();
	}

	void show(sf::RenderWindow& window)
	{
		if (myIsShown)
		{
			window.draw(myText);			
		}
	}

	void update()
	{
		if (myTimer.elapsedTime() > 1.5)
			myIsShown = false;
	}

protected:
	sf::Text myText;
	sf::Font myFont;
	Timer myTimer;
	bool myIsShown;
};

class Col2
{
	float a; 
	float b;
	float c;
	float d;
	int e;
};

template<class T>
class Col : public Eigen::Array<T, 5, 1>/*public DiscreteSpectrum,*/ //public Eigen::Array<T, DiscreteSpectrum::NB_SAMPLES + 1, 1>//Color3<T>::nbSamples, 1>
{
protected:
	//#ifdef USE_ALIGN
	//	static constexpr int ALIGNED_SAMPLES = 4;//DiscreteSpectrum::NB_SAMPLES + 1;
	//#else
	//	static constexpr int ALIGNED_SAMPLES = 3;//DiscreteSpectrum::NB_SAMPLES;
	//#endif

//public:
//	explicit Col(T val = 0.)
//		:r(coeffRef(0))
//		, g(coeffRef(1))
//		, b(coeffRef(2))
//	{
//		Eigen::Array<T, 4, 1>::setConstant(val);
//	}
//
//	Color3(T x, T y, T z, T w = (T)0)
//		: Eigen::Array<T, 4, 1>(x, y, z, w)
//		, r(coeffRef(0))
//		, g(coeffRef(1))
//		, b(coeffRef(2))
//	{
//	}
//
//	Col(const Color3<T>& c)
//		:r(coeffRef(0))
//		, g(coeffRef(1))
//		, b(coeffRef(2))
//	{
//		r = c.r;
//		g = c.g;
//		b = c.b;
//	}
//
//	Color3& operator = (const Color3<T>& c) {
//		coeffRef(0) = c.coeff(0);
//		coeffRef(1) = c.coeff(1);
//		coeffRef(2) = c.coeff(2);
//
//		return *this;
//	}
//
//	template <typename Derived> Color3(const Eigen::ArrayBase<Derived>& p)
//		: Eigen::Array<T, 4, 1>(p)
//		, r(coeffRef(0))
//		, g(coeffRef(1))
//		, b(coeffRef(2))
//	{ }
//
//	template <typename Derived> Color3 &operator=(const Eigen::ArrayBase<Derived>& p) {
//		this->Base::operator=(p);
//		return *this;
//	}
//
//	void validate()
//	{
//		r = r > 1.f ? 1.f : r < 0. ? 0.f : r;
//		g = g > 1.f ? 1.f : g < 0. ? 0.f : g;
//		b = b > 1.f ? 1.f : b < 0. ? 0.f : b;
//	}
//
//	T average() const
//	{
//		return (r + b + g) / (T)3.;
//	}
//
//	T max()
//	{
//		return std::max(r, std::max(g, b));
//	}
//
//	T luminance() const
//	{
//		return 0.2126f * r + 0.7152f * g + 0.0722f *b;
//	}
//
//	bool isZero() const
//	{
//#ifdef DOUBLE_PRECISION
//		const priv::real eps = 1e-9;
//#else
//		const priv::real eps = 1e-5f;
//#endif
//		return r < eps && g < eps && b < eps;
//	}
//
	//bool isNan() const
	//{
	//	if (std::isnan(r) || std::isnan(g) || std::isnan(b))
	//		return true;
	//	return false;
	//}

	//T& r;
	//T& g;
	//T& b;
};



int main(int argc, char* argv[])
{
	try {
		////Point3d p(0.1, 0.2, 0.3);
		////Point3d resP;
		////Eigen::Array3f f;
		////Eigen::Array3f t = f + 3.2;

		//Color c(0.5, 0.2, 0.3);
		//Color res = Color(1.) / c;
		//res = c + 0.2;
		//res = c + 2;
		//auto prod = c * res;
		////res -= 2.;

		//Color3f d(0.2, 0.3, 0.4);
		//Color3f res2 = d + 0.2;
		//res2 = d + 2;
		// res2 *= 3.f;
		// res2 = 3. * d;
		// //res2 -= 2.;
		//res2 /= 3.f;
		//res2 += 0.6;
		//auto prod2 = d * res2;
		//Color3f prod3 = d * res2;
		//std::cout << prod2 << " ||" << prod3 << std::endl;
		//Color3f tmp;
		//std::cout << sizeof(int) << " " << sizeof(double) << " " << sizeof(float) << " " << sizeof(char) << std::endl;
		//std::cout << sizeof(Color3f) << " " << std::endl;
		//Color3f c;c.exp();
		//(?!chat|?!chien)([0-9A-Za-z-]{3,})
		//auto vec = tools::regexSplit("200:3, 300:5,350:4 400:4, 450:7, 500: 6 550 : 7 ", "([[0-9]+\\s*:\\s*.+]+)([^\\s,]+)");
		//auto vec = tools::regexSplit("200:3, 300:5,350:4 400:4, 450:7, 500: 6 550 : 7 ", "([0-9]+\\.?[0-9]+\\s*:\\s*[0-9]+\.?[0-9]+)");
		////auto vec = tools::regexSplit("200:3, 300:5,350:4 400:4, 450:7, 500: 6 550 : 7 ", "[^\\s,]+");
		////auto vec = tools::regexSplit("Hello, everyone! This is: COSC-1436, SP18", "[^\\s.,:;!?]+");
		//for (auto tmp : vec)
		//	std::cout << tmp << std::endl;
		//std::cout << sizeof(Col<real>) <<  std::endl;
		//Point3d p;
		//std::cout << sizeof(p) << std::endl;
		//std::cout << sizeof(Col2) << std::endl;

		ILogger::setLogLevel(ILogger::ALL);
		ILogger::setLogger(new ConsoleLog(std::cout));
		//Sampler::ptr sampler(new RandomSampler(Parameters()));
		//std::cout << sampler->getNextSample1D() << std::endl;
		//Sampler::ptr s2 = sampler->clone();
		//std::cout << s2->getNextSample1D() << std::endl;
		//std::cout << sampler->getNextSample1D() << std::endl;
		//Rng rng;
		//for (int i = 0; i < 100; i++)
		//	std::cout << rng.random(0., 1.) << std::endl;
		//Test t;
		//t.test();
		run(argc, argv);
	}
	catch(std::exception& e)
	{
		ILogger::log() << e.what() << "\n";
		getchar();
	}
	catch(...)
	{
		ILogger::log() << "Unknown error. Program aborted. \n";
	}

	return 0;
}
#include "tools/MitsubaConverter.h"
void run(int argc, char* argv[])
{
	std::string fileIn = "./livingRoom.xml";
	std::string fileOut = "./testConversion.xml";
	MitsubaConverter converter;
	//converter.convert(fileIn, fileOut);
	std::string filePath;
	if (argc > 1)
	{
		filePath = argv[0];
	}
	else
	{
		std::ifstream file("./scene.txt");
		if (!file)
		{
			filePath = "./Scene1.xml";
			ILogger::log() << "Default file path : " << filePath << "\n";
		}
		else
		{
			std::getline(file, filePath);
		}
	}

	Scene scene;
	Timer clock;

	scene.compute(filePath);

	//int remaining = 0;
	
	//while(scene.myManager.remainingTasks() > 0)
	//{
	//	int tmp = scene.myManager.remainingTasks();
	//	if(tmp != remaining)
	//	{
	//		ILogger::log() << "Remaining task " << tmp <<"\n";
	//
	//		remaining = tmp;
	//	}
	//}

	//Parameters p;
	//p.addReal("persistence", 0.4f);
	//p.addInt("octaves", 4);
	//p.addReal("scale", 1.f);
	//p.addColor("color1", Color::fromRGB(0., 0, 0));
	//p.addColor("color2", Color::fromRGB(1., 1, 1));
	//int width = 500;
	//int height = 500;
	//PerlinNoiseTexture _tex(p);_tex.eval(Point2d());
	//Screen s(width, height, 0, 0, ReconstructionFilter::ptr(new BoxFilter(1, 1)));
	//for (int i = 0; i < width; i++)
	//	for (int j = 0; j < height; j++)
	//		s.addSample(i, j, _tex.eval(Point2d(i / (real)width, j / (real)height)));
	//s.postProcessColor();

	ILogger::log() << "Scene computed\n";
#ifdef VS_2010
	ILogger::log() << "Elapsed time " << clock.GetElapsedTime() / 1000. << "\n";
#else
	ILogger::log() << "Elapsed time " << clock.elapsedTime() << "\n";
#endif
	int width = scene.getCamera().getSizeX();
	int height = scene.getCamera().getSizeY();
	sf::Image image;
#ifdef VS_2010
	image.Create(width, height, sf::Color::Black);
#else
	image.create(width, height, sf::Color::Black);
#endif
	sf::RenderWindow window(sf::VideoMode(width, height), "");

	Screen& filmOrig = scene.getScreen();
	//Array2D<Pixel> film = filmOrig.getPixels();
	filmOrig.postProcessColor();

	bool toneMap = false;
	real exposure = real(0.5);//std::pow(2.f, (0.5f - 0.5f) * 20);

	applyProcessing(toneMap, filmOrig, image, exposure);
//	for(int y = 0; y < height; y++)
//	{
//		for(int x = 0; x < width; x++)
//		{			
//			sf::Color col((sf::Uint8) filmOrig(x, y).r, (sf::Uint8) filmOrig(x, y).g, (sf::Uint8) filmOrig(x, y).b);
//#ifdef VS_2010
//			image.SetPixel(x, y, col);
//#else
//			image.setPixel(x, y, col);
//#endif
//		}
//	}

	//toneMapper(film, image);

	window.setFramerateLimit(60);
	window.setKeyRepeatEnabled(true);

	sf::Sprite sp;
	sf::Texture texture;
	texture.loadFromImage(image);
	sp.setTexture(texture);

	if (scene.getFileName() != "")
	{
		std::string fileName = scene.getFileName();
		image.saveToFile(fileName);
		writeEXR(fileName.substr(0, fileName.size() - 4) + ".exr", filmOrig);
	}

	DisplayableText exposureText(0.f, 0.f);
	DisplayableText toneMapText(width / 2.f, height / 2.f);//height);

	while (window.isOpen())
	{
		sf::Event ev;

		while (window.pollEvent(ev))
		{
			if (ev.type == sf::Event::Closed)
				window.close();
			else if (ev.type == sf::Event::MouseButtonReleased)
				std::cout << ev.mouseButton.x << " " << ev.mouseButton.y << std::endl;
			else if (ev.type == sf::Event::KeyReleased)
			{
				if (ev.key.code == sf::Keyboard::T)
				{
					toneMap = !toneMap;

					applyProcessing(toneMap, filmOrig, image, exposure);

					texture.loadFromImage(image);
					sp.setTexture(texture);

					if (toneMap)
						toneMapText.setText("Tone map ON");
					else
						toneMapText.setText("Tone map OFF");
				}
				else if (ev.key.code == sf::Keyboard::Return)
				{
					applyProcessing(toneMap, filmOrig, image, exposure);
					texture.loadFromImage(image);
					sp.setTexture(texture);
				}
				else if (ev.key.code == sf::Keyboard::S)
				{
					std::string fileName = scene.getFileName();
					std::size_t found = fileName.find_last_of(".");
					if (found != std::string::npos)
					{
						std::string extension = fileName.substr(found);
						fileName = fileName.substr(0, found) + "_2" + extension;
						image.saveToFile(fileName);
					}
				}

			}
			else if (ev.type == sf::Event::TextEntered)
			{
				real step = real(0.05);
				if (ev.text.unicode == '+')
				{
					exposure += step;
					exposureText.setText("exposure = " + tools::numToString(exposure));
				}
				else if (ev.text.unicode == '-')
				{
					exposure -= step;
					exposureText.setText("exposure = " + tools::numToString(exposure));
				}
			}
		}

		//update
		exposureText.update();
		toneMapText.update();

		//draw
		window.clear();
	
		window.draw(sp);
		exposureText.show(window);
		toneMapText.show(window);

		window.display();
	}
}


/*
#include <stdlib.h>   // card > aek.ppm
    #include <stdio.h>
    #include <math.h>
    typedef int i;typedef float f;struct v{
    f x,y,z;v operator+(v r){return v(x+r.x
    ,y+r.y,z+r.z);}v operator*(f r){return
    v(x*r,y*r,z*r);}f operator%(v r){return
    x*r.x+y*r.y+z*r.z;}v(){}v operator^(v r
    ){return v(y*r.z-z*r.y,z*r.x-x*r.z,x*r.
    y-y*r.x);}v(f a,f b,f c){x=a;y=b;z=c;}v
    operator!(){return*this*(1/sqrt(*this%*
    this));}};i G[]={247570,280596,280600,
    249748,18578,18577,231184,16,16};f R(){
    return(f)rand()/RAND_MAX;}i T(v o,v d,f
    &t,v&n){t=1e9;i m=0;f p=-o.z/d.z;if(.01
    <p)t=p,n=v(0,0,1),m=1;for(i k=19;k--;)
    for(i j=9;j--;)if(G[j]&1<<k){v p=o+v(-k
    ,0,-j-4);f b=p%d,c=p%p-1,q=b*b-c;if(q>0
    ){f s=-b-sqrt(q);if(s<t&&s>.01)t=s,n=!(
    p+d*t),m=2;}}return m;}v S(v o,v d){f t
    ;v n;i m=T(o,d,t,n);if(!m)return v(.7,
    .6,1)*pow(1-d.z,4);v h=o+d*t,l=!(v(9+R(
    ),9+R(),16)+h*-1),r=d+n*(n%d*-2);f b=l%
    n;if(b<0||T(h,l,t,n))b=0;f p=pow(l%r*(b
    >0),99);if(m&1){h=h*.2;return((i)(ceil(
    h.x)+ceil(h.y))&1?v(3,1,1):v(3,3,3))*(b
    *.2+.1);}return v(p,p,p)+S(h,r)*.5;}
	i main()
	{
		const int larg = 512;
		const int haut = 512;
		sf::Image image;
		image.Create(larg, haut, sf::Color::Black);
		sf::RenderWindow Fenetre(sf::VideoMode(larg, haut),"");
		printf("P6 512 512 255 ");
		v g=!v(-6,-16,0),a=!(v(0,0,1)^g)*.002,b=!(g^a
    )*.002,c=(a+b)*-256+g;
		for(i y=haut;y--;)
			for(i x=larg;x--;)
			{
				v p(13,13,13);
				for(i r=64;r--;)
				{
					v t=a*(R()-.5)*99+b*(R()-.5)*
    99;p=S(v(17,16,8)+t,!(t*-1+(a*(R()+x)+b
    *(y+R())+c)*16))*3.5+p;
				}
	printf("x%d y%d% c%c%c",x, y, (i)p.x,(i)p.y,(i)p.z);
	sf::Color col(p.x, p.y,(i)p.z);
	image.SetPixel(larg-x - 1, haut-y - 1, col);
			}

	while(Fenetre.IsOpened())
	{
		sf::Event ev;
		while(Fenetre.PollEvent(ev))
		{
			if(ev.Type == sf::Event::Closed)
				Fenetre.Close();
		}

		Fenetre.Clear();

		sf::Sprite sp;
		sf::Texture texture;
		texture.LoadFromImage(image);
		sp.SetTexture(texture);//MaScene.getImage());
		Fenetre.Draw(sp);

		Fenetre.Display();
	}
	}
	*/