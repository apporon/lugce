#pragma once
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/variadic/size.hpp>
#include <boost/preprocessor/variadic/to_list.hpp>
#include <boost/preprocessor/punctuation/paren.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/if.hpp>
#include <boost/function.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/type_traits.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/ref.hpp>

#ifdef __GXX_EXPERIMENTAL_CXX0X__
#	define _HAS_CPP0X 1
#endif

#define MAX_SIGNAL_FUNCTION_PARAMS 9
#ifdef _HAS_CPP0X
#	define OBSERVER_TYPE_ID_TYPE size_t
#	define OBSERVER_GET_ID_BY_TYPE(x) x.hash_code()
#else
#	define OBSERVER_TYPE_ID_TYPE std::string
#	define OBSERVER_GET_ID_BY_TYPE(x) x.name()
#endif

namespace lugce{
	// �����¼��Ļ���
	class event_base 
	{
		friend class observer;
	public:
		event_base( const OBSERVER_TYPE_ID_TYPE v ) : _id(v){}
		OBSERVER_TYPE_ID_TYPE get_id() const{ return _id; }
	protected:
		OBSERVER_TYPE_ID_TYPE _id;
		virtual void shot( boost::function_base* )=0;
	};

	namespace details{
		// �� mpl ת�����б�, �������������� tuple ���޷����ڣ���˶������ã���Ҫת���� boost::reference_wrapper<X> ����ʽ
		template< int N, class T > 
		struct mpl_vec2tuple_helper;

#define OBSERVER_TYPE_LINE( Z, N, _ ) typedef typename boost::mpl::at_c<param_type, N>::type BOOST_PP_CAT(T,N);
#define OBSERVER_EVENT_TUPLE_LINE( Z, N, _) BOOST_PP_COMMA_IF(N) typename boost::mpl::if_c< boost::is_reference<BOOST_PP_CAT(T,N)>::value, typename boost::reference_wrapper< typename boost::remove_reference< BOOST_PP_CAT(T,N)>::type >, BOOST_PP_CAT(T,N) >::type
#define OBSERVER_TUPLE_HELPER( Z, N, _ ) \
	template< class T > \
		struct mpl_vec2tuple_helper<N,T>{ \
			typedef T param_type; \
			BOOST_PP_REPEAT( N, OBSERVER_TYPE_LINE, _ ) \
			typedef boost::tuple< BOOST_PP_REPEAT( N, OBSERVER_EVENT_TUPLE_LINE, _ ) > tuple_type; \
		};

		BOOST_PP_REPEAT( MAX_SIGNAL_FUNCTION_PARAMS, OBSERVER_TUPLE_HELPER, _ )

		struct shot_base{  
			virtual boost::function_base* get_function()=0;
			virtual ~shot_base(){} 
		}; 

		template< int i, class T > class shot_void_helper;

#define OBSERVER_SIGNAL_EVENT_IMPL( Z, N, _ ) \
	template< class T > \
		class shot_void_helper<N,T> : public shot_base \
		{ \
		public: \
			typedef typename T::param_type param_type; \
			BOOST_PP_REPEAT( N, OBSERVER_TYPE_LINE, _ ) \
			void shot( BOOST_PP_ENUM_BINARY_PARAMS(N, const T, &v ) ){ _callback( BOOST_PP_ENUM_PARAMS(N, v) ); } \
			virtual boost::function_base* get_function(){ return &_callback; } \
			shot_void_helper( const boost::function< void( BOOST_PP_ENUM_PARAMS(N, T) ) >& v ) : _callback(v){} \
		private: \
			boost::function< void( BOOST_PP_ENUM_PARAMS(N, T) ) > _callback; \
		};

		BOOST_PP_REPEAT( MAX_SIGNAL_FUNCTION_PARAMS, OBSERVER_SIGNAL_EVENT_IMPL, _ )
		/* ����ͨ�� PP �����ʺϲ�ͬ���������İ�װ��չ���Ĵ�����������:
		template< class T >
		class shot_void_helper<1,T> : public shot_base
		{
		public:
			typedef typename boost::mpl::at_c<T, 0>::type T0;
			void shot( const T0& v1 ){ _callback(v1); }
			virtual boost::function_base* get_function(){ return &_callback; }
			shot_void_helper( const boost::function< void( T0 ) >& v ) : _callback(v){}
		private:
			boost::function< void( T1 ) > _callback;
		};
		*/
	};

