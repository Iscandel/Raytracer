#include <SFML/Graphics.hpp>
#include "Logger.h"
#include "Scene.h"
#include "Timer.h"

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
	shader.setParameter("gamma", std::pow(2.f, (0.5 - 0.5f) * 20));
}

void toneMapper(Screen& film, sf::Image& out, double gamma)
{
	auto toSRGB = [&](double value) {
		if (value < 0.0031308)
			return 12.92 * value;
		return 1.055 * pow(value, 0.41666) - 0.055;
	};

	//double gamma = std::pow(2., (0.5 - 0.5) * 20);

	for (unsigned int y = 0; y < out.getSize().y; y++)
	{
		for (unsigned int x = 0; x < out.getSize().x; x++)
		{
			Color col = film(x, y) * gamma;
			col.r = toSRGB(col.r); col.g = toSRGB(col.g); col.b = toSRGB(col.b);
			out.setPixel(x, y, sf::Color(tools::thresholding(col.r * 255, 0., 255.), tools::thresholding(col.g * 255, 0., 255.), tools::thresholding(col.b * 255, 0., 255.)));
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
			out.setPixel(x, y, sf::Color(tools::thresholding(col.r * 255, 0., 255.), tools::thresholding(col.g * 255, 0., 255.), tools::thresholding(col.b * 255, 0., 255.)));
		}
	}
}

void applyProcessing(bool toneMap, Screen& filmOrig, sf::Image& image, double gamma)
{
	if (toneMap)
	{
		toneMapper(filmOrig, image, gamma);
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

	for (unsigned int y = 0; y < height; y++)
	{
		for (unsigned int x = 0; x < width; x++)
		{
			Color col = film(x, y);
			Imf_2_2::Rgba tmp((half)col.r, (half)col.g, (half)col.b);
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



int main(int argc, char* argv[])
{
	try {
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
	}
	catch(...)
	{
		ILogger::log() << "Unknown error. Program aborted. \n";
	}

	return 0;
}

void run(int argc, char* argv[])
{
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
	double gamma = std::pow(2., (0.5 - 0.5) * 20);

	applyProcessing(toneMap, filmOrig, image, gamma);
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

	DisplayableText gammaText(0.f, 0.f);
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

					applyProcessing(toneMap, filmOrig, image, gamma);

					texture.loadFromImage(image);
					sp.setTexture(texture);

					if (toneMap)
						toneMapText.setText("Tone map ON");
					else
						toneMapText.setText("Tone map OFF");
				}
				else if (ev.key.code == sf::Keyboard::Return)
				{
					applyProcessing(toneMap, filmOrig, image, gamma);
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
				if (ev.text.unicode == '+')
				{
					gamma += 0.1;
					gammaText.setText("gamma = " + tools::numToString(gamma));
				}
				else if (ev.text.unicode == '-')
				{
					gamma -= 0.1;
					gammaText.setText("gamma = " + tools::numToString(gamma));
				}
			}
		}

		//update
		gammaText.update();
		toneMapText.update();

		//draw
		window.clear();
	
		window.draw(sp);
		gammaText.show(window);
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