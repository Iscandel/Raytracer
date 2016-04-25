#ifndef H__SINGLETON_230520151431__H
#define H__SINGLETON_230520151431__H

///////////////////////////////////////////////////////////////////////////////
/// Singleton template class. Derived classes should inherit this one. The
/// template parameter T should be the name of the derived classes.
///////////////////////////////////////////////////////////////////////////////
template <typename T> class Singleton
{
protected:
	Singleton( void ) { }
    virtual ~Singleton( void ) { }

public:
	///////////////////////////////////////////////////////////////////////////
	/// Returns the unique instance
	/// 
	/// \return The instance
	///////////////////////////////////////////////////////////////////////////
    static T *getInstance( void )
    {
        if( m_pSingleton == 0 )
            m_pSingleton = new T();
       
        return m_pSingleton;
    }

	///////////////////////////////////////////////////////////////////////////
	/// Destroys the singleton instance.
	///////////////////////////////////////////////////////////////////////////
    static void kill( void )
    {
        if( m_pSingleton != 0 )
        {
            delete m_pSingleton;
            m_pSingleton = 0;
        }
    }

protected:
    static T *m_pSingleton; ///< Static unique instance
};

template <typename T> T *Singleton<T>::m_pSingleton = 0;



#endif