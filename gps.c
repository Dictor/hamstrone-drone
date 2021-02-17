#include "include/gps.h"

void GPS_type(char * dataReceive, struct Ele_Num gpsType, char * type)
{
	int commaCnt=0, i, j;
	for(i=0;i<strlen(dataReceive);i++)
	{
		if(dataReceive[i]==',')
			commaCnt++;	
	}
	if(strcmp(type,"GGA")==0 && commaCnt==14)
	{
		for(i=0;i<gpsType.num-1;i++)
		{
			if(i==5||i==6||i==7||i==9)
			{
				for(j=gpsType.Element[i];j<gpsType.Element[i+1]-1;j++)
					HAMSTERTONGUE_Debugf("%c", dataReceive[j]);
			}
			HAMSTERTONGUE_Debugf("\n");
		}
	}
	else if(strcmp(type,"GLL")==0 && commaCnt==7)
	{
		for(i=0;i<gpsType.num-1;i++)
		{
			if(i==1||i==2||i==3||i==4)
			{
				for(j=gpsType.Element[i];j<gpsType.Element[i+1]-1;j++)
					HAMSTERTONGUE_Debugf("%c", dataReceive[j]);
			}
			HAMSTERTONGUE_Debugf("\n");
		}
	}
	else
		HAMSTERTONGUE_Debugf("Data was damaged\n");
}

void Split(char *dataReceive)
{
    int dataCnt = 0, eleCnt = 1, i=0, len=0;
    char gpsType[4];
    struct Ele_Num Ele;
    Ele.Element[0]=0;
    while (1)
    {
        if (dataReceive[dataCnt] == ',')
        {
            Ele.Element[eleCnt] = dataCnt + 1;
            eleCnt++;
            if(dataReceive[dataCnt+1]==',')
            	dataCnt++;
        }
		if(dataCnt==2 || dataCnt==3 || dataCnt==4)
		{
			gpsType[i]=dataReceive[dataCnt];
			i++;
		}
        len++;
        if(len==strlen(dataReceive))
        	break;
        dataCnt++;
    }
	Ele.num = eleCnt;
	GPS_type(dataReceive,Ele,gpsType);
}

void Checking(char * Assemble_Data)
{
	while(1)
	{
		int dataLen=strlen(Assemble_Data);
		if(dataLen<60)
			break;
		else
		{
			int i, k=0, dataStart, dataEnd, dataSplit=0, condition=0;
			char Splited_Data[100]={0,};
			for(i=0;i<dataLen;i++)
			{
        		if(Assemble_Data[i]=='$'){
        			if(dataSplit==1){
        				dataEnd=i;
        				condition=1;
        				break;
					}
					else{
						dataSplit=1;
						dataStart=i+1;
					}
				}
			}
			HAMSTERTONGUE_Debugf("dataLen=%d, condition=%d", dataLen, condition);
			if(condition==1)
			{
				for(i=0; i<dataEnd-dataStart;i++)
					Splited_Data[i]=Assemble_Data[dataStart+i];
				for(i=0;i<dataLen;i++){
					if(i<dataLen-dataEnd){
						Assemble_Data[i]=Assemble_Data[dataEnd+k];
						k++;
					}
					else
						Assemble_Data[i]='\0';
				}
				HAMSTERTONGUE_Debugf("second %s\n", Assemble_Data);
				HAMSTERTONGUE_Debugf("third %s\n", Splited_Data);
        		Split(Splited_Data);
			}
			dataSplit=0;
			condition=0;
			k=0;	
		}
	}
}