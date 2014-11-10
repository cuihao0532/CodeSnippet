#include <iostream>
using namespace std;

class CCalu
{
	public:
		CCalu(int a);
		CCalu(const CCalu& obj);
		virtual ~CCalu();
		void Show();
		CCalu& operator++();    //ǰ��++
		CCalu  operator++(int); //����++

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

//ǰ��++ ��������
CCalu& CCalu::operator++()
{
   ++ (this->m_nValue);
   return *this; 
}


//����++ ���ؾ�ֵ
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














