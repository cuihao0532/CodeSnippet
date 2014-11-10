#include <iostream>
using namespace std;

class CCalu
{
	public:
		CCalu(int a);
		CCalu(const CCalu& obj);
		virtual ~CCalu();
		void Show();
		CCalu& operator++();    //前置++
		CCalu  operator++(int); //后置++

	protected:
		int m_nValue;
};


CCalu::CCalu(int a) : m_nValue(a){}
CCalu::~CCalu(){}
void CCalu::Show()
{
	cout << "-------- " << m_nValue << " ------------" << endl;
}

CCalu::CCalu(const CCalu& obj)
{
	this->m_nValue = obj.m_nValue;
}

//前置++ 返回引用
CCalu& CCalu::operator++()
{
   ++ (this->m_nValue);
   return *this; 
}


//后置++ 返回旧值
CCalu CCalu::operator++(int a)
{
	CCalu obj(*this);
	++ (this->m_nValue);
	return obj;	
}

int main()
{
    CCalu obj1(100);
	++obj1;
	obj1.Show();

	return 0;
}














