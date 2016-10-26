#pragma once

#if !defined(_MSC_VER) || (_MSC_VER < 1600)
#	define nullptr NULL
#endif

#if (_MSC_VER >= 1600)	// �Ƿ�ʹ�� C++ 0x 
#	define CPP0X	1
#endif

#if (_MSC_VER >= 1500)	// �Ƿ�ʹ�� C++ 0x TR1 ��
#	define STL_TR1	1	
#endif

// �Ƿ�64λ����
#if defined( __x86_64__ ) || defined( _WIN64 )	
#	define __LUGCE_64	1
#else
#	define __LUGCE_32	1
#endif

#if !defined(STDCALL) && defined( __cplusplus )
#	define	STDCALL	__stdcall
#else
#	define	STDCALL
#endif

#ifndef FORCEINLINE
#if (_MSC_VER >= 1200)
#	define FORCEINLINE __forceinline
#elif defined( __GNUC__ )
#	define FORCEINLINE __inline__
#else
#	define FORCEINLINE __inline
#endif
#endif