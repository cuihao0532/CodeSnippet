
//使用包含编译的类模版

#include <iostream>
#include "./classtemp_baohan.h"
using namespace std;

int main()
{
	float a = 100.0F;
	float b = 200.578F;
	CCalculate<float> obj(a, b);
	
	cout << endl << obj.Add() << endl;

	return 0;
}
