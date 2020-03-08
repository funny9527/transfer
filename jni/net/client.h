#include <jni.h>
#ifndef __FINDER_H__
#define __FINDER_H__

#include "stdafx.h"


class Client
{
private:
	int started;
	int find;
public:
	Client();
	~Client();
	int sendBroadCast(char* ip);
	void receiveBroadCastBack(JNIEnv* env, jobject thiz, jobject listener, jmethodID methodid);
	int sendtoSer(char* ip, char* data);
//	int sendPacket(char* ip, char* data);
	int sendFiles();
};

#endif
