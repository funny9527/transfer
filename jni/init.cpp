/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <string>
#include <jni.h>
#include "log.h"
#include <stdlib.h>
#include <unistd.h>
#include "net/client.h"
#include "net/server.h"

#ifdef __cplusplus
extern "C" {
#endif

static const char* clientName = "com/ijidou/transfer/Client";
static const char* serverName = "com/ijidou/transfer/Server";
Client* getClientObj(JNIEnv* env, jobject obj);
Server* getServerObj(JNIEnv* env, jobject obj);
void* start_client(void* i);


jstring getAddress(JNIEnv* env, jobject thiz)
{
	jclass cls = env->FindClass(serverName);
	jmethodID getip = env->GetStaticMethodID(cls, "getLocalHostIp","()Ljava/lang/String;");
	jstring ip = (jstring) env->CallStaticObjectMethod(cls, getip);
	return ip;
}

//客户端发送广播
void sendHeartBeat(JNIEnv* env, jobject thiz)
{
	Client* client = getClientObj(env, thiz);
	if (client == NULL)
	{
		LOGD("get client error");
		return ;
	}

	jstring ip = getAddress(env, thiz);
	char* ipstr = (char*)env->GetStringUTFChars(ip, NULL);
	if (ipstr == NULL)
	{
		return ;
	}

	client->sendBroadCast(ipstr);
	env->ReleaseStringUTFChars(ip, ipstr);
}

//客户端接收服务端收到广播后确认信息
void listenHeartBeatBack(JNIEnv* env, jobject thiz, jobject listener)
{
	Client* client = getClientObj(env, thiz);
	if (client == NULL)
	{
		LOGD("get client error");
		return ;
	}

	jclass clazz = env->GetObjectClass(listener);
	jmethodID methodid = env->GetMethodID(clazz, "onFind", "(Ljava/lang/String;)V");
	client->receiveBroadCastBack(env, thiz, listener, methodid);
}

//客户端发送命令
void sendtoServer( JNIEnv* env, jobject thiz, jstring ip, jstring data)
{
	Client* client = getClientObj(env, thiz);
	if (client == NULL)
	{
		LOGD("get client error");
		return ;
	}

	LOGD("sendtoServer  and local ip = %s", (char*)env->GetStringUTFChars(ip, NULL));
	client->sendtoSer((char*)env->GetStringUTFChars(ip, NULL), (char*)env->GetStringUTFChars(data, NULL));
}


void init_client(JNIEnv* env, jobject obj)
{
	Client* client = new Client();
	if (client == NULL)
	{
		LOGD("init client error!");
		return;
	}

	jclass cls = env->FindClass(clientName);
	jfieldID field = env->GetFieldID(cls, "mObject", "I");
    env->SetIntField(obj, field, (jint) client);
}

Client* getClientObj(JNIEnv* env, jobject obj)
{
	jclass cls = env->FindClass(clientName);
	jfieldID field = env->GetFieldID(cls, "mObject", "I");
	Client* client = (Client*) env->GetIntField(obj, field);
	return client;
}

void destory_client(JNIEnv* env, jobject obj)
{
	Client* client = getClientObj(env, obj);
	if (client == NULL)
	{
		LOGD("init client error!");
		return;
	}

	jclass cls = env->FindClass(clientName);
	jfieldID field = env->GetFieldID(cls, "mObject", "I");

	delete client;
	env->SetIntField(obj, field, (jint) 0);
}

void send_files(JNIEnv* env, jobject obj)
{
	Client* client = getClientObj(env, obj);
	if (client == NULL)
	{
		LOGD("init client error!");
		return;
	}
	client->sendFiles();
}

//客户端
static JNINativeMethod gMethods_client[] =
{
	{ "native_sendFiles", "()V", (void *) send_files},
	{ "native_init", "()V", (void *) init_client},
	{ "native_destroy", "()V", (void *) destory_client},
    { "native_sendHeartBeat", "()V", (void *) sendHeartBeat},
    { "native_listenHeartBeatBack", "(Lcom/ijidou/transfer/Client$OnFindServerListener;)V", (void *) listenHeartBeatBack},
    { "native_sendtoServer", "(Ljava/lang/String;Ljava/lang/String;)V", (void *) sendtoServer},
};


//服务端接收客户端命令
void receiveClientInfo(JNIEnv* env, jobject thiz, jobject listener)
{
	Server* server = (Server*) getServerObj(env, thiz);
	if (server == NULL)
	{
		LOGD("get server error");
		return;
	}

	jclass clazz = env->GetObjectClass(listener);
	jmethodID methodid = env->GetMethodID(clazz, "onReceive", "(Ljava/lang/String;)V");
	server->receiveClient(env, thiz, listener, methodid);
}

//服务端接收心跳
void listenHeartBeat( JNIEnv* env, jobject thiz, jobject listener)
{
	Server* server = (Server*) getServerObj(env, thiz);
	if (server == NULL)
	{
		LOGD("get server error");
		return;
	}

	jstring ip = getAddress(env, thiz);
	jclass clazz = env->GetObjectClass(listener);
	jmethodID methodid = env->GetMethodID(clazz, "onFind", "(Ljava/lang/String;)V");

	char* ipstr = (char*)env->GetStringUTFChars(ip, NULL);
	if (ipstr == NULL)
	{
		return ;
	}

	server->receiveBroadCast(env, thiz, ipstr, listener, methodid);
	env->ReleaseStringUTFChars(ip, ipstr);
}

//服务端发送命令给客户端
void sendtoClient( JNIEnv* env, jobject thiz, jstring ip, jstring data)
{
	Server* server = (Server*) getServerObj(env, thiz);
	if (server == NULL)
	{
		LOGD("get server error");
		return;
	}

	LOGD("sendtoback  and local ip = %s", (char*)env->GetStringUTFChars(ip, NULL));
	server->sendtoClt((char*)env->GetStringUTFChars(ip, NULL), (char*)env->GetStringUTFChars(data, NULL));
}


void init_server(JNIEnv* env, jobject obj)
{
	Server* server = new Server();
	if (server == NULL)
	{
		LOGD("init server error!");
		return;
	}

	jclass cls = env->FindClass(serverName);
	jfieldID field = env->GetFieldID(cls, "mObject", "I");
    env->SetIntField(obj, field, (jint) server);
}

Server* getServerObj(JNIEnv* env, jobject obj)
{
	jclass cls = env->FindClass(serverName);
	jfieldID field = env->GetFieldID(cls, "mObject", "I");
	Server* server = (Server*) env->GetIntField(obj, field);
	return server;
}

void destory_server(JNIEnv* env, jobject obj)
{
	Server* server = getServerObj(env, obj);
	if (server == NULL)
	{
		LOGD("init server error!");
		return;
	}

	jclass cls = env->FindClass(serverName);
	jfieldID field = env->GetFieldID(cls, "mObject", "I");

	delete server;
	env->SetIntField(obj, field, (jint) 0);
}

void receive_files(JNIEnv* env, jobject obj, jstring ip, jstring name, jstring newname)
{
	Server* server = getServerObj(env, obj);
	if (server == NULL)
	{
		LOGD("init server error!");
		return;
	}

	server->receiveFiles((char*)env->GetStringUTFChars(ip, NULL),
			(char*)env->GetStringUTFChars(name, NULL),
			(char*)env->GetStringUTFChars(newname, NULL));
}

//服务端
static JNINativeMethod gMethods_server[] =
{
	{ "native_receiveFiles", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V", (void *) receive_files},
	{ "native_init", "()V", (void *) init_server},
	{ "native_destroy", "()V", (void *) destory_server},
    { "native_listenHeartBeat", "(Lcom/ijidou/transfer/Server$OnFindClientListener;)V", (void *) listenHeartBeat },
    { "native_receiveClientInfo", "(Lcom/ijidou/transfer/Server$OnReceivedFromClientListener;)V", (void *) receiveClientInfo },
    { "native_sendtoClient", "(Ljava/lang/String;Ljava/lang/String;)V", (void *) sendtoClient },
};

int register_location_methods(JNIEnv *env)
{
    jclass clazz;

    clazz = env->FindClass(clientName);

    if (clazz == NULL)
    {
        LOGE("Can't find class %s\n", clientName);
        return -1;
    }

    LOGD("register native methods");

    if (env->RegisterNatives(clazz, gMethods_client, sizeof(gMethods_client) / sizeof(gMethods_client[0])) != JNI_OK)
    {
        LOGE("Failed registering methods for %s\n", clientName);
        return -1;
    }

    clazz = env->FindClass(serverName);

	if (clazz == NULL)
	{
		LOGE("Can't find class %s\n", serverName);
		return -1;
	}

    if (env->RegisterNatives(clazz, gMethods_server, sizeof(gMethods_server) / sizeof(gMethods_server[0])) != JNI_OK)
	{
		LOGE("Failed registering methods for %s\n", serverName);
		return -1;
	}

    return 0;
}

jint JNI_OnLoad(JavaVM* vm, void *reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

    LOGD("%s: +", __FUNCTION__);

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
    {
        LOGE("ERROR: GetEnv failed.\n");
        return result;
    }

    if( register_location_methods(env) < 0 )
    {
        LOGE("ERROR: register location methods failed.\n");
        return result;
    }

    return JNI_VERSION_1_4;
}



void JNI_OnUnload(JavaVM* vm, void *reserved)
{
    return;
}

#ifdef __cplusplus
}
#endif
