#include "include/gps.h"

void GPS_type_GGA(char * Data_receive, struct Ele_Num GGAN)
{
	int commaCnt=0, i, j;
	for(i=0;i<strlen(Data_receive);i++)
	{
		if(Data_receive[i]==',')
			commaCnt++;	
	}
	if(commaCnt!=14)
	{
		HAMSTERTONGUE_Debugf("Data was damagegd\n");
	}
	else
	{
		for(i=0;i<GGAN.num-1;i++)
		{
			if(i==5||i==6||i==7||i==9)
			{
				for(j=GGAN.Element[i];j<GGAN.Element[i+1]-1;j++)
					HAMSTERTONGUE_Debugf("%c", Data_receive[j]);
			}
			HAMSTERTONGUE_Debugf("\n");
		}
	}
}

void GPS_type_GLL(char * Data_receive,struct Ele_Num GLLN)
{
	int commaCnt=0, i, j;
	for(i=0;i<strlen(Data_receive);i++)
	{
		if(Data_receive[i]==',')
			commaCnt++;	
	}
	if(commaCnt!=7)
	{
		HAMSTERTONGUE_Debugf("Data was damagegd\n");
	}
	else
	{
		for(i=0;i<GLLN.num-1;i++)
		{
			if(i==1||i==2||i==3||i==4)
			{
				for(j=GLLN.Element[i];j<GLLN.Element[i+1]-1;j++)
					HAMSTERTONGUE_Debugf("%c", Data_receive[j]);
			}
			HAMSTERTONGUE_Debugf("\n");
		}
	}
}

void Split(char *Data_receive)
{
    int dataCnt = 0, eleCnt = 1, i=0, start, len=0;
    char GPS_Type[4];
    struct Ele_Num Ele;
    Ele.Element[0]=0;
    while (1)
    {
        if (Data_receive[dataCnt] == ',')
        {
            Ele.Element[eleCnt] = dataCnt + 1;
            eleCnt++;
            if(Data_receive[dataCnt+1]==',')
            	dataCnt++;
        }
		if(dataCnt==2 || dataCnt==3 || dataCnt==4)
		{
			GPS_Type[i]=Data_receive[dataCnt];
			i++;
		}
        len++;
        if(len==strlen(Data_receive))
        	break;
        dataCnt++;
    }
	Ele.num = eleCnt;
	HAMSTERTONGUE_Debugf("parser calling, elecnt=%d", Ele.num);
	if(strcmp(GPS_Type,"GGA")==0)
		GPS_type_GGA(Data_receive,Ele);		
	else if(strcmp(GPS_Type,"GLL")==0)
		GPS_type_GLL(Data_receive,Ele);
}