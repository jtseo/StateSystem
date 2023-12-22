
#ifndef BASE_STRUCTURE_HEADER
#define BASE_STRUCTURE_HEADER

typedef union{
	struct{
		char	nHour;
		char	nMinute;
		char	nSecond;
	}s;
	INT32	nTime;
} SPtTime;

typedef union {
	struct {
		short	year;
		char	month;
		char	day;
	}s;
	INT32	date;
} SPtDate;

typedef union __sp_datetime__{
	struct{
		SPtDate	sDate;
		SPtTime	sTime;
	}s;
	INT64	dateTime;

	__sp_datetime__(const struct tm& stRigth);
	__sp_datetime__() {};

	//void operator=(const struct tm &stRight);
	enum {
		NONE,
		YEAR,
		MONTH,
		DAY,
		HOUR,
		MINUTE,
		SECOND
	};
} SPtDateTime;


#endif // end of BASE_STRUCTURE_HEADER