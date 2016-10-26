#pragma once
/**
* Copyright (C) 2010  Chen Wang ( China )
* Email: jadedrip@gmail.com
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "atomic_type.hpp"

#ifdef __cplusplus
namespace lugce{
	namespace lockfree{
		namespace details
		{ /// \private
			template<int size>
			struct atomic_typor;

			template<>
			struct atomic_typor<4>
			{
				typedef atomic_int32_t atomic_type;
				typedef int32 value_type;
			};

			template<>
			struct atomic_typor<8>
			{
				typedef atomic_int64_t atomic_type;
				typedef int64 value_type;
			};
		};

		typedef details::atomic_typor< sizeof(void*) >::atomic_type atomic_intptr_t;

		/// ����ʵ����һ��֧��ԭ�Ӳ���������
		/// �������֧�ֵĲ����� ++, --��+=, -=, cas, ()
		/// ���� T ������ 32λ�� int, long (Ҳ�������޷��Ű汾) �� 64λ�� long long (Ҳ�������޷��Ű汾)
		template<typename T>
		class atomic_integer : public details::atomic_typor<sizeof(T)>::atomic_type
		{
			BOOST_STATIC_ASSERT( (sizeof(T)==4) || (sizeof(T)==8) );
			BOOST_STATIC_ASSERT( boost::is_integral<T>::value );

			typedef atomic_integer<T> this_class;
		public:
			typedef typename details::atomic_typor<sizeof(T)>::atomic_type atomic_type;
			typedef typename details::atomic_typor<sizeof(T)>::value_type value_type;

			atomic_integer(){};
			atomic_integer( const T v ){ this->data=value_type(v); }
		public:
			operator const T() const{ return static_cast<const T>(data); }
			operator T(){ return static_cast<T>(data); }

			/// ִ��һ�� cas ѭ�������뺯�������������޸��㷨
			template< typename _FUNCTOR >
			this_class& operator()( _FUNCTOR functor )
			{ 
				for(;;){
					value_type x=data;
					if( atomic_compare_and_set( *this, x, (value_type)functor( x ) ) ) 
						break;
				}
				return *this; 
			}

			FORCEINLINE T STDCALL exchange( const T v )
			{
				return T(atomic_exchange( *this, (value_type)v ));
			}

			/// ���ԭ��ֵ������ֵ��ȣ���ô��ԭ��ֵ�滻ΪҪ�޸ĵ���ֵ
			/// <param name="cmp">�Ƚ�ֵ</param>
			/// <param name="xchg">Ҫ�޸ĵ���ֵ</param>
			FORCEINLINE bool STDCALL compare_and_set( const T cmp, const T xchg )
			{
				return atomic_compare_and_set( *this, (value_type)(cmp), (value_type)(xchg) );
			}

			FORCEINLINE T operator++()
			{
				return (T)atomic_exchange_and_increment( *this ) + 1;
			}

			FORCEINLINE friend T operator++( this_class& var, int )
			{ 
				return (T)atomic_exchange_and_increment( var );
			}

			FORCEINLINE T operator--()
			{
				return (T)atomic_exchange_and_decrement( *this ) - 1;
			}

			FORCEINLINE friend T operator--( this_class& var, int )
			{ 
				return (T)atomic_exchange_and_decrement( var );
			}

			FORCEINLINE T operator +=( const T v )
			{
				return atomic_exchange_and_add( *this, (value_type)v ) + v;
			}

			FORCEINLINE T operator -=( const T v )
			{
				return atomic_exchange_and_subtract( *this, (value_type)v ) - v;
			}
		};

		typedef atomic_integer<int32> atomic_int32;
		typedef atomic_integer<int64> atomic_int64;
		typedef atomic_integer<intptr_t> atomic_intptr;

		/// ������һ��֧��ԭ�Ӳ�����ָ��
		/// T �Ƕ��������
		template<typename T>
		class atomic_pointer : public atomic_intptr_t
		{
			typedef atomic_pointer<T> this_class;
			typedef details::atomic_typor<sizeof(intptr_t)>::atomic_type atomic_type;
		public:
			atomic_pointer( T* const v=nullptr ){ this->data=intptr_t(v); }
		public:
			operator T*(){ return reinterpret_cast<T*>(data); }
			operator const T*() const{ return reinterpret_cast<const T*>(data); }
			T* operator ->(){ return reinterpret_cast<T*>(data); }
			const T* operator ->() const{ return reinterpret_cast<T*>(data); }

			FORCEINLINE T* STDCALL exchange( T* v )
			{
				return reinterpret_cast<T*>(atomic_exchange( *this, reinterpret_cast<intptr_t>(v) ));
			}

			FORCEINLINE bool STDCALL compare_and_set( const T* cmp, const T* xchg )
			{
				return atomic_compare_and_set( *this, reinterpret_cast<intptr_t>(cmp), reinterpret_cast<intptr_t>(xchg) );
			}

			/// ǰ׺�ӷ�
			FORCEINLINE T* operator++()
			{
				return reinterpret_cast<T*>( atomic_exchange_and_add( *this, sizeof(T) ) + sizeof(T) );
			}

			FORCEINLINE friend T* operator++( this_class& var, int )
			{
				return reinterpret_cast<T*>( atomic_exchange_and_add( var, sizeof(T) ) );
			}

			FORCEINLINE T* operator--()
			{
				return reinterpret_cast<T*>( atomic_exchange_and_subtract( *this, sizeof(T) ) - sizeof(T) );
			}

			FORCEINLINE friend T* operator--( this_class& var, int )
			{
				return reinterpret_cast<T*>( atomic_exchange_and_subtract( var, sizeof(T) ) );
			}

			FORCEINLINE T* operator +=( const int v )
			{
				return reinterpret_cast<T*>( atomic_exchange_and_add( *this, sizeof(T) * v ) + sizeof(T) * v );
			}

			FORCEINLINE T* operator -=( const int v )
			{
				return reinterpret_cast<T*>( atomic_exchange_and_subtract( *this, sizeof(T) * v ) - sizeof(T) * v );
			}
		};
	}; // namespace lockfree
};	// namespace lugce

#endif
