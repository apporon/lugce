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
#include <map>
#include <functional>
#include <istream>
#include <boost/tuple/tuple.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repetition.hpp>
#include "gprotocol_common.hpp"

namespace lugce
{
	namespace gprotocol
	{
		/**
			Google protocol ������������������Ƕ��
			\remarks �������������������֧���Ѿ������� serialize �������࣬�Ͳ���C++�ڲ����ͣ��Լ� std::string��
				����Ϊ����д���ǩ����ǩ���δ�� serialize �����ж��壬��ô���1��ʼ���ε�����ÿд��һ������+1����
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
		class iarchive_embedded 
		{
		private:
			typedef std::function< void(size_t) > decoder_func;
			std::map< int, std::pair< uint8_t, decoder_func> > _decoders;
		public:
			template<class T>
			void serialize(T &t){ serialize( _tag, t ); }

			/**
			 Serializes.
			
			 \tparam	T	Generic type parameter.
			 \param	tag			 	��ǩ
			 \param [in,out]	t	�����Ķ���
			 */
			template<class T>
			void serialize( uint32_t tag, T &t)
			{ 
				_tag=tag+1;
				typedef typename details::get_coder< iarchive_embedded, T >::type coder;
				_decoders[ tag ]=std::make_pair( coder::wire, std::bind( &coder::load, std::ref(_stream), std::ref(t), std::placeholders::_1 ) ); 
			}

			template<typename T>
			iarchive_embedded& operator >> (T &t){ serialize(t); return *this; }

			template<typename T>
			iarchive_embedded& operator & (T &t){ serialize(t); return *this; }

			template<typename T>
			iarchive_embedded& operator ()( const uint32_t tag, T& t ){ serialize( tag, t ); return *this; }
		private:
			void skip_type( uint8_t wire )
			{
				uint32_t v;
				switch(wire){
				case 0:	// varint
					varint_coder<uint32_t>::load( _stream, v );
					break;
				case 1: // 64-bit
					_stream.seekg( 8, std::ios::cur );
					break;
				case 2: // Length-delimited
					varint_coder<uint32_t>::load( _stream, v );
					_stream.seekg( v, std::ios::cur );
					break;
				case 3:
					throw std::bad_cast();
					break;
				case 4:
					throw std::bad_cast();
				case 5:
					_stream.seekg( 4, std::ios::cur );
					break;
				default:
					throw std::bad_cast();
				}
			}
		public:
			/**
			 ���캯��
			 \param [in]	os	������
			 */
			iarchive_embedded(std::istream & os) : _stream(os), _tag(1){}

			/**
			 ��������ʽ��������
			 \exception	std::bad_cast	����������ʹ��󣬻��׳� std::bad_cast �쳣
			 \param	byte_count	(Optional) ��ȡ���ֽ�����Ĭ��һֱ���� eof��.
			 */
			void load( const size_t byte_count=0 )
			{
				int64_t b=_stream.tellg();
				while( !_stream.eof() ){
					if( byte_count>0 && ( (b+byte_count) <= _stream.tellg() ) )
						return;
					uint32_t v;
					varint_coder<uint32_t>::load( _stream, v );
					if( _stream.eof() ) return;
					const uint32_t idx=v >> 3;
					const uint8_t wire=v & 7;
					auto i=_decoders.find(idx);
					if( i==_decoders.end() ){ // unknow index, skip it
						skip_type(wire);
						return;
					}

					if( i->second.first!=wire )
						throw std::bad_cast();
					assert( i->second.second );
					if( wire==2 ){
						//uint32_t sz;
						//varint_coder<size_t>::load( _stream, sz );
						i->second.second(0);
					}else{
						i->second.second(0);
					}
				}
			}
		private:
			std::istream &_stream;
			uint32_t _tag;
		};

#define IARCHIVE_LOAD_FUNCTION_SERIALIZE( Z, N, _ ) ar.serialize( boost::get<N>(v) );
#define IARCHIVE_LOAD_FUNCTION( Z, N, _ ) \
	template< BOOST_PP_ENUM_PARAMS( N, class T ) > \
	void load( boost::tuple<BOOST_PP_ENUM_PARAMS(N, T) >& v ) \
	{ \
		iarchive_embedded ar(_istream); \
		BOOST_PP_REPEAT( N, IARCHIVE_LOAD_FUNCTION_SERIALIZE, _ ) \
		ar.load(); \
	}

		/**
			Google protocol ��������������
			\remarks �������������������֧���Ѿ������� serialize �������࣬�� tuple ����
				���� iarchive_embedded �Ĳ�֮ͬ�����ڣ�������Ϊ����д�����������Ӧ��ֻ��������������롣
				���� tuple ����������1��ʼ���ε�����
		 */
		class iarchive
		{
		private:
			std::istream& _istream;
		public:
			iarchive( std::istream& is ) : _istream(is){}
		public:
			/**
			 �����ж�������
			 \param [out]	object �������Ķ���
			 */
			template<class T>
			void load(T &object)
			{ 
				iarchive_embedded ar(_istream);
				lugce::gprotocol::serialize( ar, object );
				ar.load();
			}

			template<typename T>
			void operator >> (T &t){ load(t); }
		public:
			BOOST_PP_REPEAT_FROM_TO( 1, 10, IARCHIVE_LOAD_FUNCTION, _ )
		};
#undef IARCHIVE_LOAD_FUNCTION
#undef IARCHIVE_LOAD_FUNCTION_SERIALIZE
	};
};
