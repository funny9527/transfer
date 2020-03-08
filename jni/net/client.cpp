#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <jni.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "client.h"
#include "log.h"

Client::Client()
{
	find = -1;
	started = 1;
}

Client::~Client()
{
	started = 0;
}

int Client::sendBroadCast(char* ip)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	fflush(stdout);

	int sock = -1;
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		LOGD("%s", "[client] socket error");
		return -1;
	}

	const int opt = 1;
	int nb = 0;
	nb = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));
	if(nb == -1)
	{
		LOGD("%s", "[client] set socket error...");
		return -1;
	}

	struct sockaddr_in addrto;
	memset(&addrto, 0, sizeof(struct sockaddr_in));
	addrto.sin_family = AF_INET;
	addrto.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	addrto.sin_port = htons(PORT);
	int nlen = sizeof(addrto);

	while(find < 0)
	{
		int ret = sendto(sock, ip, strlen(ip), 0, (struct sockaddr*)&addrto, nlen);
		if(ret < 0)
		{
			LOGD("%s", "[client] send error....");
		}
		else
		{
			LOGD("[client] send %s", ip);
		}
		sleep(2);
	}

	return 0;
}

/**
 * 服务端确认连接
 */
void Client::receiveBroadCastBack(JNIEnv* env, jobject thiz, jobject listener, jmethodID methodid)
{
	LOGD("%s", "[client] receive from server");
	int sockfd;
	struct sockaddr_in servaddr;

	sockfd = socket(PF_INET, SOCK_DGRAM, 0);

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(BACKPORT);

	bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	int n;
	char recvline[1024];

	while (started)
	{
	    recvfrom(sockfd, recvline, 1024, 0, NULL, NULL);

	    LOGD("[client] receive feedback form server %s", recvline);
	    if (strlen(recvline) > 0)
	    {
	    	find = 1;
	    	jstring result = env->NewStringUTF(recvline);
	    	env->CallObjectMethod(listener, methodid, result);
	    	env->DeleteLocalRef(result);
	    }

	    sleep(1);
	}

	close(sockfd);
}

int Client::sendtoSer(char* ip, char* data)
{
	int sockfd;
	struct sockaddr_in servaddr;

	sockfd = socket(PF_INET, SOCK_DGRAM, 0);

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(ORDERPORT);
	servaddr.sin_addr.s_addr = inet_addr(ip);

	char sendline[UDP_PACKAGE_LENGTH];
	memset(sendline, 0, sizeof(sendline));

	sprintf(sendline, data);

	LOGD("[client] send packat : %s", sendline);
	sendto(sockfd, sendline, strlen(sendline), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

	close(sockfd);
}

//int Client::sendtoSer(char* ip, char* data)
//{
//	char sendline[UDP_PACKAGE_LENGTH];
//	int length = strlen(data);
//	//////////////////////////////////
//	LOGD("[client] send info length: %d", length);
//	int size = UDP_PACKAGE_LENGTH - strlen(BODY_HEAD) - 1;
//	LOGD("[client] send info body length: %d", strlen(BODY_HEAD));
//	LOGD("[client] send info body size: %d", size);
//
//	int step = length / size + (length % size == 0 ? 0 : 1);
//
//	LOGD("[client] send info body step: %d", step);
//	char piece[size];
//	for (int i = 0; i < step - 1; i++)
//	{
//		memset(piece, 0, sizeof(piece));
//		strncpy(piece, data + i * size, size);
//		LOGD("[client] send info body piece: %s", piece);
//		char send[size + strlen(BODY_HEAD)];
//		memset(send, 0, sizeof(send));
//		strcat(send, BODY_HEAD);
//		strcat(send, piece);
//
//		sprintf(sendline, send);
//		LOGD("[client] send info : %s", sendline);
//		sendPacket(ip, sendline);
//		sleep(1);
//	}
//
//	int end = length % size;
//	if (end == 0)
//	{
//		end = size;
//	}
//
//	memset(piece, 0, sizeof(piece));
//	LOGD("[client] send info body start  end: %d", end);
//	strncpy(piece, data + (step - 1) * size, end);
//	LOGD("[client] send info body piece: %s", piece);
//
//	char send[size + strlen(BODY_LAST)];
//	memset(send, 0, sizeof(send));
//	strcat(send, BODY_LAST);
//	strcat(send, piece);
//	LOGD("[client] send info send length: %d  %d", strlen(send), strlen(sendline));
//
//	sprintf(sendline, send);
//
//	LOGD("[client] send info : %s", sendline);
//	sendPacket(ip, sendline);
//}

int Client::sendFiles()
{
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	server_addr.sin_port = htons(FILE_PORT);

	int server_socket = socket(PF_INET, SOCK_STREAM, 0);
	if (server_socket < 0)
	{
		LOGD("File Create Socket Failed!\n");
		return -1;
	}

	if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)))
	{
		LOGD("File Server Bind Port: %d Failed!\n", FILE_PORT);
		return -1;
	}

	if (listen(server_socket, QUEUE_NUMBERS))
	{
		LOGD("File Server Listen Failed!\n");
		return -1;
	}

	while(1)
	{
		struct sockaddr_in client_addr;
		socklen_t length = sizeof(client_addr);

		int new_server_socket = accept(server_socket, (struct sockaddr*)&client_addr, &length);
		if (new_server_socket < 0)
		{
			LOGD("File Server Accept Failed!\n");
			break;
		}

		char buffer[BUFFER_SIZE];
		memset(buffer, 0, sizeof(buffer));
		length = recv(new_server_socket, buffer, BUFFER_SIZE, 0);
		if (length < 0)
		{
			LOGD("File Server Recieve Data Failed!\n");
			break;
		}

		char file_name[FILE_NAMT_LIMIT + 1];
		memset(file_name, 0, sizeof(file_name));
		strncpy(file_name, buffer,
				strlen(buffer) > FILE_NAMT_LIMIT ? FILE_NAMT_LIMIT : strlen(buffer));

		FILE *fp = fopen(file_name, "r");
		if (fp == NULL)
		{
			LOGD("File:\t%s Not Found!\n", file_name);
		}
		else
		{
			memset(buffer, 0, BUFFER_SIZE);
			int file_block_length = 0;
			while( (file_block_length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0)
			{
				LOGD("file_block_length = %d\n", file_block_length);

				// 发送buffer中的字符串到new_server_socket,实际上就是发送给客户端
				if (send(new_server_socket, buffer, file_block_length, 0) < 0)
				{
					LOGD("Send File:\t%s Failed!\n", file_name);
					break;
				}

				memset(buffer, 0, sizeof(buffer));
			}
			fclose(fp);
			LOGD("File:\t%s Transfer Finished!\n", file_name);
		}

		close(new_server_socket);
	}

	close(server_socket);

	return 0;
}
