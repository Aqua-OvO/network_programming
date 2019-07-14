#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<string.h>
static void usage(const char* proc)
{
 printf("Usage:%s[local-ip][local-port]\n",proc);
}
static int start_up(const char *local_ip,int local_port)
{
    //1.create sock
    int sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock < 0)
    {
        perror("socket");
        close(sock);
        exit(1);
    }
    //2,createbind
    struct sockaddr_in local;
    bzero(&local,sizeof(local));
    local.sin_family = AF_INET;
    local.sin_port = htons(local_port);
    local.sin_addr.s_addr = inet_addr(local_ip);
    if( bind(sock,(struct sockaddr *)&local,sizeof(local)) < 0)
    {
        perror("bind");
        close(sock);
        exit(2);
    }
    //3.listen
    if(listen(sock,10) < 0)
    {
     perror("listen");
    close(sock);
     exit(3);
    }
    return sock;

}
int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        usage(argv[0]);
    }
    int sock = start_up(argv[1],atoi(argv[2]));
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    while(1)
    {
        int new_sock = accept(sock,(struct sockaddr*)&client, &len);
        if(new_sock < 0)
        {
            perror("accept");
            close(sock);
            return 1;
        }
        printf("client_ip:%s client_port:%d\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
        char buf[1024];
        while(1)
        {
            ssize_t s = read(new_sock,buf,sizeof(buf)-1);
            if(s > 0)
            {
                buf[s] = 0;
                printf("client say# %s",buf);
            }
            else if(s == 0)
            {
                printf("client quit!\n");
                break;
            }
            write(new_sock,buf, strlen(buf));
        }
    }

    close(sock);
    return 0;
}
