#pragma once

//#include "Object.h"
#include "core/Parameters.h"

#include <string>
#include <memory>


///////////////////////////////////////////////////////////////////////////////
/// \brief Base class of object factory.
///////////////////////////////////////////////////////////////////////////////
//template<class BaseType, typename ...Args>
template<class BaseType>
class ObjectFactory
{
public:
	typedef std::shared_ptr<ObjectFactory> ptr;

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor.
	///  
	/// \param type : Type name of the widget to be handled by this factory.
	///////////////////////////////////////////////////////////////////////////
	ObjectFactory(const std::string& type)
	:myType(type)
	{

	}

	///////////////////////////////////////////////////////////////////////////
	// \brief Virtual destructor.
	///////////////////////////////////////////////////////////////////////////
	virtual ~ObjectFactory(void) {}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Returns the widget type name created by this factory.
	///
	/// \return Widget type.
	///////////////////////////////////////////////////////////////////////////
	const std::string& getObjectType() const {return myType;}

	///////////////////////////////////////////////////////////////////////////
	// \brief Creates a widget with the given name.
	///////////////////////////////////////////////////////////////////////////
	//virtual typename BaseType::ptr create(Args&&... args) = 0;//const std::string& name) = 0;
	virtual typename BaseType::ptr create(const Parameters& params) = 0;//const std::string& name) = 0;

protected:
	std::string myType;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Constructs a widget of the given type.
///
/// Usage:
///
/// \code
/// PtrWidgetFactory p(new TplWidgetFactory<SomeWidget>);
/// WidgetFactoryManager::getInstance()->addFactory(p);
/// \endcode
///
/// \see addFactory
///////////////////////////////////////////////////////////////////////////////
template<class BaseType, class ObjectType, typename ...Args>
class TplObjectFactory : public ObjectFactory<BaseType>
{
public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor.
	///////////////////////////////////////////////////////////////////////////
	TplObjectFactory() 
	:ObjectFactory<BaseType>(ObjectStaticType<ObjectType>::get())
	{
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Creates a new widget with the given name.
	///
	/// \param name : Unique name to give to this widget.
	///
	/// \return A smart pointer to the newly created widget.
	///////////////////////////////////////////////////////////////////////////
	//typename BaseType::ptr create(Args&&... args) override//const std::string& name) 
	typename BaseType::ptr create(const Parameters& params) override//const std::string& name) 
	{
		return BaseType::ptr(new ObjectType(params));
		//return std::make_shared<ObjectType>(params);
		//return std::make_shared<ObjectType>(std::forward<Args>(args)...);
	}
//protected:
	//std::string myType; ///< Object type
};

