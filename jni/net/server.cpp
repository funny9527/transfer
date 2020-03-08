#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <jni.h>
#include "server.h"
#include "log.h"

Server::Server()
{
	started = 1;
}

Server::~Server()
{
	LOGD("%s", "[server] stopped ******************");
	started = 0;
}

/**
 * 接收设备接入广播
 */
int Server::receiveBroadCast(JNIEnv* env, jobject thiz, char* ip, jobject listener, jmethodID methodid)
{
	LOGD("[server] start listen broadcast... ip = %s", ip);
	setvbuf(stdout, NULL, _IONBF, 0);
	fflush(stdout);

	struct sockaddr_in addrto;
	memset(&addrto, 0, sizeof(struct sockaddr_in));
	addrto.sin_family = AF_INET;
	addrto.sin_addr.s_addr = htonl(INADDR_ANY);
	addrto.sin_port = htons(PORT);

	struct sockaddr_in from;
	memset(&from, 0, sizeof(struct sockaddr_in));
	from.sin_family = AF_INET;
	from.sin_addr.s_addr = htonl(INADDR_ANY);
	from.sin_port = htons(PORT);

	int sock = -1;
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		LOGD("%s", "[server] socket error");
		return -1;
	}

	const int opt = 1;
	int nb = 0;
	nb = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));
	if(nb == -1)
	{
		LOGD("%s", "[server] set socket error...");
		return -1;
	}

	if(bind(sock,(struct sockaddr *)&(addrto), sizeof(struct sockaddr_in)) == -1)
	{
		LOGD("%s", "[server] bind error...");
		return -1;
	}

	int len = sizeof(struct sockaddr_in);
	char smsg[100] = {0};

	while(started)
	{
		int ret = recvfrom(sock, smsg, 100, 0, (struct sockaddr*)&from,(socklen_t*)&len);

		LOGD("[server] server started == %d", started);

		if(ret <= 0)
		{
			LOGD("%s", "[server] read error....");
		}
		else
		{
			LOGD("[server] receive = %s", smsg);
			jstring result = env->NewStringUTF(smsg);
			env->CallObjectMethod(listener, methodid, result);
			env->DeleteLocalRef(result);

			char str[strlen(ip) + 1];
			strcpy(str, ip);
			sendtoClt(smsg, str);
		}

		sleep(1);
	}

	close(sock);

	return 0;
}

/**
 * 返回服务端地址信息
 */
int Server::sendtoClt(char* ip, char* data)
{
	int sockfd;
	struct sockaddr_in servaddr;

	sockfd = socket(PF_INET, SOCK_DGRAM, 0);

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(BACKPORT);
	servaddr.sin_addr.s_addr = inet_addr(ip);

	int len = strlen(data) + 1;
	char sendline[len];
	sprintf(sendline, data);

	LOGD("[server] send to clinet : %s", sendline);
	sendto(sockfd, sendline, strlen(sendline), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

	close(sockfd);
}


void Server::receiveClient(JNIEnv* env, jobject thiz, jobject listener, jmethodID methodid)
{
	LOGD("%s", "[sever] receive from client");
	int sockfd;
	struct sockaddr_in servaddr;

	sockfd = socket(PF_INET, SOCK_DGRAM, 0);

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(ORDERPORT);

	bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	char recvline[UDP_PACKAGE_LENGTH];
	//char feedback[65536];

	while (1)
	{
		memset(recvline, 0, sizeof(recvline));
	    recvfrom(sockfd, recvline, UDP_PACKAGE_LENGTH, 0, NULL, NULL);

	    LOGD("[server] receive info from client %s", recvline);
	    if (strlen(recvline) > 0)
	    {
	    	jstring result = env->NewStringUTF(recvline);
			env->CallObjectMethod(listener, methodid, result);
			env->DeleteLocalRef(result);
//	    	int headsize = strlen(BODY_HEAD);
//	    	char head[headsize + 1];
//	    	memset(head, 0, sizeof(head));
//	    	strncpy(head, recvline, headsize);
//	    	LOGD("[server] receive info from client head = %s", head);
//	    	if (strcmp(head, BODY_HEAD))
//	    	{
//	    		char body[UDP_PACKAGE_LENGTH];
//	    		memset(body, 0, sizeof(body));
//	    		strncpy(body, recvline + headsize, UDP_PACKAGE_LENGTH - headsize);
//
//	    		LOGD("[server] receive info from client body = %s", body);
//	    		strcat(feedback, body);
//	    	}
//	    	else if (strcmp(head, BODY_LAST))
//	    	{
//	    		int last = strlen(BODY_LAST);
//	    		char body[UDP_PACKAGE_LENGTH];
//	    		memset(body, 0, sizeof(body));
//				strncpy(body, recvline + last, UDP_PACKAGE_LENGTH - last);
//
//				LOGD("[server] receive info from client tear = %s", body);
//				strcat(feedback, body);
//
//				jstring result = env->NewStringUTF(feedback);
//				env->CallObjectMethod(listener, methodid, result);
//				env->DeleteLocalRef(result);
//				memset(feedback, 0, sizeof(feedback));
//	    	}
	    }

	    sleep(1);
	}

	close(sockfd);
}

int Server::receiveFiles(char* ip, char* file_name, char* new_name)
{
	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = htons(INADDR_ANY);
	client_addr.sin_port = htons(0);

	int client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket < 0)
	{
		LOGD("Receive Create Socket Failed!\n");
		return -1;
	}

	if (bind(client_socket, (struct sockaddr*)&client_addr, sizeof(client_addr)))
	{
		LOGD("Receive Bind Port Failed!\n");
		return -1;
	}

	struct sockaddr_in  server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;

	if (inet_aton(ip, &server_addr.sin_addr) == 0)
	{
		LOGD("Receive IP Address Error!\n");
		return -1;
	}

	server_addr.sin_port = htons(FILE_PORT);
	socklen_t server_addr_length = sizeof(server_addr);

	if (connect(client_socket, (struct sockaddr*)&server_addr, server_addr_length) < 0)
	{
		LOGD("Receive Can Not Connect To %s!\n", ip);
		return -1;
	}

	char buffer[BUFFER_SIZE];
	memset(buffer, 0, sizeof(buffer));
	strncpy(buffer, file_name, strlen(file_name) > BUFFER_SIZE ? BUFFER_SIZE : strlen(file_name));
	send(client_socket, buffer, BUFFER_SIZE, 0);

	FILE *fp = fopen(new_name, "w");
	if (fp == NULL)
	{
		LOGD("File:\t%s Can Not Open To Write!\n", new_name);
		return -1;
	}

	memset(buffer, 0, sizeof(buffer));
	int length = 0;
	while(length = recv(client_socket, buffer, BUFFER_SIZE, 0))
	{
		if (length < 0)
		{
			LOGD("Recieve Data From Server %s Failed!\n", ip);
			break;
		}

		int write_length = fwrite(buffer, sizeof(char), length, fp);
		if (write_length < length)
		{
			LOGD("File:\t%s Write Failed!\n", new_name);
			break;
		}
		memset(buffer, 0, BUFFER_SIZE);
		LOGD("receive from %s\n", new_name);
	}

	LOGD("Recieve File:\t %s From Server[%s] Finished!\n", new_name, ip);

	fclose(fp);
	close(client_socket);
	return 0;
}
