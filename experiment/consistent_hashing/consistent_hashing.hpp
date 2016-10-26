#pragma once
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

namespace lugce
{
	/// һ���Թ�ϣ�㷨����
	/// <remarks>�㷨���������Բο� http://blog.csdn.net/sparkliang/article/details/5279393 </remarks>
	template< typename T >
	class consistent_hashing_container
	{
		friend class handle;
	private:
		typedef std::map< uint32_t, typename T > values_type;
		struct data_type
		{
			boost::shared_mutex rwmutex;
			values_type value_map;
		};
		boost::shared_ptr<data_type> _data;
	public:
		/// һ���Թ�ϣ�㷨���������ʱ���أ����������Ƴ������ֵ
		class handle
		{
			friend class consistent_hashing_container<T>;
		public:
			typedef boost::shared_ptr< handle > pointer;
			/// ��һ���Թ�ϣ�㷨�������Ƴ�����Ľڵ�
			void remove()
			{
				boost::unique_lock<boost::shared_mutex> write_lock( _data->rwmutex );
				BOOST_AUTO( i, _hash_keys.begin() );
				BOOST_AUTO( e, _hash_keys.end() );
				for( ; i!=e; ++i )
					_data->value_map.erase( *i );
				_hash_keys.clear();
			}
			operator const bool () const{ return !_hash_keys.empty(); }
		private:
			handle( const boost::shared_ptr< data_type >& d ) : _data(d){} //
			boost::shared_ptr< data_type > _data;
			std::vector< size_t > _hash_keys;
		};
	public:
		consistent_hashing_container() : _data( new data_type() )
		{
		}

		/// ͨ�� Key ��ȡһ��ֵ
		/// <remarks>����ʱ��Ҫ��֤���� U ������Ϊ�� boost::hash �����Ĳ���</remarks>
		template< typename U >
		const T get( const U& key ) const
		{
			assert( _data );
			if( _data->value_map.empty() ) return T();
			boost::hash<typename U> hasher;
			uint32_t v=hasher( key );
			boost::shared_lock<boost::shared_mutex> read_lock( _data->rwmutex );
			BOOST_AUTO( i, _data->value_map.lower_bound(v) );
			if( i==_data->value_map.end() ) i=_data->value_map.begin();
			return i->second;
		}

		template< typename U >
		handle set( const U& key, const T& value, const int cnt=32 )
		{
			assert( _data );
			size_t seed=boost::hash_value( key );
			handle h( _data );

			boost::unique_lock<boost::shared_mutex> write_lock( _data->rwmutex );
			if( !set_one(seed,value) )
				return h;
			h._hash_keys.push_back(seed);

			for( int i=0; i<cnt-1; ++i ){
				boost::hash_combine( seed, i+1 );
				if( set_one( seed, value ) )
					h._hash_keys.push_back(seed);
				else{
					h.remove();
					return h;
				};
			}
			return h;
		}
	private:
		bool set_one( const uint32_t hash_key, const T& v )
		{
			assert( _data );
			return _data->value_map.insert( std::make_pair(hash_key, v) ).second;
		}
	};
};