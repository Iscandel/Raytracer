///////////////////////////////////////////////////////////////////////////////
// Headers
///////////////////////////////////////////////////////////////////////////////
#include "ObjectFactoryManager.h"

////=============================================================================
/////////////////////////////////////////////////////////////////////////////////
//void ObjectFactoryManager::addFactory(ObjectFactory::ptr factory) 
//{
//	FactoryMap::iterator it = myFactories.find(factory->getObjectType());
//	
//	if(factory->getObjectType() == "" )
//	{
//		throw std::runtime_error("No specified type for the object");
//	}
//
//	if(it != myFactories.end())
//	{
//		throw std::runtime_error("Object " + factory->getObjectType() + " has already been registered");
//	}
//
//	myFactories.insert(std::make_pair(factory->getObjectType(), factory));
//}
//
////=============================================================================
/////////////////////////////////////////////////////////////////////////////////
//void ObjectFactoryManager::removeFactory(const std::string& type)
//{
//	FactoryMap::iterator it = myFactories.find(type);
//	if(it != myFactories.end())
//	{
//		myFactories.erase(type);
//	}
//}
//
////=============================================================================
/////////////////////////////////////////////////////////////////////////////////
//ObjectFactory::ptr ObjectFactoryManager::getFactory(const std::string& type)
//{
//	FactoryMap::iterator it = myFactories.find(type);
//	if(it != myFactories.end())
//	{
//		return it->second;
//	}
//	else
//	{
//		throw std::runtime_error(std::string("The requested factory " + type + " doesn't exist"));
//	}
//}
