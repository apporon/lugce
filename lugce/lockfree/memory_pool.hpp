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

#pragma once
#include <exception>
#include "../../libs/shared_ptr.hpp"
#include "atomic.hpp"

/// C++ �� lugce �����ֿռ�
namespace lugce
{
	namespace lockfree
	{
		namespace details
		{	/// \private
			class memory_pool_base
			{
				static const int barrel=16;
			public:
				memory_pool_base( const int osize, const size_t bsize )
					: _objsize(osize), _alloc_mark(0),_dealloc_mark(7)
				{
					_blocksize=(bsize/barrel);
					if( _blocksize < 256 ) _blocksize=256;
					for( int i=0; i<barrel; ++i ){
						_first_block[i]=tadem_block(i);
						_free_head[i]=reinterpret_cast<intptr_t>(_first_block[i])+sizeof(intptr_t)+_objsize;	// ָ������ͷ
					}
				}
			
				~memory_pool_base()
				{
					// �ͷ��ڴ��
					for( int i=0; i<barrel; ++i ){
						char * next=_first_block[i];
						do{
							char *p=next;
							intptr_t x=*(intptr_t*)p;
							next=(char*)x;
							delete[] p;
						}while(next);
					}
				}
			protected:
				/// \~chinese	�����µ��ڴ��
				/// \~english	Create a new block of the memory.
				void create_new_block( int idx )
				{
					atomic_intptr& head=_free_head[idx];
					char *block=tadem_block( idx );	// �����ڴ�
					atomic_intptr_t *p=(atomic_intptr_t*)(_first_block[idx]);

					// ���Թҽӵ��ڴ������
					while( p->data )	// �ҵ�����β
						p=(atomic_intptr_t*)(p->data);	// �ƶ���������һλ

					while( !atomic_compare_and_set( *p, 0, intptr_t(block) ) ){
						p=(atomic_intptr_t*)(p->data);	// �ƶ���������һλ
					}

					p=(atomic_intptr_t*)( block+sizeof(intptr_t) );	// �� p ָ������β��
					// ���Թҽӵ������ڴ�ջͷ��
					intptr_t old;
					intptr_t nval;
					
					do{
						old=head.data;
						p->data=old;		// ������βָ��ǰβ	
						intptr_t x=*(intptr_t*)(p->data);
						nval=(intptr_t)(block+sizeof(intptr_t)+_objsize);	// �µ��¿����ָ�򱾿�
					} while( !head.compare_and_set( old, nval ) );
				}

				/// \~chinese	�����ڴ棬�����ڴ洮��Ϊ����
				char* tadem_block( const int idx )
				{
					char *block=new char[ _blocksize * _objsize+sizeof(intptr_t)];	// ׼��һ���ڴ棬ע�� new �����׳��쳣
					char *p=block;
					*reinterpret_cast<intptr_t*>(p)=0;	// �ڴ��ͷ�Ƕ���ģ���������������һ���ڴ�ĵ�ַ���Թ����ڴ�����������ڴ������ʱ�ͷ��ڴ�飩
					p+=sizeof(intptr_t);
					*reinterpret_cast<intptr_t*>(p)=0;	// ���������ֽڣ�ͬ���Ƕ���ģ���Ϊ�����β��
					p+=_objsize;
					// ������ڴ���������
					for( uint32 i=0; i<_blocksize-2; ++i ){
						*reinterpret_cast<intptr_t*>(p)=reinterpret_cast<intptr_t>(p)+_objsize;	// ���ݳ�Ϊָ����һ��Ŀ��е�Ԫ��ָ��
						p+=_objsize;
					}
					// ����ָ��ָ�����涨���β��
					*reinterpret_cast<intptr_t*>(p)=reinterpret_cast<intptr_t>(block)+sizeof(intptr_t);		// ���һ��ָ��β�ڵ�
					return block;
				}
			public:
				/// \~english	Allocate a block of memory for one element.
				/// \~chinese	������Դ��һ��Ԫ�ص��ڴ档
				/// \return 	һ��ָ�����ڴ��ָ�롣
				/// \note		ͬ STL ��׼��ͬ������ڴ����ʧ�ܣ����᷵�ؿ�ָ�룬������ new �׳� std::bad_alloc ������
				void* allocate()
				{
					// ���ԴӶ�ջ�е���һ����������
					intptr_t nval;
					intptr_t old;
					
					const int idx= _alloc_mark++ & 0xF; //barrel; 
					//atomic_increment( g_alloc_count + idx );
					atomic_intptr& head=_free_head[idx];
					do{
						old=head.data;
						intptr_t *next=(intptr_t*)old;	// ָ����һ����е�λ��ָ��
						nval=*next;
						if( 0==nval ){	// û�п��У���Ҫ�����¿�
							// �����¿�
							create_new_block( idx );
							// goto label;
							continue;
						}
					}while( !head.compare_and_set( old, nval ) );
					return (void*)old;
				}

				/// \~english
				/// \brief	Free the memory of the object.
				/// \~chinese	
				/// \brief	�ͷŶ����ڴ�
				void deallocate( void* ptr )
				{
					intptr_t *p=(intptr_t*)ptr;
					intptr_t nval;
					intptr_t old;
					// ���Խ���Ż�����
					const int idx= _dealloc_mark++ & 0xF;// barrel; 
					//atomic_increment( g_dealloc_count + idx );
					atomic_intptr& head=_free_head[idx];
					do{
						old=head.data;
						*p=old;	// �����ݸ�Ϊ��һ����������
						nval=(intptr_t)ptr;
					}while( !head.compare_and_set( old, nval) );
				}
			protected:
				atomic_intptr _free_head[barrel];	// ��һ�����п��ָ��
				atomic_int32 _alloc_mark;		
				atomic_int32 _dealloc_mark;
				uint32 _blocksize;					// ÿ���ڴ�Ŀ��Դ�ŵĶ�����
				char * _first_block[barrel];		// �׿��ڴ�
				const int _objsize;					// ����Ĵ�С
			};
		}

