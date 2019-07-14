#include <sys/types.h>
#include <sys/socket.h>
#include <sys/event.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define PORT 5001
#define MAX_EVENT_COUNT 64

int createSocket()
{
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        printf("socket() failed:%d\n",errno);
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval));

    if (bind(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr)) == -1)
    {
        printf("bind() failed:%d\n",errno);
        return -1;
    }

    if (listen(sock, 5) == -1)
    {
        printf("listen() failed:%d\n",errno);
        return -1;
    }

    return sock;
}

int main(int argc, const char * argv[])
{
    int listenfd = createSocket();
    if (listenfd == -1)
        return -1;

    int kq = kqueue();
    if (kq == -1)
    {
        printf("kqueue failed:%d",errno);
        return -1;
    }

    struct kevent event = {listenfd,EVFILT_READ,EV_ADD,0,0,NULL};
    int ret = kevent(kq, &event, 1, NULL, 0, NULL);
    if (ret == -1)
    {
        printf("kevent failed:%d",errno);
        return -1;
    }

    while (true)
    {
        struct kevent eventlist[MAX_EVENT_COUNT];
        struct timespec timeout = {5,0};
        int ret = kevent(kq, NULL, 0, eventlist, MAX_EVENT_COUNT, &timeout);
        if (ret <= 0)
            continue;

        for (int i=0; i<ret; i++)
        {
            struct kevent event = eventlist[i];
            int sock = (int)event.ident;
            int16_t filter = event.filter;
            uint32_t flags = event.flags;
            intptr_t data = event.data;

            //有新的客户端链接
            if (sock == listenfd)
            {
                socklen_t client_addrlen = 4;
                struct sockaddr client_addrlist[client_addrlen];
                int clientfd = accept(listenfd, client_addrlist, &client_addrlen);
                if (clientfd > 0)
                {
                    struct kevent changelist[2];
                    EV_SET(&changelist[0], clientfd, EVFILT_READ, EV_ADD, 0, 0, NULL);
                    EV_SET(&changelist[1], clientfd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
                    kevent(kq, changelist, 1, NULL, 0, NULL);
                }
                continue;
            }

            //异常事件
            if (flags & EV_ERROR)
            {
                close(sock);
                struct kevent event = {sock,EVFILT_READ,EV_DELETE,0,0,NULL};
                kevent(kq, &event, 1, NULL, 0, NULL);
                printf("socket broken,error:%ld\n",data);
                continue;
            }

            //数据可读
            if (filter == EVFILT_READ)
            {
                char buffer[data];
                memset(buffer, '\0', data);
                ssize_t recvlen = recv(sock, buffer, data, 0);
                if (recvlen <= 0)
                {
                    //链接断开
                    close(sock);
                    struct kevent event = {sock,EVFILT_READ,EV_DELETE,0,0,NULL};
                    kevent(kq, &event, 1, NULL, 0, NULL);
                    printf("socket broken!\n");
                    continue;
                }

                printf("%s\n",buffer);
            }

            //数据可写
            if (filter == EVFILT_WRITE)
            {
                char buffer[data];
                memset(buffer, 'a', data);
                ssize_t sendlen = send(sock, buffer, data, 0);
                if (sendlen <= 0)
                {
                    //链接断开
                    close(sock);
                    struct kevent event = {sock,EVFILT_READ,EV_DELETE,0,0,NULL};
                    kevent(kq, &event, 1, NULL, 0, NULL);
                    printf("socket broken!\n");
                    continue;
                }
            }

        }
    }

    return 0;
}
