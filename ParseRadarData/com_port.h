#ifndef TOPSKY_COM_PORT_H
#define TOPSKY_COM_PORT_H


typedef unsigned char uchar;

#ifndef __cplusplus
#define bool   int
#define false  0
#define true   1
#endif // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif // __cpluscplus

	bool OpenComport(const int port, const int baud_rate);
	void CloseComport();
	int  ReadComport(void *data, const int size);
	int  WriteComport(const void *data, const int size);
	void AnalyseComData(const unsigned char *com_data, const int com_size);

#ifdef __cplusplus
};
#endif // __cpluscplus


#endif // TOPSKY_COM_PORT_H