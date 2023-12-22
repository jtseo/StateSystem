#include "stdafx.h"
#include "BaseTime.h"
#include "BaseFile.h"

int BaseTime::dayinmonth[12] = {
	31,
	28,
	31,
	30,
	31,
	30, // 6
	31, // 7
	31, // 8
	30,
	31,
	30,
	31
};

char dayinweek[7][3]	= {
	"Mo",
	"Tu", 
	"We", 
	"Th", 
	"Fr", 
	"Sa", 
	"Su"
};

char dayinweek3[7][4] = {
	"Sun",
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri",
	"Sat"
};

char s_month[12][4] =
{
	"Jan","Feb","Mar","Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

BaseTime::BaseTime(void)
{
}

BaseTime::~BaseTime(void)
{
}

int	get_int(char *_strInt)
{
	int	nValue;
	sscanf_s(_strInt, "%d", &nValue);

	return nValue;
}

int get_month(char *_strMonth)
{
	for(int i=0; i<12; i++)
	{
		if(strncmp(s_month[i], _strMonth, 3) == 0)
			return i+1;
	}
	return 0;
}

SPtDateTime BaseTime::parse_datetime_by_gmail(const char *_strGmailDateTime)
{
	SPtDateTime	stDT;

	//Tue, 04 Dec 2012 04:18:15 -0800
	BaseFile parser;

	parser.set_asc_seperator(" :");
	parser.set_asc_deletor(",");

	parser.OpenFile((void*)_strGmailDateTime, (UINT32)strlen(_strGmailDateTime));
	parser.read_asc_line();

	int	nBuf;
	char strBuffer[255];
	parser.read_asc_string(strBuffer, 255);
	parser.read_asc_integer(&nBuf);

	stDT.s.sDate.s.day	 = nBuf;
	parser.read_asc_string(strBuffer, 255);
	stDT.s.sDate.s.month	= get_month(strBuffer);
	parser.read_asc_integer(&nBuf);
	stDT.s.sDate.s.year	= nBuf;
	parser.read_asc_integer(&nBuf);
	stDT.s.sTime.s.nHour	= nBuf;
	parser.read_asc_integer(&nBuf);
	stDT.s.sTime.s.nMinute	= nBuf;
	parser.read_asc_integer(&nBuf);
	stDT.s.sTime.s.nSecond	= nBuf;

	return stDT;
}

INT64 BaseTime::parse_date_time(const char *_strDate)
{
	SPtDateTime	stDT;

	stDT.dateTime	= 0;

	char strTemp[255];
	const char	*strTemp2;
	strTemp2	= _strDate;
	while(*strTemp2 && (strTemp2 - _strDate) < 255)
	{
		int	nLen	= 0;
		while(*strTemp2 >= '0' && *strTemp2 <= '9')
		{
			strTemp[nLen]	= *strTemp2;
			strTemp2++;
			nLen++;
		}

		int nValue	= 0;
		if(nLen > 0)
		{
			strTemp[nLen]	= NULL;
			nValue	= get_int(strTemp);
		}
		switch(*strTemp2)
		{
		case 'y':
			stDT.s.sDate.s.year	= nValue;
			break;
		case 'm':
			stDT.s.sDate.s.month	= nValue;
			break;
		case 'd':
			stDT.s.sDate.s.day	= nValue;
			break;
		case 'h':
			if(nValue > 24)
			{
				stDT.s.sDate.s.day	= nValue/24;
				nValue	-= stDT.s.sDate.s.day * 24;
			}
			stDT.s.sTime.s.nHour	= nValue;
			break;
		case ' ':
			break;
		default:
			stDT.s.sDate.s.day	= nValue;
			return stDT.dateTime;
		}

		strTemp2++;
	}
	return stDT.dateTime;
}

char* BaseTime::make_date(SPtDate _stDate, char *_strDate, int _nSize)
{
	sprintf_s(_strDate, _nSize, "%d/%d/%d", _stDate.s.year, _stDate.s.month, _stDate.s.day);
	return _strDate;
}

char* BaseTime::make_date(SPtDateTime _stDate, char *_strDate, int _nSize)
{
	sprintf_s(_strDate, _nSize, "%d/%d/%d %d:%d:%d", _stDate.s.sDate.s.year, _stDate.s.sDate.s.month, _stDate.s.sDate.s.day, _stDate.s.sTime.s.nHour, _stDate.s.sTime.s.nMinute, _stDate.s.sTime.s.nSecond);
	return _strDate;
}

char* BaseTime::make_date_time(INT64 _nDT, char *_strDate)
{
	char strTemp[255];

	*_strDate	= NULL;

	SPtDateTime	stDT;
	stDT.dateTime	= _nDT;
	if(stDT.s.sDate.s.year > 0)
	{
		sprintf_s(strTemp, 255, "%dy", stDT.s.sDate.s.year);
		strcat_s(_strDate, 255, strTemp);
	}
	if(stDT.s.sDate.s.month > 0)
	{
		sprintf_s(strTemp, 255, "%dm", stDT.s.sDate.s.month);
		strcat_s(_strDate, 255, strTemp);
	}
	if(stDT.s.sDate.s.day > 0)
	{
		sprintf_s(strTemp, 255, "%dd", stDT.s.sDate.s.day);
		strcat_s(_strDate, 255, strTemp);
	}
	if(stDT.s.sTime.s.nHour > 0)
	{
		sprintf_s(strTemp, 255, "%dh", stDT.s.sTime.s.nHour);
		strcat_s(_strDate, 255, strTemp);
	}
	return _strDate;
}


SPtDateTime BaseTime::datetime_set(int _nY, int _nM, int _nD, int _nH, int _nMi, int _nS)
{
	SPtDateTime sptDT;

	sptDT.s.sDate.s.year = _nY;
	sptDT.s.sDate.s.month = _nM;
	sptDT.s.sDate.s.day = _nD;
	sptDT.s.sTime.s.nHour = _nH;
	sptDT.s.sTime.s.nMinute = _nMi;
	sptDT.s.sTime.s.nSecond = _nS;
	
	return sptDT;
}

char *BaseTime::make_date_time_http(INT64 _nDT, char* _strDate, int _size)
{
	char strTemp[255];

	*_strDate = NULL;

	SPtDateTime	stDT;
	stDT.dateTime = _nDT;
	//"Mon, 19 Jul 2021 05:33:34 GMT\n";
	sprintf_s(strTemp, 255, "%s, %02d %s %04d %02d:%02d:%02d GMT", dayinweek3[dayofweek(stDT.s.sDate.s.year, stDT.s.sDate.s.month, stDT.s.sDate.s.day)],
		stDT.s.sDate.s.day, s_month[stDT.s.sDate.s.month+1], stDT.s.sDate.s.year,
		stDT.s.sTime.s.nHour, stDT.s.sTime.s.nMinute, stDT.s.sTime.s.nSecond);
	strcpy_s(_strDate, _size, strTemp);
	return _strDate;
}

char* BaseTime::make_date_time_project(INT64 _nDT, char *_strDate, int _size)
{
	char strTemp[255];

	*_strDate	= NULL;

	SPtDateTime	stDT;
	stDT.dateTime	= _nDT;
	sprintf_s(strTemp, 255, "%02d-%02d-%02d (%s)", stDT.s.sDate.s.year, stDT.s.sDate.s.month, stDT.s.sDate.s.day, 
		dayinweek[dayofweek(stDT.s.sDate.s.year, stDT.s.sDate.s.month, stDT.s.sDate.s.day)]);
	strcpy_s(_strDate, _size, strTemp);

	return _strDate;
}

void BaseTime::dec(int _nType, SPtDateTime *_ptm, int _nCnt)
{
	for (int i = 0; i < _nCnt; i++){
		switch (_nType)
		{
		case SPtDateTime::YEAR:		_ptm->s.sDate.s.year--;		break;
		case SPtDateTime::MONTH:	month_dec(&_ptm->s.sDate);	break;
		case SPtDateTime::DAY:		day_dec(&_ptm->s.sDate);		break;
		case SPtDateTime::HOUR:		hour_dec(_ptm);				break;
		case SPtDateTime::MINUTE:	min_dec(_ptm);				break;
		case SPtDateTime::SECOND:	sec_dec(_ptm);				break;
		}
	}
}

void BaseTime::inc(int _nType, SPtDateTime *_ptm, int _nCnt)
{
	for (int i = 0; i < _nCnt; i++){
		switch (_nType)
		{
		case SPtDateTime::YEAR:		_ptm->s.sDate.s.year++;		break;
		case SPtDateTime::MONTH:	month_inc(&_ptm->s.sDate);	break;
		case SPtDateTime::DAY:		day_inc(&_ptm->s.sDate);		break;
		case SPtDateTime::HOUR:		hour_inc(_ptm);				break;
		case SPtDateTime::MINUTE:	min_inc(_ptm);				break;
		case SPtDateTime::SECOND:	sec_inc(_ptm);				break;
		}
	}
}

void BaseTime::pase_loop_datetime(const char *_strFormat, SPtDateTime *_ptm)
{
	BaseFile	paser;

	paser.set_asc_seperator(" -:/~");
	paser.set_asc_deletor(" \t");
	paser.OpenFile((void*)_strFormat, (UINT32)strlen(_strFormat));

	memset(_ptm, 0, sizeof(SPtDateTime));
	int nValue;
	SPtDateTime	tmCur;
	BaseSystem::timeCurrent(&tmCur);
	char strBuf[255];
	int _nIncType = SPtDateTime::NONE;

	if(paser.read_asc_line())
	{
		if(paser.read_asc_string(strBuf, 255))// year
		{
            if(strcmp(strBuf, "?") == 0){
                _ptm->s.sDate.s.year = tmCur.s.sDate.s.year; _nIncType = SPtDateTime::YEAR;
            }else{
				sscanf_s(strBuf, "%d", &nValue);
				_ptm->s.sDate.s.year = nValue;
			}
			if(paser.read_asc_string(strBuf, 255))
			{
                if(strcmp(strBuf, "?") == 0){
                    _ptm->s.sDate.s.month = tmCur.s.sDate.s.month; _nIncType = SPtDateTime::MONTH;
                }else{
					sscanf_s(strBuf, "%d", &nValue);
					_ptm->s.sDate.s.month = nValue;
				}
				if(paser.read_asc_string(strBuf, 255))// day
				{
                    if(strcmp(strBuf, "?") == 0){
                        _ptm->s.sDate.s.day = tmCur.s.sDate.s.day; _nIncType = SPtDateTime::DAY;
                    }else{
						sscanf_s(strBuf, "%d", &nValue);
						_ptm->s.sDate.s.day = nValue;
					}
					if(paser.read_asc_string(strBuf, 255))// hour
					{
                        if(strcmp(strBuf, "?") == 0){
                            _ptm->s.sTime.s.nHour = tmCur.s.sTime.s.nHour; _nIncType = SPtDateTime::HOUR;
                        }else{
							sscanf_s(strBuf, "%d", &nValue);
							_ptm->s.sTime.s.nHour = nValue;
						}
						if(paser.read_asc_string(strBuf, 255))
						{
                            if(strcmp(strBuf, "?") == 0){
                                _ptm->s.sTime.s.nMinute = tmCur.s.sTime.s.nMinute; _nIncType = SPtDateTime::MINUTE;
                            }else{
								sscanf_s(strBuf, "%d", &nValue);
								_ptm->s.sTime.s.nMinute = nValue;
							}
							if(paser.read_asc_string(strBuf, 255))
							{
								sscanf_s(strBuf, "%d", &nValue);
								_ptm->s.sTime.s.nSecond = nValue;
							}
						}
					}// end of hour
				}
			}
		}// end of year
	}

	if(_nIncType != SPtDateTime::NONE && tmCur > *_ptm)
		inc(_nIncType, _ptm);
}

bool operator<(const SPtDate &stDTLeft, const SPtDate &stDTRight)
{
	if(stDTLeft.s.year < stDTRight.s.year)
		return true;

	if(stDTLeft.s.year != stDTRight.s.year)
		return false;

	if(stDTLeft.s.month < stDTRight.s.month)
		return true;

	if(stDTLeft.s.month != stDTRight.s.month)
		return false;

	if(stDTLeft.s.day < stDTRight.s.day)
		return true;

	return false;
}

bool operator<(const SPtTime &stDTLeft, const SPtTime &stDTRight)
{
	if(stDTLeft.s.nHour < stDTRight.s.nHour)
		return true;

	if(stDTLeft.s.nHour != stDTRight.s.nHour)
		return false;

	if(stDTLeft.s.nMinute < stDTRight.s.nMinute)
		return true;

	if(stDTLeft.s.nMinute != stDTRight.s.nMinute)
		return false;

	if(stDTLeft.s.nSecond < stDTRight.s.nSecond)
		return true;

	return false;
}

bool operator<(const SPtDateTime &stDTLeft, const SPtDateTime &stDTRight)
{
	if(stDTLeft.s.sDate < stDTRight.s.sDate)
		return true;

	if(stDTLeft.s.sDate.date != stDTRight.s.sDate.date)
		return false;

	if(stDTLeft.s.sTime < stDTRight.s.sTime)
		return true;
	
	return false;
}

bool operator!=(const SPtDate &stDTLeft, const SPtDate &stDTRight)
{
	return (stDTLeft.date != stDTRight.date);
}

bool operator!=(const SPtTime &stDTLeft, const SPtTime &stDTRight)
{
	return (stDTLeft.nTime != stDTRight.nTime);
}

bool operator!=(const SPtDateTime &stDTLeft, const SPtDateTime &stDTRight)
{
	return (stDTLeft.dateTime != stDTRight.dateTime);
}

bool operator==(const SPtDate &stDTLeft, const SPtDate &stDTRight)
{
	return !(stDTLeft != stDTRight);
}
bool operator==(const SPtTime &stDTLeft, const SPtTime &stDTRight)
{
	return !(stDTLeft != stDTRight);
}
bool operator==(const SPtDateTime &stDTLeft, const SPtDateTime &stDTRight)
{
	return !(stDTLeft != stDTRight);
}

bool operator >(const SPtDate &stDTLeft, const SPtDate &stDTRight)
{
	if(stDTLeft < stDTRight || stDTLeft == stDTRight)
		return false;
	return true;
}
bool operator >(const SPtTime &stDTLeft, const SPtTime &stDTRight)
{
	if(stDTLeft < stDTRight || stDTLeft == stDTRight)
		return false;
	return true;
}

bool operator >(const SPtDateTime &stDTLeft, const SPtDateTime &stDTRight)
{
	if(stDTLeft < stDTRight || stDTLeft == stDTRight)
		return false;
	return true;
}

bool operator >=(const SPtDate &stDTLeft, const SPtDate &stDTRight)
{
	if(!(stDTLeft < stDTRight))
		return true;
	return false;
}
bool operator >=(const SPtTime &stDTLeft, const SPtTime &stDTRight)
{
	if(!(stDTLeft < stDTRight))
		return true;
	return false;
}
bool operator >=(const SPtDateTime &stDTLeft, const SPtDateTime &stDTRight)
{
	if(!(stDTLeft < stDTRight))
		return true;
	return false;
}

bool operator <=(const SPtDate &stDTLeft, const SPtDate &stDTRight)
{
	if(!(stDTLeft > stDTRight))
		return true;
	return false;
}
bool operator <=(const SPtTime &stDTLeft, const SPtTime &stDTRight)
{
	if(!(stDTLeft > stDTRight))
		return true;
	return false;
}
bool operator <=(const SPtDateTime &stDTLeft, const SPtDateTime &stDTRight)
{
	if(!(stDTLeft > stDTRight))
		return true;
	return false;
}

int operator-(const SPtDateTime &stDTLeft, const SPtDateTime &stDTRight)
{
	int nRet = stDTLeft.s.sDate - stDTRight.s.sDate;

	nRet = nRet * 24 * 3600;
	nRet += stDTLeft.s.sTime - stDTRight.s.sTime;

	return nRet;
}

__sp_datetime__::__sp_datetime__(const struct tm& stRight)
{
//void SPtDateTime::operator=(const struct tm &stRight)
//{
	s.sDate.s.year	= stRight.tm_year + 1900;
	s.sDate.s.month	= stRight.tm_mon+1;
	s.sDate.s.day	= stRight.tm_mday;
	
	s.sTime.s.nHour	= stRight.tm_hour;
	s.sTime.s.nMinute	= stRight.tm_min;
	s.sTime.s.nSecond	= stRight.tm_sec;
}

int operator-(const SPtTime &stDTLeft, const SPtTime &stDTRight)
{
	int nRet = 0;
	nRet = (stDTLeft.s.nHour - stDTRight.s.nHour) * 3600
		+ (stDTLeft.s.nMinute - stDTRight.s.nMinute) * 60
		+ (stDTLeft.s.nSecond - stDTRight.s.nSecond);

	return nRet;
}

int operator-(const SPtDate &stDTLeft, const SPtDate &stDTRight)
{
	int	nRet;

	nRet	= (stDTLeft.s.day - stDTRight.s.day) + 
		(stDTLeft.s.month - stDTRight.s.month) * 31 + 
		(stDTLeft.s.year - stDTRight.s.year) * 365;

	if(nRet > 1000)
		return nRet;

	if(nRet < 0)
		return nRet;

	SPtDate date;
	date	= stDTRight;

	nRet	= 0;
    while(date < stDTLeft){
        date.date	= BaseTime::day_increase(date.date); nRet++;
    }

	return nRet;
}



void getsubstr(char *instr,int pos,int size,char *outstr)
{
	char buff[1024];
	char *pch;
	int i;

	pch = instr;
	for(i=0;i<pos;i++){ pch++;}
	for(i=0;i<size;i++){
		buff[i] = *pch;
		pch++;
	}
	buff[i]='\0';
	strcpy_s(outstr, 20, buff);
}

char lunar[][14] = 
{"1212122322121","1212121221220","1121121222120","2112132122122","2112112121220",

"2121211212120","2212321121212","2122121121210","2122121212120","1232122121212",

"1212121221220","1121123221222","1121121212220","1212112121220","2121231212121",

"2221211212120","1221212121210","2123221212121","2121212212120","1211212232212",

"1211212122210","2121121212220","1212132112212","2212112112210","2212211212120",

"1221412121212","1212122121210","2112212122120","1231212122212","1211212122210",

"2121123122122","2121121122120","2212112112120","2212231212112","2122121212120",

"1212122121210","2132122122121","2112121222120","1211212322122","1211211221220",

"2121121121220","2122132112122","1221212121120","2121221212110","2122321221212",

"1121212212210","2112121221220","1231211221222","1211211212220","1221123121221",

"2221121121210","2221212112120","1221241212112","1212212212120","1121212212210",

"2114121212221","2112112122210","2211211412212","2211211212120","2212121121210",

"2212214112121","2122122121120","1212122122120","1121412122122","1121121222120",

"2112112122120","2231211212122","2121211212120","2212121321212","2122121121210",

"2122121212120","1212142121212","1211221221220","1121121221220","2114112121222",

"1212112121220","2121211232122","1221211212120","1221212121210","2121223212121",

"2121212212120","1211212212210","2121321212221","2121121212220","1212112112210",

"2223211211221","2212211212120","1221212321212","1212122121210","2112212122120",

"1211232122212","1211212122210","2121121122210","2212312112212","2212112112120",

"2212121232112","2122121212110","2212122121210","2112124122121","2112121221220",

"1211211221220","2121321122122","2121121121220","2122112112322","1221212112120",

"1221221212110","2122123221212","1121212212210","2112121221220","1211231212222",

"1211211212220","1221121121220","1223212112121","2221212112120","1221221232112",

"1212212122120","1121212212210","2112132212221","2112112122210","2211211212210",

"2221321121212","2212121121210","2212212112120","1232212122112","1212122122120",

"1121212322122","1121121222120","2112112122120","2211231212122","2121211212120",

"2122121121210","2124212112121","2122121212120","1212121223212","1211212221220",

"1121121221220","2112132121222","1212112121220","2121211212120","2122321121212",

"1221212121210","2121221212120","1232121221212","1211212212210","2121123212221",

"2121121212220","1212112112220","1221231211221","2212211211220","1212212121210",

"2123212212121","2112122122120","1211212322212","1211212122210","2121121122120",

"2212114112122","2212112112120","2212121211210","2212232121211","2122122121210",
"2112122122120","1231212122212","1211211221220" };
int lday[] = {31,0,31,30,31,30,31,31,30,31,30,31};

void get_negative(int gf_year,int gf_month,int gf_day,int *year,int *month,int *day,int *youn)
{
	// 처리가능 기간 1881 - 2043퀋E

	int i,j;
	char buff[20];
	int dt[500];
	int tmp,td,td0,td1,td2,k11;
	int m1,m2,jcount;

	// 양력 -> 음력
	for(i = 0;i <= 162;i++) {
		dt[i] = 0;
		for(j = 0;j < 12;j++) {
			getsubstr(lunar[i],j,1,&buff[0]);
			tmp = atoi(buff);
			if(tmp == 1 || tmp == 3) {
				dt[i] += 29;
			}
			else if(tmp == 2 || tmp == 4) {
				dt[i] += 30;
			}
			else;
		}

		getsubstr(lunar[i],12,1,&buff[0]);
		tmp = atoi(buff);
		if(tmp == 1 || tmp == 3) {
			dt[i] += 29;
		}
		else if(tmp == 2 || tmp == 4) {
			dt[i] += 30;
		}
		else;
	}

	td1 = 1880 * 365 + (int)(1880 / 4) - (int)(1880 / 100) + (int)(1880 / 400) + 30;
	k11 = gf_year - 1;
	td2 = k11 * 365 + (int)(k11 / 4) - (int)(k11 / 100) + (int)(k11 / 400);

	if((gf_year % 400) == 0 || (
                                ((gf_year % 100) != 0) && ((gf_year % 4)== 0)
                                )
       )
	{
		lday[1] = 29;
	}
	else {
		lday[1] = 28;
	}

	if(gf_day > lday[gf_month - 1]) {
		return;
	}

	for(i = 0;i <= gf_month - 2;i++) {
		td2 += lday[i];
	}
	td2 += gf_day;
	td = td2 - td1 + 1;
	td0 = dt[0];

	for(i = 0;i <= 162;i++) {
		if(td <= td0) {
			break;
		}
		td0 += dt[i + 1];
	}

	gf_year = i + 1881;
	td0 -= dt[i];
	td -= td0;

	getsubstr(lunar[i],12,1,&buff[0]);
	if(atoi(buff) == 0) {
		jcount = 11;
	}
	else {
		jcount = 12;
	}
	m2 = 0;

	for(j = 0;j <= jcount;j++) {
		getsubstr(lunar[i],j,1,&buff[0]);
		if(atoi(buff) <= 2) {
			m2++;
			m1 = atoi(buff) + 28;
			*youn = 0;
		}
		else {
			m1 = atoi(buff) + 26;
			*youn = 1;
		}
		if(td <= m1) {
			break;
		}
		td -= m1;
	}
	gf_month = m2;
	gf_day = td;

	*year = gf_year;
	*month = gf_month;
	*day = gf_day;
}

STLMnInt	stlMnnLunarCache;

SPtDate BaseTime::get_lunar(const SPtDate &_dtSolar)
{
	int nY, nM, nD, nYoun;
	int nRY, nRM, nRD;
	SPtDate dtRet;
	STLMnInt::iterator	it;
	it	= stlMnnLunarCache.find(_dtSolar.date);
	if(it != stlMnnLunarCache.end())
	{
		dtRet.date	= it->second;
		return dtRet;
	}

	nY	= _dtSolar.s.year;
	nM	= _dtSolar.s.month;
	nD	= _dtSolar.s.day;
	//1881 - 2043
	if(nY >= 1881 && nY <= 2043){
		get_negative(nY, nM, nD, &nRY, &nRM, &nRD, &nYoun);
		dtRet.s.year	= (short)nRY;
		dtRet.s.month	= (char)nRM;
		dtRet.s.day		= (char)nRD;

		stlMnnLunarCache[_dtSolar.date]	= dtRet.date;
	}else
		dtRet.date	= 0;
	
	return dtRet;
}
