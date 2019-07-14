#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<string.h>
static void Usage(const char *proc)
{
	printf("Usage:%s [local_ip] [local_port]\n",proc);
}
static int start_up(const char* local_ip,int local_port)
{
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock < 0)
	{
		perror("sock");
		exit(1);
	}
	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(local_port);
	local.sin_addr.s_addr = inet_addr(local_ip);
	if(bind(sock,(struct sockaddr *)&local,sizeof(local)) < 0)
	{
		perror("bind");
		exit(2);
	}
	if(listen(sock,10) < 0)
	{
		perror("listen");
		exit(3);
	}
	return sock;
}
int main(int argc, char*argv[])
{
	if(argc != 3)
	{
		Usage(argv[0]);
	}
	int sock = start_up(argv[1],atoi(argv[2]));
		struct sockaddr_in client;
		socklen_t len = sizeof(client);
		while(1)
		{
			int new_sock = accept(sock,(struct sockaddr*)&client,&len);
			if(new_sock < 0)
			{
				perror("accept");
				return 4;
			}
		pid_t id = fork();
		if(0 == id)
		{
			char buf[1024];
			while(1)
			{
				ssize_t s = read(new_sock,buf,sizeof(buf)-1);
				if(s > 0)
				{
					buf[s] = 0;
					printf("%s say# %s",inet_ntoa(client.sin_addr),buf);
					write(new_sock,buf,strlen(buf));
				}
				else if(0 == s)
				{
					printf("client quit\n");
					break;
				}
			}
		}
		else if(id > 0){//father
			pid_t _id = fork();
			if(0 == _id)
			{
				continue;//continue wait accept
			}
			else if(_id > 0)
			{
				close(new_sock);
				exit(0);
			}
			else
			{
				break;
			}
		}
		else
		{
			perror("fork");
			break;
		}
	}
	close(sock);
	return 0;
}
