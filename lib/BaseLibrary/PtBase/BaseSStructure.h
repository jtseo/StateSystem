
#ifndef BASE_SSTRUCTURE_HEADER
#define BASE_SSTRUCTURE_HEADER

typedef union{
	struct{
		char	nHour;
		char	nMinute;
		char	nSecond;
	}s;
	__int32	nTime;
} SPtTime;

typedef union {
	struct {
		short	year;
		char	month;
		char	day;
	}s;
	__int32	date;
} SPtDate;

typedef union{
	struct{
		SPtDate	sDate;
		SPtTime	sTime;
	}s;
	__int64	dateTime;
} SPtDateTime;

#endif // end of BASE_STRUCTURE_HEADER