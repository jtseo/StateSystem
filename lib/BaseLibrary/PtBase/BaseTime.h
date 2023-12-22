#pragma once
#include "Fixed.h"

class BaseTime
{
public:
	BaseTime(void);
	~BaseTime(void);

	static SPtDate get_lunar(const SPtDate &_dtSolar);
	static int day(int _nDate)
	{
		SPtDate date;
		date.date = _nDate;
		return date.s.day;
	}

	static int month(int _nDate)
	{
		SPtDate date;
		date.date = _nDate;
		return date.s.month;	
	}

	static int year(int _nDate)
	{
		SPtDate date;
		date.date = _nDate;
		return date.s.year;
	}

	static int date(int y, int m, int d)
	{
		SPtDate date;
		date.s.year		= (short)y;
		date.s.month	= (char)m;
		date.s.day		= (char)d;
		return date.date;	
	}

	static int dayofweek(int _nDate)
	{
		SPtDate date;
		date.date	= _nDate;
		return dayofweek((int)date.s.year, (int)date.s.month, (int)date.s.day);
	}

	static int dayofweek(int y, int m, int d)	/* 0 = Sunday */
	{
		// http://bytes.com/groups/c/517828-day-week-algorithm
		static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
		y -= m < 3;
		return (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
	}

	static int day_increase(int _nDate)
	{
		SPtDate	date;
		date.date	= _nDate;

		date.s.day++;

		if(date.s.day > dayinmonth[date.s.month-1])
        {date.s.day=1; month_inc(&date);}

		return date.date;
	}

	static int day_decrease(int _nDate)
	{
		SPtDate	date;
		date.date	= _nDate;

		date.s.day--;

		if(date.s.day < 1)
		{
			date.s.month--;
			if(date.s.month < 1)
			{
				date.s.year--;
				date.s.month	= 12;
			}
			date.s.day	= (char)dayinmonth[date.s.month-1];
		}

		return date.date;
	}

	static void month_inc(SPtDate *_pDate)
	{
		_pDate->s.month++;
		if(_pDate->s.month > 12)
        {_pDate->s.month=1; _pDate->s.year++;}
	}

	static void day_inc(SPtDate *_pDate)
	{
		_pDate->s.day++;

		if(_pDate->s.day > dayinmonth[_pDate->s.month-1])
        {_pDate->s.day=1; month_inc(_pDate);}
	}

	static void hour_inc(SPtDateTime *_pstDT)
	{
		_pstDT->s.sTime.s.nHour++;
		if(_pstDT->s.sTime.s.nHour >= 24)
        {_pstDT->s.sTime.s.nHour = 0; day_inc(&_pstDT->s.sDate);}
	}

	static void min_inc(SPtDateTime *_pstDT)
	{
		_pstDT->s.sTime.s.nMinute++;
		if(_pstDT->s.sTime.s.nMinute >= 60)
        {_pstDT->s.sTime.s.nMinute = 0; hour_inc(_pstDT);}
	}

	static void sec_inc(SPtDateTime *_pstDT)
	{
		_pstDT->s.sTime.s.nSecond++;
		if(_pstDT->s.sTime.s.nSecond >= 60)
        {_pstDT->s.sTime.s.nSecond = 0; min_inc(_pstDT);}
	}

	static void month_dec(SPtDate *_pDate)
	{
		_pDate->s.month--;
		if (_pDate->s.month < 1)
        {_pDate->s.month = 12; _pDate->s.year--;}
	}

	static void day_dec(SPtDate *_pDate)
	{
		_pDate->s.day--;
		if (_pDate->s.day < 1)
        {month_dec(_pDate); _pDate->s.day = dayinmonth[_pDate->s.month - 1];}
	}

	static void hour_dec(SPtDateTime *_pstDT)
	{
		_pstDT->s.sTime.s.nHour--;
		if (_pstDT->s.sTime.s.nHour < 0)
        {_pstDT->s.sTime.s.nHour = 23; day_dec(&_pstDT->s.sDate);}
	}

	static void min_dec(SPtDateTime *_pstDT)
	{
		_pstDT->s.sTime.s.nMinute--;
		if (_pstDT->s.sTime.s.nMinute < 0)
        {_pstDT->s.sTime.s.nMinute = 59; hour_dec(_pstDT);}
	}

	static void sec_dec(SPtDateTime *_pstDT)
	{
		_pstDT->s.sTime.s.nSecond--;
		if (_pstDT->s.sTime.s.nSecond < 0)
        {_pstDT->s.sTime.s.nSecond = 59; min_dec(_pstDT);}
	}

	static void dec(int _nType, SPtDateTime *_ptm, int _nCnt = 1);
	static void inc(int _nType, SPtDateTime *_ptm, int _nCnt = 1);

	static void pase_loop_datetime(const char *_strFormat, SPtDateTime *_ptm);
	static SPtDateTime parse_datetime_by_gmail(const char *_strGmailDateTime);
	static INT64 parse_date_time(const char *_strDate);
	static char *make_date_time(INT64, char *_strDate);
	static char* make_date_time_http(INT64, char* _strDate, int _size);
	static char* make_date_time_project(INT64, char *_strDate, int _size);
	static char* make_date(SPtDate _stDate, char *_strDate, int _nSize);
	static char* make_date(SPtDateTime _stDate, char *_strDate, int _nSize);
	static SPtDateTime datetime_set(int _nY, int _nM, int _nD, int _nH, int _nMi, int _nS);
	
	static int dayinmonth[12];
};


bool operator<(const SPtDate &stDTLeft, const SPtDate &stDTRight);
bool operator<(const SPtTime &stDTLeft, const SPtTime &stDTRight);
bool operator<(const SPtDateTime &stDTLeft, const SPtDateTime &stDTRight);

bool operator!=(const SPtDate &stDTLeft, const SPtDate &stDTRight);
bool operator!=(const SPtTime &stDTLeft, const SPtTime &stDTRight);
bool operator!=(const SPtDateTime &stDTLeft, const SPtDateTime &stDTRight);

bool operator==(const SPtDate &stDTLeft, const SPtDate &stDTRight);
bool operator==(const SPtTime &stDTLeft, const SPtTime &stDTRight);
bool operator==(const SPtDateTime &stDTLeft, const SPtDateTime &stDTRight);

bool operator >(const SPtDate &stDTLeft, const SPtDate &stDTRight);
bool operator >(const SPtTime &stDTLeft, const SPtTime &stDTRight);
bool operator >(const SPtDateTime &stDTLeft, const SPtDateTime &stDTRight);

bool operator >=(const SPtDate &stDTLeft, const SPtDate &stDTRight);
bool operator >=(const SPtTime &stDTLeft, const SPtTime &stDTRight);
bool operator >=(const SPtDateTime &stDTLeft, const SPtDateTime &stDTRight);

bool operator <=(const SPtDate &stDTLeft, const SPtDate &stDTRight);
bool operator <=(const SPtTime &stDTLeft, const SPtTime &stDTRight);
bool operator <=(const SPtDateTime &stDTLeft, const SPtDateTime &stDTRight);

int operator-(const SPtDate &stDTLeft, const SPtDate &stDTRight);
int operator-(const SPtDateTime &stDTLeft, const SPtDateTime &stDTRight);
int operator-(const SPtTime &stDTLeft, const SPtTime &stDTRight);
