// SPLine.cpp: implementation of the PtSPLine class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "SPLine.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


PtSPLine::PtSPLine()
{

}

PtSPLine::~PtSPLine()
{

}

float PtSPLine::GetBezierCurve(float *ctr, float t)
{
	int NumCtlPoints = 4;
	float c[4];
	int k; 
	INT32 n, choose=0;
	float result;

	n = NumCtlPoints - 1;

	for(k=0; k<= n; k++)
	{
		if(k == 0) choose = 1;
		else if(k == 1) choose = n;
		else choose = choose * (n-k+1)/k;
		
		c[k] = *(ctr+k)*(float)choose;
	}

	float t1, tt, u;
	float b[4];

	n = NumCtlPoints - 1; 
	u = t;

	b[0] = c[0];

	for(k=1; k<=n; k++)
	{
		b[k] = c[k] * u;
		u = u * t;
	}

	result = b[n];
	t1 = (float)1 - t;			
	tt = t1;

	for(k=(int)n-1; k>=0; k--)
	{
		result += b[k] * tt;
		tt = tt*t1;
	}

	return result;
}

PtVector3 PtSPLine::GetBezierCurve(PtVector3* point, float t)
{
	PtVector3 ret;
	float a[4];
	for (int i = 0; i < 3; i++)
	{
		for (int c = 0; c < 4; c++)
			a[c] = point[c][i];

		ret[i] = PtSPLine::GetBezierCurve(a, t);
	}
	return ret;
}

float PtSPLine::GetCommonBezierCurve(float fDif, float t)
{
	float	fPoints[4];
	fPoints[0]	= (float)0;
	fPoints[1]	= fDif;
	fPoints[2]	= (float)1 - fDif;
	fPoints[3]	= (float)1;

	return GetBezierCurve(&fPoints[0], t);
}

PtVector3 PtSPLine::GetCommonBezierCurve(PtVector3 &v3From, PtVector3 &v3To, float fDif, float t)
{
	PtVector3	v3Ret;
	float result	= 0;

	float	fPoints[4];
	fPoints[0]	= (float)0;
	fPoints[1]	= fDif;
	fPoints[2]	= (float)1 - fDif;
	fPoints[3]	= (float)1;

	result	= GetCommonBezierCurve(fDif, t);

	v3Ret	= (v3To - v3From)*result + v3From;

	return v3Ret;
}

void PtSPLine::Make2ExtCtrPoint(float *xpfTimes, float *xpfValues, float *xpfRets)
{
	PtVector3	v3Forward, v3Backward, v3Average;

	v3Forward.SetValue( *(xpfTimes+2) - *(xpfTimes+1), *(xpfValues+2) - *(xpfValues+1), 0);
	v3Backward.SetValue( *(xpfTimes+1) - *(xpfTimes+0), *(xpfValues+1) - *(xpfValues+0), 0);

	float xDbLength;
	float x1, x2;

	x1 = v3Backward.x/(float)-3;
	x2 = v3Forward.x/(float)3;

	xDbLength = v3Forward.DoubleLength();
	v3Forward /= xDbLength;
	xDbLength = v3Backward.DoubleLength();
	v3Backward /= xDbLength;

	v3Average = v3Forward + v3Backward;
	
	*(xpfRets+1) = (x2/v3Average.x)*v3Average.y + *(xpfValues+1);
	*xpfRets = (x1/v3Average.x)*v3Average.y + *(xpfValues+1);
}