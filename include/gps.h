struct Ele_Num {
    int Element[50];
    int num;
};

void GPS_type_GGA(char * Data_receive, struct Ele_Num GGAN);
void GPS_type_GLL(char * Data_receive,struct Ele_Num GLLN);
void Split(char *Data_receive);