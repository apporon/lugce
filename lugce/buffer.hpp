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
* =============================================
* Copyright (c) 2010�����߱�������Ȩ����( All rights reserved. )
* �ļ����ƣ�readable_data.hpp
* ժ Ҫ�������ṩһ�� buffer �࣬������������һ���ڴ�������ʹ��
* 
* ��ǰ�汾��1.0
* �� �ߣ����
* ������ڣ�2012��4��18��
* ƽ̨���ԣ�Visual Studio 2010��gcc 4.3.3
* =============================================
*/
#pragma once
#include <stdint.h>
#include <string>
#include <ostream>

namespace lugce
{
	/// ����һ���ڴ�������ʹ��
	/// �������STL����������塣����Ч�ʿ��ǣ������Ƿ��̰߳�ȫ�ġ�
	/// ע�⣬�����ƺ�ָ��ͬһ���ڴ棬������ڼ����ü�����ʵ���ڴ���ͷš�
	class buffer 
	{
	public:
		typedef const uint8_t * const_reference;
		typedef uint8_t value_type;
	public:
		enum { block_size = 1024 };
		static const size_t npos = (size_t)-1;

		buffer()  { _block = 0; _size = 0; _data = NULL; }

		buffer( const buffer& v ) : _data(v._data), _size(v._size), _block(v._block)
		{
			if( _data )	++*reinterpret_cast<size_t*>(_data);
		}

#ifdef _HAS_CPP0X
		/// Move constructor
		buffer( buffer&& v ) : _data(v._data), _size(v._size), _block(v._block)
		{
			v._data=nullptr;
		}
#endif

		buffer(const void* v, size_t sz) : _data(NULL), _size(0), _block(0)
		{
			assign( v, sz );
		}

		virtual ~buffer()
		{
			if( _data ){
				size_t *p=(size_t*)_data;
				if(--*p==0)	free( _data );
			}
		}
	public:
		/// �������ü���
		size_t ref_count() const
		{
			return _data ? *reinterpret_cast<size_t*>(_data) : 0;
		}

		void assign( const void* data, size_t sz )
		{
			if(sz==0)
				clear();
			else{
				resize(sz);
				memcpy(_data+sizeof(size_t), data, sz);
			}
		}

		void assign( const std::string& v ){ assign(v.data(),v.size()); }

		inline bool   empty() const     { return _size == 0; }
		/// �������
		void clear(){ _size=0; }

		uint8_t* data(){ return _data+sizeof(size_t); }
		inline uint8_t * data() const      { return _data+sizeof(size_t); }
		inline size_t size() const      { return _size; }
		const char * c_str()
		{
			reserve( _size+1 );
			_data[_size+sizeof(size_t)]=0;
			return (const char*)data();
		}

		/// �����趨��С
		void resize(size_t n, uint8_t c)
		{
			reserve(n);
			if( n > _size )	memset( data()+_size, c, n-_size );
			_size=n;
		}

		/// �����趨��С
		void resize(size_t n)
		{
			reserve(n);
			_size=n;
		}

		/// Ԥ�����ڴ�
		void reserve(size_t n)
		{
			n+=sizeof(size_t);
			if( _data ){
				if( n <= _block*block_size )	return; // ����ڴ��Ѿ��㹻
				size_t nb=n / block_size + ( (n % block_size) ? 1 : 0 ); // �����¿�
				uint8_t *p=(uint8_t*)realloc( _data, block_size * nb );
				if( !p ) throw std::bad_alloc();
				_data=p;
				_block=nb;
			}else{
				_block=n / block_size + ( (n % block_size) ? 1 : 0 ); // �����¿�
				_data=(uint8_t*)malloc( block_size * _block );
				_size=0;
				*reinterpret_cast<size_t*>(_data)=1;
			}
		}

		/// ����Ԥ������ڴ���
		size_t reserved() const{ return _block*block_size;  }

		/// �ڽ�β����һ���ַ�
		void push_back( uint8_t c )
		{
			reserve(1+_size);
			_data[ sizeof(size_t)+_size++ ]=c;
		}

		void append(const void *app, size_t len)
		{
			reserve(len+_size);
			memcpy( data()+_size, app, len );
			_size+=len;
		}

		void add(const uint8_t c, size_t len)
		{
			reserve(len+_size);
			memset( data()+_size, c, len );
			_size+=len;
		}

		void replace(size_t pos, const void *rep, size_t n)
		{
			reserve( pos+n );
			memcpy( data()+pos, rep, n );
			if( pos+n > _size ) _size=pos+n;
		}

		void erase(size_t pos = 0, size_t n = npos )
		{
			if( pos >= _size ) return;
			if( n==npos || (pos+n>=_size) )
				_size=pos;
			else{
				_size-=pos+n;
				memcpy( data()+pos, _data+pos+n, _size );
			}
		}

		void swap( buffer& bb )
		{
			std::swap( _data, bb._data );
			std::swap( _size, bb._size );
			std::swap( _block, bb._block );
		}

		uint8_t& operator[]( const size_t idx )
		{
			assert( idx < _size );
			return data()[idx];
		}

		const uint8_t& operator[]( const size_t idx ) const
		{
			assert( idx < _size );
			return data()[idx];
		}
	public:
		friend std::ostream& operator << ( std::ostream& os, const buffer& bu )
		{
			return os.write( (const char*)bu.data(), bu.size() );
		}
	private:
		uint8_t  *_data;
		size_t _size;	// ��Ч����
		size_t _block;	// Ԥ��������ݿ���
	};
};
