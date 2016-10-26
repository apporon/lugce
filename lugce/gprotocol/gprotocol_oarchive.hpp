/* 
* Copyright (C) 2010  
* author:	Chen Wang ( China )
* Email:	jadedrip@gmail.com
* ============================================
*		* GNU Lesser General Public License *
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
#pragma once
#include <stdint.h>
#include <assert.h>
#include <ostream>
#include <boost/tuple/tuple.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repetition.hpp>
#include "gprotocol_common.hpp"

namespace lugce
{
	namespace gprotocol
	{
		/**
			Google protocol �����������������Ƕ��
			\remarks ������������������֧���Ѿ������� serialize �������࣬�Ͳ���C++�ڲ����ͣ��Լ� std::string��
				������ǩ�������ݣ���ǩ���δ�� serialize �����ж��壬��ô���1��ʼ���ε�����ÿ����һ������+1����
				������֧�ֵ��ڲ�����������Ӧ�� Google protocol ���Ͷ�Ӧ��
					int16_t		=>	sint32
					uint16_t	=>	uint32
					int32_t		=>	sint32
					uint32_t	=>	uint32
					int64_t		=>	sint64
					uint64_t	=>	uint64
					float		=>	float
					double		=>	double
					std::string	=>	string
					enum		=>	enum
					bool		=>	bool
		 */
		class oarchive_embedded
		{
		public:
			oarchive_embedded( std::ostream& os ) : _stream(os), _tag(1){}
		public:
			template<class T>
			void serialize(const T &t)
			{ 
				typedef typename details::get_coder< oarchive_embedded, T >::type coder;
				serialize( _tag, t );
			}

			template<class T>
			void serialize( uint32_t tag, const T &t, int option=0 )
			{
				_tag=tag+1;
				typedef typename details::get_coder< oarchive_embedded, T >::type coder;
				coder::save( _stream, tag, t, option );
			}

			void write( uint32_t tag, const void* data, const size_t sz )
			{
				varint_coder<uint32_t>::save( _stream, wire_key( 2, tag ) );
				varint_coder<size_t>::save( _stream, sz );
				_stream.write( (const char*)data, sz );
			}

			template<typename T>
			oarchive_embedded& operator << ( const T &t ){ serialize(t); return *this; }

			template<typename T>
			oarchive_embedded& operator & ( const T &t ){ serialize(t); return *this; }

			template<typename T>
			oarchive_embedded& operator ()( const uint32_t tag, const T& t, int option=0 ){ serialize(tag,t, option); return *this; }
		private:
			uint32_t _tag;
			std::ostream &_stream;
		};

#define OARCHIVE_SAVE_FUNCTION_SERIALIZE( Z, N, _ ) ar.serialize( boost::get<N>(v) );
#define OARCHIVE_SAVE_FUNCTION( Z, N, _ ) \
	template< BOOST_PP_ENUM_PARAMS( N, class T ) > \
	void save( const boost::tuple<BOOST_PP_ENUM_PARAMS(N, T)>& v ) \
	{ \
		oarchive_embedded ar(_ostream); \
		BOOST_PP_REPEAT( N, OARCHIVE_SAVE_FUNCTION_SERIALIZE, _ ) \
	}

		/**
			Google protocol ��������������
			\remarks ������������������֧���Ѿ������� serialize �������࣬�� tuple ����
				���� oarchive_embedded �Ĳ�֮ͬ�����ڣ�������Ϊ����д�����������Ӧ��ֻ����������������
				���� tuple ����������1��ʼ���ε�����
		 */
		class oarchive 
		{
		public:
			/**
			 �Ѷ���д�뵽��
			 \param [in]	object ��д��Ķ���
			 */
			template<class T>
			void save(const T &object)
			{ 
				oarchive_embedded ar( _ostream );
				lugce::gprotocol::serialize( ar, const_cast<T&>(object) );
			}

			template<typename T>
			void operator << ( const T &t){ save(t); }
		public:
			oarchive(std::ostream & os) : _ostream(os){}
		public:
			BOOST_PP_REPEAT_FROM_TO( 1, 10, OARCHIVE_SAVE_FUNCTION, _ );
		private:
			std::ostream &_ostream;
		};
#undef OARCHIVE_SAVE_FUNCTION
#undef OARCHIVE_SAVE_FUNCTION_SERIALIZE
	};
};