	/** ����һ���۲���ģʽ�ļ�ʵ�֣������ڴ���Ľ��
	* \remarks �����Ԥ�ȶ���һЩ�¼����¼��Ķ���ʹ��OBSERVER_EVENT�꣺
	*	OBSERVER_EVENT( Name, <Params> )
	*		Name ���¼�������
	*		Params �ǲ�����
	* ���� 
	*	OBSERVER_EVENT( MyEvent, int, std::string, long )
	*	 
	** ע��OBSERVER_EVENT �������ඨ����ڲ�ʹ�õġ�
	*
	* �б�Ҫ��ʱ�򣬾Ϳ��Խ�һ���ص������󶨵�����¼������ģ�
	*	 observer a;
	*	 a.subscribe<Name>( Handle );
	*		Name ���¼�������
	*		Handle �ǻص����������Ĳ�����Ӧ�ú��¼��Ĳ�����ƥ��
	* ��ȻҲ���Գ�������
	*	 a.unsubscribe<Name>();
	*		
	* ���¼�����ʱ������ͨ�� observer �����������¼�
	*	a.shot<Name>( <Params> );
	* ���⣬Ҳ�������¼����������������Ա��ӳٵ���
	*   lugce::event_base* xx=new MyEvent( "hello" );
	*   a.shot( xx );
	*	
	* ������������Ϊ����ʹ�ã��԰���������������Ƶ��ŵ����ڣ��¼��Ķ��塢�ص��Ĳ�����
	* �����ϸ�ƥ�䣬����ͻᷢ����������Է�ֹ�������
	* �ر�ģ��������Զ���Ϊ�������ͣ��Ա��ûص����������޸�������ʱ�����¼�ʱ��Ҫע��ʹ�� ref() ����װ������
	*/
	class observer
	{
	public:
#define OBSERVER_SHOT_FUNCTION(Z, N, _) \
	template<class X BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_PARAMS(N, class T)> \
	void shot( BOOST_PP_ENUM_BINARY_PARAMS(N, T, v) ) \
	{  \
		typedef typename X::param_type mpl_vec; \
		BOOST_AUTO( i, _shots.find( OBSERVER_GET_ID_BY_TYPE(typeid(X)) ) ); \
		if( i!=_shots.end() ) static_cast<details::shot_void_helper< N, X >*>(i->second)->shot( BOOST_PP_ENUM_PARAMS(N, v) ); \
	}

		BOOST_PP_REPEAT( MAX_SIGNAL_FUNCTION_PARAMS, OBSERVER_SHOT_FUNCTION, _ )

		template< class X, class HANDLE >
		void subscribe( HANDLE h )
		{
			typedef typename X::param_type mpl_vec;
			typedef details::shot_void_helper< boost::mpl::size<mpl_vec>::value, X > helper;
			assert( _shots[ OBSERVER_GET_ID_BY_TYPE(typeid(X)) ]==NULL );
			delete _shots[ OBSERVER_GET_ID_BY_TYPE(typeid(X)) ];
			_shots[ OBSERVER_GET_ID_BY_TYPE(typeid(X)) ]=new helper(h);
		}
		template< class X >
		void unsubscribe(){ unsubscribe(OBSERVER_GET_ID_BY_TYPE(typeid(X))); }
		void unsubscribe( const event_base* ev ){ unsubscribe( ev->get_id() ); }
		void unsubscribe( const OBSERVER_TYPE_ID_TYPE &id )
		{ 
			BOOST_AUTO( i, _shots.find(id) );
			if( i!=_shots.end() ){
				delete i->second;
				_shots.erase(i);
			}		
		}

		void shot( event_base* ev )
		{
			BOOST_AUTO( i, _shots.find( ev->get_id() ) ); 
			if( i!=_shots.end() ) 
				ev->shot( i->second->get_function() );
		}
		~observer()
		{
			BOOST_AUTO( i, _shots.begin() );
			BOOST_AUTO( e, _shots.begin() );
			for( ; i!=e; ++i ) delete i->second;
		}
	protected:
		std::map< OBSERVER_TYPE_ID_TYPE, lugce::details::shot_base* > _shots;
	};
};

#undef OBSERVER_EVENT_TUPLE_LINE
#undef OBSERVER_TUPLE_HELPER
#undef OBSERVER_SIGNAL_EVENT_IMPL
#undef OBSERVER_TYPE_LINE
#undef OBSERVER_SHOT_FUNCTION
#undef MAX_SIGNAL_FUNCTION_PARAMS

#define OBSERVER_EVENT_CREATOR(r, data, i, elem) BOOST_PP_COMMA_IF(i) \
	const boost::mpl::if_c< boost::is_reference<elem>::value, boost::reference_wrapper< boost::remove_reference<elem>::type >, elem >::type \
	BOOST_PP_CAT( &v, i )
#define OBSERVER_EVENT_TUPLE( ... ) lugce::details::mpl_vec2tuple_helper< \
			BOOST_PP_VARIADIC_SIZE( __VA_ARGS__ ), \
			boost::mpl::vector< __VA_ARGS__ > \
		>::tuple_type
#define OBSERVER_EVENT_HELPER_GET_LINE( Z, N, _ ) BOOST_PP_COMMA_IF(N) boost::get<N>( *this )

/// ��������������¼�
#define OBSERVER_EVENT( NAME, ... ) \
	struct NAME : public OBSERVER_EVENT_TUPLE(__VA_ARGS__), public lugce::event_base { \
		typedef OBSERVER_EVENT_TUPLE(__VA_ARGS__) tuple_type; \
		typedef boost::function< void( __VA_ARGS__ ) > function_type; \
		typedef boost::mpl::vector< __VA_ARGS__ > param_type; \
		NAME( BOOST_PP_LIST_FOR_EACH_I(OBSERVER_EVENT_CREATOR, _, BOOST_PP_VARIADIC_TO_LIST( __VA_ARGS__ ) ) ) \
		 : tuple_type( BOOST_PP_ENUM_PARAMS(BOOST_PP_VARIADIC_SIZE( __VA_ARGS__ ), v) ), event_base( OBSERVER_GET_ID_BY_TYPE(typeid(NAME) )) {} \
	private: \
		virtual void shot( boost::function_base* p ){ \
			function_type& cb=*static_cast< function_type* >(p); \
			cb BOOST_PP_LPAREN() BOOST_PP_REPEAT( BOOST_PP_VARIADIC_SIZE( __VA_ARGS__ ), OBSERVER_EVENT_HELPER_GET_LINE, _ ) BOOST_PP_RPAREN(); \
		} \
	};
