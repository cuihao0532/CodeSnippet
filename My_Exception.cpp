// Exception.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <stdexcept>
using namespace std;


class My_Exception 
{
public:
		
	//construct
	My_Exception() : m_pMsg(0) {}

	//construct
	My_Exception(const char* pMsg) 
	{
		size_t len = strlen(pMsg);
		m_pMsg = new char[len + 1]();
		if (0 == m_pMsg)
		{
			throw logic_error("alloc memory failed !");
		}
		strcpy_s(m_pMsg, sizeof(char) * (len + 1), pMsg);
	}

	//copy construct
	My_Exception(const My_Exception& excep)
	{
		size_t len = strlen(excep.m_pMsg);
		this->m_pMsg = new char[len + 1]();
		if (0 == this->m_pMsg)
		{
			throw logic_error("copy construct alloc memory failed !");
		}

		strcpy_s(this->m_pMsg, sizeof(char) * (len + 1), excep.m_pMsg);
	}

	//operator=
	My_Exception& operator=(const My_Exception& excep)
	{
		if (this != &excep)
		{
			size_t len = strlen(excep.m_pMsg);
			this->m_pMsg = new char[len + 1]();
		    if (0 == this->m_pMsg)
			{
				throw logic_error("copy construct alloc memory failed !");
			}

			strcpy_s(this->m_pMsg, sizeof(char) * (len + 1), excep.m_pMsg);	
		}

		return *this;
	}

	virtual const char* What() const
	{
		return m_pMsg;
	}

	//destruct
	virtual ~My_Exception()
	{
		if(0 == m_pMsg)
			delete[] m_pMsg;
		m_pMsg = 0;
	}


protected:
	char* m_pMsg;

};
template<class T>
auto Devide(const T& a, const T& b) ->decltype(a / b)
{
	if (0 == b || ( b - 0.0 <= 0.000001 && b - 0.0 >= -0.000001 ))
	{
		throw My_Exception("dev ZERO error");
	}

	return a / b;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int a = 100;
	int b = 0;

    int nResult = 0;

	try
	{
		nResult = Devide(a, b);
	}
	catch (const My_Exception& except)
	{
		cout << except.What() << endl;		
	}
	catch (...)
	{
		cout << "Some exception happened \n";
	}

	cout << nResult << endl;


	return 0;
}

