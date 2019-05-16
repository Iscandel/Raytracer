#pragma once

#include <memory>

//class A;
//class B;
//
//template <typename T>
//struct Traits {
//	typedef std::shared_ptr<A>  Ptr;
//};
//
//class A
//{
//	Traits<B>::Ptr foo();
//};
//
//class B
//{
//	Traits<A>::Ptr bar();
//};

template<class Derived>
class WithSmartPtr
{
public:
#ifdef USE_RAW_PTR
	typedef Derived* ptr;
	typedef const Derived* ptr;
	template<class ...Args>
	inline static typename ptr createPtr(Args&&...args) {
		return new Derived(args);
	}

	inline static typename constPtr createConstPtr(Args&&...args) {
		return new const Derived(args);
	}
#else
	typedef std::shared_ptr<Derived> ptr;
	typedef std::shared_ptr<const Derived> constPtr;
	template<class ...Args>
	inline static typename ptr createPtr(Args&&...args) {
		return std::make_shared<Derived>(args);
	}

	//template<class ...Args>
	//inline static typename constPtr createConstPtr(Args&&...args) {
	//	return std::make_shared<const Derived>(std::forward(args));
	//}
#endif
};