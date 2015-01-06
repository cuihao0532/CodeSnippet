// WalkLook.cpp : �������̨Ӧ�ó������ڵ㡣
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
/* ƽ�������ļн�
 * ���� a = (x1, y1),  b = (x2, y2)
 * cos�� = ( (x1 * x2 ) + (y1 * y2) ) / ( sqrt(x1^2 + y1^2) * sqrt(x2^2 + y2^2) )
 * A---B---C(��ǰ��)----D-----E---F
/************************************************************************/
void GetPoints(double pArr[][2], size_t nColumns, double dThreshhold, size_t nCountOfNext = 3)
{ 
    for (size_t i = 0; i < nColumns; ++ i)
    { 
        cout << pArr[i][0] << ", " << pArr[i][1] << endl; 
    }

    if (nColumns <= 2 )
        return;
     
    //��ǰֱ�ߵ������յ�
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



        //����a = (xB - xA, yB - yA) = (x1, x2)
        //����b = (xC - xB, yC - yB) = (y1, y2)
        double x1 = xB - xA;
        double y1 = yB - yA;
        double x2 = xC - xB;
        double y2 = yC - yB;

        double dCosRad = ( x1 * x2 + y1 * y2 ) / ( sqrt(x1 * x1 + y1 * y1) * sqrt(x2 * x2 + y2 * y2));
        double dAngle = acos(dCosRad) * 180 / M_PI;
        cout << "angle = " << dAngle << endl;        

        //�����ǰ�н� >= ��ֵ
        if (dAngle >= dThreshhold)
        {
            size_t nCount = 0;
            size_t nCurrMin = 0;

            //��ǰ��Ϊ���ƹյ㣬�����жϽ�������nCountOfNext�����Ƿ���ȻΪ���ƹյ�
            //����nCountOfNextΪ3�� ���������AB �ֱ�� ����BD BE BF�ļн�
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
                    //���������nCountOfNext����͵�ǰ�㹹�ɵ����� �� ��һ��ֱ������ �ļнǳ�������ֵ�Ĵ���
                    ++ nCount; 
                }
 
            } //for
 
            //�Ƿ�����ȫ��GPS���ݵ�
            if (i + index >= nColumns)
            {
                break;
            }

            //�ж��Ƿ�Ϊ��Ĺյ�

            //���������nCountOfNext��������Ϊ���ƹ���
            if (nCount >= nCountOfNext)
            {
                //�������Ϊ��һ���յ�
                nStart = nLast;     

                //�����յ�ΪnCountOfNext�����һ�����ƹյ�
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

