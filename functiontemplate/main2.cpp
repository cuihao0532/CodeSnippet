

//使用分别编译模型函数模版

#include <iostream>
#include "functemp_fenbie.h"
using namespace std;

int main()
{
	int a = 1000;
	int b = 2000;

	int nSum = Add(a, b);
	cout << endl << nSum << endl;

	return 0;
}
