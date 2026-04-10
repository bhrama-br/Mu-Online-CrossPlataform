//////////////////////////////////////////////////////////////////////////
//  - ½Ì±ÛÅæ -
//  
//  
//////////////////////////////////////////////////////////////////////////
#ifndef __SINGLETON_H__
#define __SINGLETON_H__

#include <cstddef>

/*+++++++++++++++++++++++++++++++++++++
    CLASS.
+++++++++++++++++++++++++++++++++++++*/
template <typename T> 
class Singleton
{
    static T* _Singleton;

public:
    Singleton( void )
    {
        if ( _Singleton==0 )
        {
            const std::ptrdiff_t offset =
                reinterpret_cast<char*>(static_cast<T*>(reinterpret_cast<T*>(1))) -
                reinterpret_cast<char*>(static_cast<Singleton<T>*>(reinterpret_cast<T*>(1)));
            _Singleton = reinterpret_cast<T*>(reinterpret_cast<char*>(this) + offset);
        }
    }
    
    virtual ~Singleton( void ) {  /*assert( _Singleton );*/  _Singleton = 0;  }
    
    static T&   GetSingleton ( void )      {  /*assert( _Singleton );*/  return ( *_Singleton );  }
    static T*   GetSingletonPtr ( void )   {  return ( _Singleton ); } 
	static bool IsInitialized ( void )     { return _Singleton ? true : false; }

	//¿©±â ºÎºÐÀº Á» »ý°¢À» ÇØº¸ÀÚ..
	//new·Î ¸¸µé¾î¼­ ³ÖÀ¸¸é...delete¸¦ ÇØÁà¾ß ÇÏ´Âµ¥...
	//ÇÒ·Á¸é boost·Î ¸¸µé¾îÁø data¸¸ ³Öµµ·Ï ÇÏÀÚ.
	//static void RegisterSingleton ( T* p ) { _Singleton = p; }
};

template <typename T> T* Singleton <T>::_Singleton = 0;

#endif
