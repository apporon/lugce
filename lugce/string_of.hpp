/*
 * Copyright (c) 2008
 * ��������Ȩ����( All rights reserved. )
 * 
 * �ļ����ƣ�string_of.hpp
 * �ļ���ʶ��
 * ժ    Ҫ��
 * 
 * ��ǰ�汾��1.0
 * ��    �ߣ����
 * ������ڣ�2008��7��18��
 *
 */
#if !defined( __STRING_OF_HPP_7B6D6375_E63F_4847_955D_38C6D2228E09__ )
#define __STRING_OF_HPP_7B6D6375_E63F_4847_955D_38C6D2228E09__

#include <string>
namespace lugce
{
	template< typename _CHAR >
	struct string_of
	{
		typedef std::basic_string<_CHAR, std::char_traits<_CHAR>, std::allocator<_CHAR> > type;
	};
}


#endif //__STRING_OF_HPP_7B6D6375_E63F_4847_955D_38C6D2228E09__
