#pragma once

#include "ObjectFactory.h"
#include "tools/Singleton.h"

#include <map>
#include <memory>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////
/// \brief Manager for object factories.
///////////////////////////////////////////////////////////////////////////////
template<class ReturnType>
class ObjectFactoryManager : public Singleton<ObjectFactoryManager<ReturnType>>
{
public:
	typedef std::map<std::string, typename ObjectFactory<ReturnType>::ptr> FactoryMap;

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Registers a new factory in the manager.
	///
	/// \param factory : Smart pointer to the factory to register.
	///////////////////////////////////////////////////////////////////////////
	void addFactory(typename ObjectFactory<ReturnType>::ptr factory);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Removes a previously added factory.
	///
	/// \param type : Factory name.
	///////////////////////////////////////////////////////////////////////////
	void removeFactory(const std::string& type);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Returns the factory identified by its type name. If the 
	/// requested factory cannot be found, an exception is thrown.
	///
	/// \param type : Factory type name.
	///
	/// \return A smart pointer to the factory.
	///////////////////////////////////////////////////////////////////////////
	typename ObjectFactory<ReturnType>::ptr getFactory(const std::string& type);

protected:
	FactoryMap myFactories;
};

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
template<class ReturnType>
void ObjectFactoryManager<ReturnType>::addFactory(typename ObjectFactory<ReturnType>::ptr factory)
{
	FactoryMap::iterator it = myFactories.find(factory->getObjectType());

	if (factory->getObjectType() == "")
	{
		throw std::runtime_error("No specified type for the object");
	}

	if (it != myFactories.end())
	{
		throw std::runtime_error("Object " + factory->getObjectType() + " has already been registered");
	}

	myFactories.insert(std::make_pair(factory->getObjectType(), factory));
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
template<class ReturnType>
void ObjectFactoryManager<ReturnType>::removeFactory(const std::string& type)
{
	FactoryMap::iterator it = myFactories.find(type);
	if (it != myFactories.end())
	{
		myFactories.erase(type);
	}
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
template<class ReturnType>
typename ObjectFactory< ReturnType>::ptr ObjectFactoryManager<ReturnType>::getFactory(const std::string& type)
{
	FactoryMap::iterator it = myFactories.find(type);
	if (it != myFactories.end())
	{
		return it->second;
	}
	else
	{
		throw std::runtime_error(std::string("The requested factory " + type + " doesn't exist"));
	}
}

//Should we define template<class T> struct WidgetStaticType; instead
//and get an error during the compilation if the class hasn't been
//registered ?
template<class T>
struct ObjectStaticType
{
	static const char* get() {
		throw std::runtime_error("Missing widget static type. "
			"Did you forget to register your class type ?");
	}
};


#define RT_REGISTER_TYPE(ObjectType, BaseType) \
struct ObjectType ##_ \
{ \
	ObjectType ##_ () \
	{ \
		ObjectFactoryManager<BaseType>::getInstance()->addFactory(ObjectFactory<BaseType>::ptr(new TplObjectFactory<BaseType, ObjectType>() )); \
	} \
} ObjectType##_RT;  \
template<> \
struct ObjectStaticType<ObjectType> \
{ \
	static const char* get() {return #ObjectType;} \
};

#define RT_REGISTER_TYPE_WITH_FACTORY(ObjectType, BaseType, Factory) \
struct ObjectType ##_ \
{ \
	ObjectType ##_ () \
	{ \
		ObjectFactoryManager<BaseType>::getInstance()->addFactory(ObjectFactory<BaseType>::ptr(new Factory<BaseType, ObjectType>())); \
	} \
} ObjectType##_RT; \
template<> \
struct ObjectStaticType<ObjectType> \
{ \
	static const char* get() {return #ObjectType;} \
};

//