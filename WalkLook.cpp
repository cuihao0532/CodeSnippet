// WalkLook.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <math.h>
#include <iomanip>
using namespace std;

#ifndef M_PI
#define M_PI   3.14159265358979323846
#endif

double arrLine[][2] = 
{
116.359157, 40.021963,
116.359298, 40.021822,
116.359538, 40.021813,
116.359631,	40.021676,
116.359669,	40.021488,
116.35973,	40.021281,
116.359896,	40.02123,	
116.359717,	40.021182,
116.359495,	40.021169,
116.35927,	40.021136,
116.359076,	40.021123,
116.358931,	40.021269,
116.35887,	40.021439,
116.358887,	40.021591,
116.358911,	40.021745,
116.358984,	40.021880
};



/************************************************************************/
/* 平面向量的夹角
 * 向量 a = (x1, y1),  b = (x2, y2)
 * cosθ = ( (x1 * x2 ) + (y1 * y2) ) / ( sqrt(x1^2 + y1^2) * sqrt(x2^2 + y2^2) )
 * A---B---C(当前点)----D-----E---F
/************************************************************************/
void GetPoints(double pArr[][2], size_t nColumns, double dThreshhold, size_t nCountOfNext = 3)
{ 
    for (size_t i = 0; i < nColumns; ++ i)
    { 
        cout << pArr[i][0] << ", " << pArr[i][1] << endl; 
    }

    if (nColumns <= 2 )
        return;
     
    //当前直线的起点和终点
    size_t nStart = 0;
    size_t nLast  = 1;

    for (size_t i = nLast + 1; i < nColumns; )
    { 
        double xA = pArr[nStart][0];
        double yA = pArr[nStart][1];
        double xB = pArr[nLast][0];
        double yB = pArr[nLast][1];
        double xC = pArr[i][0];
        double yC = pArr[i][1];



        //向量a = (xB - xA, yB - yA) = (x1, x2)
        //向量b = (xC - xB, yC - yB) = (y1, y2)
        double x1 = xB - xA;
        double y1 = yB - yA;
        double x2 = xC - xB;
        double y2 = yC - yB;

        double dCosRad = ( x1 * x2 + y1 * y2 ) / ( sqrt(x1 * x1 + y1 * y1) * sqrt(x2 * x2 + y2 * y2));
        double dAngle = acos(dCosRad) * 180 / M_PI;
        cout << "angle = " << dAngle << endl;        

        //如果当前夹角 >= 阈值
        if (dAngle >= dThreshhold)
        {
            size_t nCount = 0;
            size_t nCurrMin = 0;

            //当前点为疑似拐点，继续判断接下来的nCountOfNext个点是否仍然为疑似拐点
            //假如nCountOfNext为3， 则计算向量AB 分别和 向量BD BE BF的夹角
            size_t index = 1;
            for ( index = 1; index <= nCountOfNext && (i + index < nColumns); ++ index)
            {
                double xIndex = pArr[i + index][0];
                double yIndex = pArr[i + index][1];
                
                double xNext = xIndex - xB;
                double yNext = yIndex - yB;

                dCosRad = ( x1 * xNext + y1 * yNext ) / ( sqrt(x1 * x1 + y1 * y1) * sqrt(xNext * xNext + yNext * yNext) );
                dAngle = acos(dCosRad) * 180 / M_PI;
                cout << "---- angle = " << dAngle << endl;

                if (dAngle < dThreshhold)
                {
                    nCurrMin = i + index;
                    break;
                }
                else
                {
                    //计算后续的nCountOfNext个点和当前点构成的向量 和 上一条直线向量 的夹角超过门限值的次数
                    ++ nCount; 
                }
 
            } //for
 
            //是否用完全部GPS数据点
            if (i + index >= nColumns)
            {
                break;
            }

            //判断是否为真的拐点

            //如果后续的nCountOfNext个向量都为疑似拐弯
            if (nCount >= nCountOfNext)
            {
                //更新起点为上一条终点
                nStart = nLast;     

                //更新终点为nCountOfNext中最后一个疑似拐点
                nLast = i + nCount;

                cout << "****** Point index is " << nStart << endl;
            }
            else
            {
                nLast = nCurrMin;
            }

        } //if (dAngle >= dThreshhold)
        else
        {
            nLast = i;
        }

        i = nLast + 1;

    } //for


}


int _tmain(int argc, _TCHAR* argv[])
{
    GetPoints(arrLine, sizeof(arrLine) / sizeof(double) / 2, 10.0 );
	return 0;
}

