#ifndef __SERVER_H__
#define __SERVER_H__
#include "stdafx.h"

class Server
{
private:
	int started;
public:
	Server();
	~Server();
	int receiveBroadCast(JNIEnv* env, jobject thiz, char* ip, jobject listener, jmethodID methodid);
	void receiveClient(JNIEnv* env, jobject thiz, jobject listener, jmethodID methodid);
	int sendtoClt(char* ip, char* data);
	int receiveFiles(char* ip, char* file_name, char* new_name);
};
#endif
