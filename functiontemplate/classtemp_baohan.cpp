
//包含编译模型的类模版实现

template<class T>
class CCalculate
{
	public:
		CCalculate(T a, T b) : m_A(a), m_B(b) {}
		T Add() const
		{
			return m_A + m_B;
		}

	protected:
		T m_A;
		T m_B;
};


