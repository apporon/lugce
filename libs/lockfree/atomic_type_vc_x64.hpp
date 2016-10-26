/*	This library is free software; you can redistribute it and/or
*	modify it under the terms of the GNU Lesser General Public
*	License as published by the Free Software Foundation; either
*	version 2.1 of the License, or (at your option) any later version.
*
*	This library is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*	Lesser General Public License for more details.
*
*	You should have received a copy of the GNU Lesser General Public
*	License along with this library; if not, write to the Free Software
*	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*****************************************************************************
*	A beg: Please tell me if you are using this library.
*	Author: Chen Wang ( China )
*	Email: jadedrip@gmail.com
*/

// ֧�� VC �����ԭ�Ӳ�����ʵ��
// ע�⣺����ֱ�Ӱ������ļ�
#pragma once
#include <Windows.h>

#ifdef __cplusplus
namespace lugce{
	namespace lockfree{
#endif

/// ����ֵ, ������ԭʼֵ
FORCEINLINE int32 atomic_exchange( atomic_int32_t& a, int32 new_value )
{
	return _InterlockedExchange( &a.data, new_value );
}

/// ����ֵ, ������ԭʼֵ
FORCEINLINE int64 atomic_exchange( atomic_int64_t& atomic, int64 new_value )
{
	return InterlockedExchange64( &atomic.data, new_value );
}

/// �Ƚϲ�������ֵ�������Ƿ�ɹ�
FORCEINLINE bool atomic_compare_and_set( atomic_int32_t& atomic, int32 comp, int32 xchgn )
{
	return ( comp==InterlockedCompareExchange( &atomic.data, xchgn, comp ) );
}

/// �Ƚϲ�������ֵ�������Ƿ�ɹ�
FORCEINLINE bool atomic_compare_and_set( atomic_int64_t& atomic, int64 comp, int64 xchgn )
{
	return ( comp==InterlockedCompareExchange64( &atomic.data, xchgn, comp ) );
}

/// ������������ԭʼֵ
FORCEINLINE int32 atomic_exchange_and_increment( atomic_int32_t& atomic )
{
	return InterlockedIncrement( &atomic.data );
}

/// �Լ���������ԭʼֵ
FORCEINLINE int32 atomic_exchange_and_decrement( atomic_int32_t& atomic )
{
	return InterlockedDecrement( &atomic.data );
}

/// ������������ԭʼֵ
FORCEINLINE int64 atomic_exchange_and_increment( atomic_int64_t& atomic )
{
	return InterlockedIncrement64( &atomic.data );
}

/// �Լ���������ԭʼֵ
FORCEINLINE int64 atomic_exchange_and_decrement( atomic_int64_t& atomic )
{
	return InterlockedDecrement64( &atomic.data );
}

/// ԭ�Ӽӷ�������ԭʼֵ
FORCEINLINE int32 atomic_exchange_and_add( atomic_int32_t& atomic, int32 v )
{
#ifdef InterlockedAdd
	return InterlockedAdd( &atomic.data, v );
#else
	int32 old;
	do{
		old=atomic;
	}while( !atomic_compare_and_set( atomic, old, old+v ) );
	return old;
#endif
}

FORCEINLINE int32 atomic_exchange_and_subtract( atomic_int32_t& atomic, int32 v )
{
	return atomic_exchange_and_add( atomic, -v );
}

FORCEINLINE int64 atomic_exchange_and_add( atomic_int64_t& atomic, int64 v )
{
#ifdef InterlockedAdd64
	return InterlockedAdd64( &atomic.data, v );
#else
	int64 old;
	do{
		old=atomic;
	}while( !atomic_compare_and_set( atomic, old, old+v ) );
	return old;
#endif
}

FORCEINLINE int64 atomic_exchange_and_subtract( atomic_int64_t& atomic, int64 v )
{
	return atomic_exchange_and_add( atomic, -v );
}

#ifdef __cplusplus
	};
};
#endif