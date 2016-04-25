#pragma once
#include <random>

class Rng
{
public:
	Rng();
	~Rng();


	std::default_random_engine& global_urng()
	{
		static std::default_random_engine u;
		return u;
	}

	void randomize()
	{
		static std::random_device rd{};
		global_urng().seed(rd());
	}

	int random(int from, int thru)
	{
		static std::uniform_int_distribution<>d{};

		using parm_t = decltype(d)::param_type;
		return d(global_urng(), parm_t{ from, thru });
	}

	double random(double from, double upto)
	{
		static std::uniform_real_distribution<>	d{};

		using parm_t = decltype(d)::param_type;

		return d(global_urng(), parm_t{ from, upto });
	};
};

