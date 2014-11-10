//函数对象做形参

#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

class GT_cls
{
public:
	GT_cls(size_t val);
	bool operator()(const int& val);

protected:
	int m_nBound;
};

GT_cls::GT_cls(size_t val) : m_nBound(val){}
bool GT_cls::operator()(const int& val)
{
    return (val >= m_nBound);
}


int main()
{
	vector<int> iVec;
	for(int i = 0; i < 10; ++ i)
		iVec.push_back(i);

	int nVal = 7;
	cout << endl << "output the counts of elements in iVec which element is  bigger or equal than " << nVal << endl;
    cout << endl << count_if(iVec.begin(), iVec.end(), GT_cls(nVal));

	

	cout << " hello world" << endl;
	return 0;
}

