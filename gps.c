#include "include/gps.h"

int GPS_type(char * dataReceive, struct Ele_Num gpsType, int dataLen, int assembleCnt)
{
	int commaCnt=0, i, j;
	double convert;
	for(i=0;i<dataLen;i++){
		if(dataReceive[i]==',')
			commaCnt++;	
	}
	HAMSTERTONGUE_Debugf("%s\n", dataReceive);
	if(dataReceive[2]=='R' && dataReceive[3]=='M' && dataReceive[4]=='C' && commaCnt==12){
		assembleCnt++;
		for(i=0;i<gpsType.num-1;i++){
			int k=0;
			char assembleData[15]={0,};
			if(i==1 || i==3 || i==5){
				for(j=gpsType.Element[i];j<gpsType.Element[i+1]-1;j++){
					assembleData[k]=dataReceive[j];
					k++;
				}
				convert=atof(assembleData);
				if(i==1){// UTC
					convert*=(int)100;
					HAMSTRONE_WriteValueStore(11, (uint32_t)convert);
				}
				else if(i==3){// Latitude
					convert*=(int)100000;
					HAMSTRONE_WriteValueStore(12, (uint32_t)convert);
				}
				else if(i==5){// Longitude
					convert*=(int)100000;
					HAMSTRONE_WriteValueStore(13, (uint32_t)convert);
				}
			}
		}
	}
	else if(dataReceive[2]=='G' && dataReceive[3]=='G' && dataReceive[4]=='A' && commaCnt==14){
		assembleCnt++;
		for(i=0;i<gpsType.num-1;i++){
			int k=0;
			char assembleData[15]={0,};
			if(i==7 || i==8){
				for(j=gpsType.Element[i];j<gpsType.Element[i+1]-1;j++){
					assembleData[k]=dataReceive[j];
					k++;
				}
				convert=atof(assembleData);
				if(i==7){// Number of Satellites used for Calculation
					convert=(int)convert;
					HAMSTRONE_WriteValueStore(14, (uint32_t)convert);
				}
				else if(i==8){// HDOP
					convert*=(int)100;
					HAMSTRONE_WriteValueStore(15, (uint32_t)convert);
				}
			}
		}
	}
	return assembleCnt;
}

int Split(char *Receive, int insertLen, int assembleCnt)
{
    int splitCnt = 0, eleCnt = 1, splitlen=0;
    struct Ele_Num Ele;
    Ele.Element[0]=0;
    for(splitlen=0;splitlen<insertLen;splitlen++){
        if (Receive[splitCnt] == ','){
            Ele.Element[eleCnt] = splitCnt + 1;
            eleCnt++;
        }
        splitCnt++;
    }
	Ele.num = eleCnt;
	assembleCnt=GPS_type(Receive,Ele,insertLen,assembleCnt);
	return assembleCnt;
}

int Insert_Zero(char *dataReceive, int splitLen, int assembleCnt)
{
	char insert[200]={0,};
	int i=0, len=0, dataCnt=0, insertLen=0;
	for(len=0;len<splitLen;len++){
		if ((*(dataReceive+dataCnt) == ',') && ((*(dataReceive+dataCnt+1)==',')||(*(dataReceive+dataCnt+1)=='*'))){
			insert[i]=*(dataReceive+dataCnt);
			i++;
			insert[i]='0';
        }
		else
			insert[i]=*(dataReceive+dataCnt);
		i++;
		dataCnt++;
	}
	insertLen=strlen(insert);
	assembleCnt=Split(insert, insertLen, assembleCnt);
	return assembleCnt;
}

int Checking(char * Assemble_Data, int assembleLen, int assembleCnt)
{
	while(1){
		if(assembleLen<150)
			break;
		else{
			int i, condition=0, dataStart, dataEnd, dataSplit=0, splitLen=0;
			char Splited_Data[200]={0,};
			for(i=0;i<assembleLen;i++){
        		if(*(Assemble_Data+i)=='$'){
        			if(dataSplit==1){
        				dataEnd=i;
        				break;
					}
					else{
						dataSplit=1;
						dataStart=i+1;
					}
				}
			}
			for(i=0; i<dataEnd-dataStart;i++)
				Splited_Data[i]=*(Assemble_Data+dataStart+i);
			for(i=0;i<200;i++){
				if(i<assembleLen-dataEnd){
					*(Assemble_Data+i)=*(Assemble_Data+dataEnd+condition);
					condition++;
				}
				else
					*(Assemble_Data+i)='\0';
			}
			assembleLen=strlen(Assemble_Data);
			splitLen=strlen(Splited_Data);
        	assembleCnt=Insert_Zero(Splited_Data, splitLen, assembleCnt);
			dataSplit=0;
			condition=0;
		}
	}
	return assembleCnt;
}