#include <SFML/Graphics.hpp>
#include "Logger.h"
#include "Scene.h"
#include "Timer.h"

#ifdef _DEBUG 
#pragma comment(lib,"sfml-graphics-d.lib")
#pragma comment(lib,"sfml-window-d.lib")
#pragma comment(lib,"sfml-system-d.lib")
#pragma comment(lib,"sfml-main-d.lib")
//#pragma comment(lib,"sfml-network-s-d.lib")
//#pragma comment(lib,"sfml-audio-s-d.lib")
//#pragma comment(lib,"tinyxmld_STL.lib")
#else
#pragma comment(lib,"sfml-graphics.lib")
#pragma comment(lib,"sfml-window.lib")
#pragma comment(lib,"sfml-system.lib")
//#pragma comment(lib,"sfml-network-s.lib")
//#pragma comment(lib,"sfml-audio-s.lib")
//#pragma comment(lib,"tinyxml_STL.lib")
#pragma comment(lib,"sfml-main.lib")
//#pragma comment(lib,"opengl32.lib")
#endif

//


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

void toneMapper(Screen& film, sf::Image& out)
{
	auto toSRGB = [&](double value) {
		if (value < 0.0031308)
			return 12.92 * value;
		return 1.055 * pow(value, 0.41666) - 0.055;
	};

	double gamma = std::pow(2., (0.5 - 0.5) * 20);

	for (int y = 0; y < out.getSize().y; y++)
	{
		for (int x = 0; x < out.getSize().x; x++)
		{
			Color col = film(x, y) * gamma;
			col.r = toSRGB(col.r); col.g = toSRGB(col.g); col.b = toSRGB(col.b);
			out.setPixel(x, y, sf::Color(tools::thresholding(col.r * 255, 0., 255.), tools::thresholding(col.g * 255, 0., 255.), tools::thresholding(col.b * 255, 0., 255.)));
			if (x == 600 && y == 130)
				std::cout <<(int) out.getPixel(x, y).r << " " << (int)out.getPixel(x, y).g << " " << (int)out.getPixel(x, y).b;
		}
	}
}


//#include <ImfInputFile.h>
//#include <ImfOutputFile.h>
//#include <ImfChannelList.h>
//#include <ImfStringAttribute.h>
//#include <ImfVersion.h>
//#include <ImfIO.h>


int main(int argc, char* argv[])
{
	//Imf::InputFile file("");
	//const Imf::Header &header = file.header();
	//const Imf::ChannelList &channels = header.channels();

	////Imath::Box2i dw = file.header().dataWindow();
	////std::resize(dw.max.y - dw.min.y + 1, dw.max.x - dw.min.x + 1);

	////std::cout << "Reading a " << cols() << "x" << rows() << " OpenEXR file from \""
	////	<< filename << "\"" << endl;

	//const char *ch_r = nullptr, *ch_g = nullptr, *ch_b = nullptr;
	//for (Imf::ChannelList::ConstIterator it = channels.begin(); it != channels.end(); ++it) {
	//	std::string name = "";//std::tolower(it.name());

	//	if (it.channel().xSampling != 1 || it.channel().ySampling != 1) {
	//		/* Sub-sampled layers are not supported */
	//		continue;
	//	}

	//	if (!ch_r && (name == "r" || name == "red" //||
	//		/*endsWith(name, ".r") || endsWith(name, ".red")*/)) {
	//		ch_r = it.name();
	//	}
	//	else if (!ch_g && (name == "g" || name == "green" //||
	//		/*endsWith(name, ".g") || endsWith(name, ".green")*/)) {
	//		ch_g = it.name();
	//	}
	//	else if (!ch_b && (name == "b" || name == "blue" //||
	//		/*endsWith(name, ".b") || endsWith(name, ".blue")*/)) {
	//		ch_b = it.name();
	//	}
	//}

	//size_t compStride = sizeof(float),
	//	pixelStride = 0,//3 * compStride,
	//rowStride = 0;//pixelStride * cols();

	//char *ptr = nullptr;//reinterpret_cast<char *>(data());

	//Imf::FrameBuffer frameBuffer;
	//frameBuffer.insert(ch_r, Imf::Slice(Imf::FLOAT, ptr, pixelStride, rowStride)); ptr += compStride;
	//frameBuffer.insert(ch_g, Imf::Slice(Imf::FLOAT, ptr, pixelStride, rowStride)); ptr += compStride;
	//frameBuffer.insert(ch_b, Imf::Slice(Imf::FLOAT, ptr, pixelStride, rowStride));
	//file.setFrameBuffer(frameBuffer);
	////file.readPixels(dw.min.y, dw.max.y);

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

	int remaining = 0;
	
	while(scene.myManager.remainingTasks() > 0)
	{
		int tmp = scene.myManager.remainingTasks();
		if(tmp != remaining)
		{
			ILogger::log() << "Remaining task " << tmp <<"\n";
	
			remaining = tmp;
		}
	}

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
	sf::RenderWindow window(sf::VideoMode(width, height),"");

	Screen& film = scene.getScreen();
	film.postProcessColor();

	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{			
			sf::Color col((sf::Uint8) film(x, y).r, (sf::Uint8) film(x, y).g, (sf::Uint8) film(x, y).b);
#ifdef VS_2010
			image.SetPixel(x, y, col);
#else
			image.setPixel(x, y, col);
#endif
		}
	}

	//toneMapper(film, image);

	window.setFramerateLimit(60);

	sf::Sprite sp;
	sf::Texture texture;
	texture.loadFromImage(image);
	sp.setTexture(texture);

	if (scene.getFileName() != "")
		image.saveToFile(scene.getFileName());

	while (window.isOpen())
	{
		sf::Event ev;

		while (window.pollEvent(ev))
		{
			if (ev.type == sf::Event::Closed)
				window.close();
			else if (ev.type == sf::Event::MouseButtonReleased)
				std::cout << ev.mouseButton.x << " " << ev.mouseButton.y << std::endl;
		}

		window.clear();

		window.draw(sp);

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