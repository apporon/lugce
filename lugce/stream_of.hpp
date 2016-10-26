/*
 * Copyright (c) 2008
 * ��������Ȩ����( All rights reserved. )
 * 
 * �ļ����ƣ�stringstream_of.hpp
 * �ļ���ʶ��
 * ժ    Ҫ��
 * 
 * ��ǰ�汾��1.0
 * ��    �ߣ����
 * ������ڣ�2008��7��18��
 */
#if !defined( __STREAM_OF_HPP_2E4C4079_069E_458B_8026_4771F060A3A0__ )
#define __STREAM_OF_HPP_2E4C4079_069E_458B_8026_4771F060A3A0__
#if _MSC_VER > 1000
#pragma once
#endif

#include <ostream>
#include <istream>
namespace lugce
{
	// istream
	template< typename _CHAR >
	struct istream_of
	{
		typedef std::basic_istream<_CHAR, std::char_traits<_CHAR> > type;
	};

	// ostream
	template< typename _CHAR >
	struct ostream_of
	{
		typedef std::basic_ostream<_CHAR, std::char_traits<_CHAR> > type;
	};
}




#endif //__STREAM_OF_HPP_2E4C4079_069E_458B_8026_4771F060A3A0__
