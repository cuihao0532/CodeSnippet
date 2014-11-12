#include <iostream>
#include <stdexcept>
using namespace std;

class CBase
{
	public:
		CBase(){}
		virtual ~CBase(){}
		CBase(const CBase& obj){ }
		CBase& operator=(const CBase& obj)
		{
			if(this != &obj)
			{
				//todo 
			}	

			return *this;
		}
	    virtual void Show() {cout << "CBase::Show" << endl; }
		virtual CBase* Clone() const  { return new CBase(*this); }	
};

class CDerived1 : public CBase
{
	public:
		CDerived1() : CBase() {}
		virtual ~CDerived1() { }
		CDerived1(const CDerived1& obj) : CBase(obj) { }
		CDerived1& operator=(const CDerived1& obj)
		{
			if(this != &obj)
			{
				//todo
				CBase::operator=(obj);
			}
			return *this;
		}
		
		virtual void Show() { cout << "CDerived1::Show()" << endl; }
		virtual CDerived1* Clone() const { return new CDerived1(*this); }	
};


class CDerived2 : public CBase
{
	public:
		CDerived2() : CBase() {}
		virtual ~CDerived2() {}
		CDerived2(const CDerived2& obj) : CBase(obj) {}
		CDerived2& operator=(const CDerived2& obj)
		{
			if(this != &obj)
			{
				//todo
				CBase::operator=(obj);
			}

			return *this;
		}
		virtual void Show() { cout << "CDerived2::Show()" << endl; }
		virtual CDerived2* Clone() const { return new CDerived2(*this); }
};


class CAutoPtr
{
	public:
		//construct
		CAutoPtr() : m_pPtr(0), m_pUse(new std::size_t(1)) { }

		//construct
		CAutoPtr(const CBase& obj) : m_pPtr(obj.Clone()), m_pUse(new std::size_t(1)) {}

		//destruct
		virtual ~CAutoPtr() { Decr_use(); }

		//copy construct
		CAutoPtr(const CAutoPtr& ptr) : m_pPtr(ptr.m_pPtr), m_pUse(ptr.m_pUse) { ++*m_pUse; }

		//operator=
		CAutoPtr& operator=(const CAutoPtr& ptr) 
		{
			++(*ptr.m_pUse); //right val ref add 1
			Decr_use();
			m_pPtr = ptr.m_pPtr;
			m_pUse = ptr.m_pUse;	
		}

		void Decr_use()
		{
			if(--*m_pUse == 0)
			{
				delete m_pPtr;
				m_pPtr = 0;
				delete m_pUse;
				m_pUse = 0;	
			}
		}

		const CBase* operator->() const 
		{
			if(m_pPtr)
				return m_pPtr;
			else
				throw std::logic_error("error");
		}

		const CBase& operator*() const
		{
			if(m_pPtr)
				return *m_pPtr;
			else
				throw std::logic_error("error");
		
		}

	protected:
		CBase* m_pPtr;
		std::size_t* m_pUse;
};



int main()
{
	CAutoPtr ptr1;
	CDerived1 d1;

	CAutoPtr ptr2(d1);
	
	ptr1 = ptr2;

	return 0;





}









