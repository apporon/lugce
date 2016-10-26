/*
 * Copyright (c) 
 * ��������Ȩ����( All rights reserved. )
 * 
 * �ļ����ƣ�stringstream_of.hpp
 * �ļ���ʶ��
 * ժ    Ҫ��
 * 
 * ��ǰ�汾��1.0
 * ��    �ߣ����
 * ������ڣ�2008��7��18��
 *
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
 */
#if !defined( __STRINGSTREAM_OF_HPP_7715DC23_271A_4F86_BD97_1B348D92822C__ )
#define __STRINGSTREAM_OF_HPP_7715DC23_271A_4F86_BD97_1B348D92822C__
#if _MSC_VER > 1000
#pragma once
#endif

#include <sstream>
namespace lugce
{
	// istringstream
	template< typename _CHAR >
	struct istringstream_of;

	template<>
	struct istringstream_of<char>
	{
		typedef std::istringstream type;
	};
	template<>
	struct istringstream_of<wchar_t>
	{
		typedef std::wistringstream type;
	};

	// ostringstream
	template< typename _CHAR >
	struct ostringstream_of;

	template<>
	struct ostringstream_of<char>
	{
		typedef std::ostringstream type;
	};
	template<>
	struct ostringstream_of<wchar_t>
	{
		typedef std::wostringstream type;
	};

	// stringstream
	template< typename _CHAR >
	struct stringstream_of;

	template<>
	struct stringstream_of<char>
	{
		typedef std::stringstream type;
	};
	template<>
	struct stringstream_of<wchar_t>
	{
		typedef std::wstringstream type;
	};
}

#endif //__STRINGSTREAM_OF_HPP_7715DC23_271A_4F86_BD97_1B348D92822C__
