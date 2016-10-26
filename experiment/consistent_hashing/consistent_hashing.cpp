// consistent_hashing.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <boost/thread.hpp>
#include "consistent_hashing.hpp"



int main(int argc, char* argv[])
{
	lugce::consistent_hashing_container< int > chc;
	assert( chc.set( "Hello", 1 ) );
	assert( chc.set( "World", 2 ) );
	assert( chc.set( "Friend", 3 ) );
	assert( chc.set( "1Friend", 4 ) );

	int i=chc.get("Hello");

	return 0;
}

