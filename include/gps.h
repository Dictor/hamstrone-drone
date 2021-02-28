#include <string.h>
#include "hamster_tongue.h"
//#include "hamstrone_main.h"
struct Ele_Num {
    int Element[50];
    int num;
};

void GPS_type_GGA(char * Data_receive, struct Ele_Num GGAN);
void GPS_type_GLL(char * Data_receive,struct Ele_Num GLLN);
int Split(char *Data_receive, int insertLen,int assembleCnt);
int Checking(char * Assemble_Data, int assembleLen, int assembleCnt);