		/// \~english 
		/// \brief	Lock-free memory pool
		/// \~chinese 
		/// \brief		��Ч�������ڴ��
		/// \remarks	��ֱ��ʹ�� new ��Ч�ʸ�100�����ϡ�
		template< typename T >
		class memory_pool 
		{
		private:
			std::shared_ptr< details::memory_pool_base > _pool;
		public:	// Typedef
			typedef T			value_type;
			typedef T*			pointer;
			typedef const T*	const_pointer;
			typedef T&			reference;
			typedef const T&	const_reference;
			typedef size_t		size_type;
			typedef ptrdiff_t	difference_type;

			/// \private
			/// \~english 
			/// \brief	A structure that enables an allocator for objects of one type to allocate storage for objects of another type.
			/// \~chinese 
			/// \brief	ʹ����������ܹ�������һ�����͵ķ������Ľṹ��
			template<class _Other>
			struct rebind
			{	
				typedef memory_pool<_Other> other;
			};
		public:	//
			void show_count( ){ _pool->show_count(); }
			/// \~chinese 
			/// \brief	���캯��
			/// \param bsize ÿ�η�����ڴ����Է�����ٸ�����
			memory_pool( const size_t bsize=1024 * 16 ) 
				: _pool( new details::memory_pool_base(
					sizeof(T) < sizeof(intptr_t) ? sizeof(intptr_t) : sizeof(T),
					(bsize < 8 ? 8 : bsize) 
					) )
			{
				
			}
		public:	// Interface for STL	// STL �ӿ�
			/// \~english 
			/// \brief	Finds the address of an object whose value is specified.
			/// \~chinese
			/// \brief	��ȡָ������ĵ�ַ��
			pointer address( reference _Val	) const{ return &_Val; }
			/// \~english 
			/// \brief	Finds the address of an object whose value is specified.
			/// \~chinese
			/// \brief	��ȡָ������ĵ�ַ��
			const_pointer address( const_reference _Val	) const{ return &_Val; }

			/// \~english
			/// Allocates a block of memory large enough to store at least some specified number of elements.
			/// \param count The number of elements for which sufficient storage is to be allocated.
			/// \param _hint <i>the parameter is useless.</i>
			/// \~chinese
			/// ����һ���㹻���ָ����Ŀ������ڴ档
			/// \param count �㹻���ָ������Ԫ�ص��ڴ潫�����䡣
			/// \param _hint <i>���������õġ�</i>
			/// \note	�������Ϊ�˼��� STL ���ṩ�ķ�����ֻ�д���� count Ϊ1ʱ�Ż�����޲�����allocte����ͨ���ڴ��
			///			�����ڴ棬count��Ϊ1ʱ��ʹ�� new �������ڴ棬Ϊ��Ч�ʿ��ǣ��뾡��ֱ�ӵ����޲����� allocte ������
			///			���� vector��deque �����������ڴ�ʱ������һ��һ������ģ���˲��ʺ�ʹ������ڴ�ء�
			/// \sa allocate()
			pointer allocate( size_type count=1, const void* _hint=NULL )
			{
				assert( count==1 );
				if( 1==count )
					return pointer( _pool->allocate() );
				else{
					return new T[count];
				}
			}

			/// \~english
			/// Frees a specified number of objects from storage beginning at a specified position.
			/// \~chinese
			/// �ͷ���ָ��λ�õĶ���Ķ���
			/// \param ptr		Ҫ���ͷŵĶ������ָ�롣
			/// \param count	Ҫ�ͷŵĶ������� ��
			/// \note deallocate ����� allocate ���ʹ�ã�count ����������ͷ���ʱ��������ͬ��
			/// \sa deallocate(pointer)
			void deallocate( pointer ptr, size_type count=1 )
			{
				assert( count==1 );
				if( count==1 )
					_pool->deallocate( ptr );
				else
					delete[] ptr;
			}

			/// \~english
			/// \brief	Constructs a specific type of object at a specified address that is initialized with a specified value.
			/// \~chinese
			/// \brief	��ָ��λ����ָ��ֵ�������
			void construct( pointer _Ptr, const value_type& _Val )
			{
				new (_Ptr) T( _Val );
			}

			/// \~chinese
			/// \brief	��ָ��λ����ָ��ֵ���������ֵ���ð汾��
			template<class Types>
			void construct(	pointer ptr, Types &&val )
			{
				new (ptr) T( std::forward<value_type>( val ) );
			}

			/// \~english
			/// \brief	Calls an objects destructor without deallocating the memory where the object was stored.
			/// \~chinese	
			/// \brief	���ö�����������������ͷ��ڴ档
			void destroy( pointer ptr )
			{
				ptr->~value_type();
			}

			/// \~english	
			/// \brief	Returns the number of elements of type Type that could be allocated by an object of 
			///			class allocator before the free memory is used up. 
			/// \~chinese	
			/// \brief	���ؿ��Է���Ķ������������
			size_type max_size( ) const
			{	// estimate maximum array size
				size_type _Count = (size_type)(-1)/sizeof(value_type);
				return (0 < _Count ? _Count : 1);
			}
		};
	};
};