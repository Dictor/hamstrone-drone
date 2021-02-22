#include "include/gps.h"

int GPS_type(char * dataReceive, struct Ele_Num gpsType, char * type, int assembleCnt)
{
	int commaCnt=0, i, j;
	double convert;
	for(i=0;i<strlen(dataReceive);i++){
		if(dataReceive[i]==',')
			commaCnt++;	
	}
	HAMSTERTONGUE_Debugf("%s\n", dataReceive);
	if(strcmp(type,"GGA")==0 && commaCnt==14){
		assembleCnt++;
		for(i=0;i<gpsType.num-1;i++){
			int k=0;
			char assembleData[15]={0,};
			if(i==1){//UTC Positional Time
				for(j=gpsType.Element[i];j<gpsType.Element[i+1]-1;j++){
					assembleData[k]=dataReceive[j];
					k++;
				}
				convert=atof(assembleData);
				convert*=1000;
				convert=(int)convert;
				HAMSTRONE_WriteValueStore(11, (uint32_t)convert);
			}
			if(i==2){//Latitude
				for(j=gpsType.Element[i];j<gpsType.Element[i+1]-1;j++){
					assembleData[k]=dataReceive[j];
					k++;
				}
				convert=atof(assembleData);
				convert*=10000;
				convert=(int)convert;
				HAMSTRONE_WriteValueStore(12, (uint32_t)convert);
			}
			else if(i==4){//Longitude
				for(j=gpsType.Element[i];j<gpsType.Element[i+1]-1;j++){
					assembleData[k]=dataReceive[j];
					k++;
				}
				convert=atof(assembleData);
				convert*=10000;
				convert=(int)convert;
				HAMSTRONE_WriteValueStore(13, (uint32_t)convert);
			}
			else if(i==7){//Number of satellites used for calculation
				for(j=gpsType.Element[i];j<gpsType.Element[i+1]-1;j++){
					assembleData[k]=dataReceive[j];
					k++;
				}
				convert=atof(assembleData);
				convert=(int)convert;
				HAMSTRONE_WriteValueStore(14, (uint32_t)convert);
			}
			else if(i==8){//HDOP
				for(j=gpsType.Element[i];j<gpsType.Element[i+1]-1;j++){
					assembleData[k]=dataReceive[j];
					k++;
				}
				convert=atof(assembleData);
				convert*=10;
				convert=(int)convert;
				HAMSTRONE_WriteValueStore(15, (uint32_t)convert);
			}
		}
	}
	return assembleCnt;
}

int Split(char *Receive, int assembleCnt)
{
    int dataCnt = 0, eleCnt = 1, i=0, len=0;
    char gpsType[4];
    struct Ele_Num Ele;
    Ele.Element[0]=0;
    while (1){
        if (Receive[dataCnt] == ','){
            Ele.Element[eleCnt] = dataCnt + 1;
            eleCnt++;
        }
		if(dataCnt==2 || dataCnt==3 || dataCnt==4){
			gpsType[i]=Receive[dataCnt];
			i++;
		}
        len++;
        if(len==strlen(Receive))
        	break;
        dataCnt++;
    }
	Ele.num = eleCnt;
	assembleCnt=GPS_type(Receive,Ele,gpsType,assembleCnt);
	return assembleCnt;
}

int Insert_Zero(char *dataReceive, int assembleCnt)
{
	char insert[100]={0,};
	int i=0, len=0, dataCnt=0;
	while(1)
	{
		if (((dataReceive[dataCnt] == ',') && (dataReceive[dataCnt+1]==','))||((dataReceive[dataCnt] == ',') && (dataReceive[dataCnt+1]=='*'))){
			insert[i]=dataReceive[dataCnt];
			i++;
			insert[i]='0';
        }
		else
			insert[i]=dataReceive[dataCnt];
		len++;
		if(len==strlen(dataReceive))
        	break;
		i++;
		dataCnt++;
	}
	assembleCnt=Split(insert, assembleCnt);
	return assembleCnt;
}

int Checking(char * Assemble_Data, int assembleCnt)
{
	while(1){
		int dataLen=strlen(Assemble_Data);
		if(dataLen<67)
			break;
		else{
			int i, k=0, dataStart, dataEnd, dataSplit=0, condition=0;
			char Splited_Data[100]={0,};
			for(i=0;i<dataLen;i++){
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
			if(condition==1){
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
        		assembleCnt=Insert_Zero(Splited_Data, assembleCnt);
			}
			dataSplit=0;
			condition=0;
			k=0;
		}
	}
	return assembleCnt;
}