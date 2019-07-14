#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
static void usage(const char* proc)
{
	printf("Usage:%s [server-ip] [server-port]",proc);
}
int main(int argc,char *argv[])
{
	if(argc != 3)
	{
		usage(argv[0]);
	}
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock < 0)
	{
		perror("socket");
		return 1;
	}
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[2]));
	server.sin_addr.s_addr = inet_addr(argv[1]);
	if(connect(sock,(struct sockaddr *)&server,sizeof(server)) < 0)
	{
		perror("connect");
		return 2;
	}
	while(1)
	{
		printf("please Entry#");
		fflush(stdout);
		char buf[1024];
		ssize_t s = read(0,buf,sizeof(buf)-1);
		if(s > 0)//read success
		{
			buf[s] = 0;
		}
		write(sock,buf,strlen(buf));
		ssize_t _s = read(sock,buf,sizeof(buf)-1);
		if(_s > 0)
		{
			buf[_s - 1] = 0;
			printf("server echo# %s\n",buf);
		}
	}
	close(sock);
	return 0;
}
