#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#define MAX 10
void Usage(const char *proc)
{
	printf("%s [local_ip] [local_port]\n",proc);
}
int start_up(const char* local_ip,int local_port)
{
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock < 0)
	{
		perror("socket");
		exit(1);
	}
	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(local_port);
	local.sin_addr.s_addr = inet_addr(local_ip);
	if(bind(sock,(struct sockaddr*)&local,sizeof(local)) < 0)
	{
		perror("bind");
		exit(2);
	}
	if(listen(sock,MAX) < 0)
	{
		perror("listen");
		exit(3);
	}
	return sock;
}
void * HandlerRequest(void *arg)
{
	int sock = (long)arg;
	char buf[10024];
	char *msg = "HTTP/1.0 200 OK <\r\n\r\n<html><h1>yingying beautiful</h1></html>\r\n";
	while(1)
	{
		ssize_t s = read(sock,buf,sizeof(buf)-1);
		if(s > 0){//read success
			buf[s] = 0;
			printf("Client Say#%s",buf);
			write(sock,msg,strlen(msg));
			break;
		}
		else if(0 == s)
		{
			printf("client quit\n");
			break;
		}
		else{
			perror("read");
			return (void *)2;
		}
	}
}
int main(int argc,char *argv[])
{
	if(argc != 3)
	{
		Usage(argv[0]);
		return 1;
	}
	int sock = start_up(argv[1],atoi(argv[2]));
	struct sockaddr_in client;
	socklen_t len = sizeof(client);
	while(1)
	{
		int new_sock = accept(sock,(struct sockaddr *)&client,&len);
		pthread_t id;
		pthread_create(&id,NULL,HandlerRequest,(void *)new_sock);
		pthread_detach(id);
	}
	return 0;
}
