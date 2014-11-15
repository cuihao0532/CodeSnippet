#include <iostream>
#include <stdexcept>
#include <string.h>
using namespace std;

template<typename T, size_t N>
void ShowArray(T (&arr)[N])
{
	for(size_t i = 0; i < N - 1; ++ i)
	{
		cout << arr[i] << ", ";
	}
	
	if (N > 0)
		cout << arr[N - 1] << endl;
}

template<typename T>
class CObj
{
	public:
		CObj() : m_N(0), m_pPtr(0) {}
		CObj(size_t n) : m_N(n), m_pPtr(new T[n]) {}
		CObj(T* ptr, size_t n) : m_N(n)
		{
			m_pPtr = new T[n];
			if(0 != m_pPtr)
			{
				//errno_t err = memcpy_s(ptr, n * sizeof(T), ptr, n * sizeof(T));
				memcpy(m_pPtr, ptr, sizeof(T) * n);
			}
				
		}

		virtual ~CObj() { if(0 != m_pPtr) delete[] m_pPtr; m_pPtr = 0; m_N = 0; } 
		friend 	ostream& operator<< (ostream& os, CObj& obj)
		{
			for(size_t i = 0; i < obj.m_N - 1; ++ i)
			{
				os << obj.m_pPtr[i] << ", ";
			}

			if(obj.m_N > 0)
				os << obj.m_pPtr[obj.m_N - 1] << endl;

			return os;
		}

	protected:
		size_t m_N;
		T* m_pPtr;	
};

int main()
{
	int arr1[10];
	for(int i = 0; i < sizeof(arr1) / sizeof(arr1[0]); ++ i)
	{
		arr1[i] = i * i;
	}

	ShowArray(arr1);	

	CObj<int> arr2(arr1, sizeof(arr1) / sizeof(arr1[0]));
	CObj<int> arr3(arr1, sizeof(arr1) / sizeof(arr1[0]));

	CObj<int> arrs[] = {arr2, arr3};
	ShowArray(arrs);


	return 0;

}
