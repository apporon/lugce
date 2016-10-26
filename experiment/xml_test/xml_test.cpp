// xml_test.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "xml_test.h"
//#define XML_STEP 1

#include <memory>
#include <boost/function.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "lugce/xml/simple_sax.hpp"
#include "lugce/xml/xml_writer.hpp"
#include "lugce/xml/dom.hpp"


/// ����Ŀ¼strPath�е������ļ�
int GetAllFile(const std::string &strPath)
{
	namespace fs = boost::filesystem;	
	// �õ������ļ���.	
	fs::path full_path( fs::initial_path() );
	full_path = fs::system_complete( fs::path(strPath, fs::native ) );	
	unsigned long file_count = 0;
	unsigned long dir_count = 0;
	unsigned long err_count = 0;	
	if ( !fs::exists( full_path ) )
	{
		std::string strMsg = "�Ҳ����ļ�Ŀ¼,�����Ŀ¼�Ƿ����: ";
		strMsg.append(full_path.native_file_string());	 return -1;
	}	// ����ָ�����ļ���,�õ����е��ļ���.
	if ( fs::is_directory( full_path ) )
	{
		fs::directory_iterator end_iter;
		for( fs::directory_iterator dir_itr( full_path ); dir_itr != end_iter; ++dir_itr ){
			try
			{
				if ( fs::is_directory( *dir_itr ) )
				{
					std::string strSubDir(full_path.native_directory_string()) ;
					strSubDir.append( "\\ ");
					strSubDir.append(dir_itr-> leaf());	
					GetAllFile(strSubDir);	// �������Ŀ¼,��ݹ����.
				}
				else
				{
					// ����ɰ�������·�����ļ���
					std::string strFileName(full_path.native_directory_string());
					strFileName.append( "\\");
					strFileName.append(dir_itr-> leaf());
					//�ж��ļ��Ƿ�ΪXML�����ļ�
					if( strFileName.substr( strFileName.size()-4 )==".xml" ){
						try{
							std::cout << "����:" << strFileName << std::endl;
							std::ifstream f( strFileName );
							lugce::xml::document dx( f );
							std::cout << "���:" << std::endl;
							std::cout << dx.xml() << std::endl << std::endl;
						}catch( std::exception& e ){
							std::cout << "��������:" << e.what() << std::endl;
						}
					}

					
					fs::path full_file( fs::initial_path() );
					full_file = fs::system_complete(fs::path(strFileName, fs::native));	 //���ؽ����ļ��е���Ϣ.
					//readFileInfo(full_file.native_directory_string());
				}
			}
			catch ( const std::exception & ex )
			{
				++err_count;	 
			}
		}// <--for()
		std::cout <<"�ɹ����������豸�����ļ�. ";
	}
	else // must be a file
	{ 
		std::string strMsg = full_path.native_file_string();
		strMsg.append( ",�����ļ�Ŀ¼. ");
		return -1;
	}	
	return err_count;
}

int main(int _Argc, char ** )
{
	using namespace std;
	using namespace lugce::xml;

	GetAllFile(".");
#ifndef _DEBUG
	//system( "pause" );
#endif
	return 0;
}