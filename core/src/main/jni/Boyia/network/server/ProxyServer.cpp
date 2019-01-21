#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/sendfile.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_EVENTS 10
#define LISTENQ 20
#define PORT 5000

namespace yanbo
{
class ProxyServer
{
public:
	ProxyServer()
	{
        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        setNonblock(m_socket);

        struct sockaddr_in local;
        bzero (&local, sizeof(local));
		local.sin_family = AF_INET;
		local.sin_addr.s_addr = htonl(INADDR_ANY);;
		local.sin_port = htons(PORT);
		if (bind(m_socket, (struct sockaddr *) &local, sizeof(local)) < 0)
		{
			return;
		}

		listen(m_socket, LISTENQ);

		m_epoll = epoll_create(MAX_EVENTS);
		
	}

	void start() 
	{
		if (m_epoll == -1)
		{
			return;
		}

        struct sockaddr_in client;
        struct epoll_event ev;
        ev.events = EPOLLIN;
	    ev.data.fd = m_socket;
		if (epoll_ctl(m_epoll, EPOLL_CTL_ADD, m_socket, &ev) == -1)
		{
			return;
		}

        struct epoll_event events[MAX_EVENTS];

        int nfds = -1, connfd;
        socklen_t addrlen;
		for (;;)
		{
			nfds = epoll_wait(m_epoll, events, MAX_EVENTS, -1);
			if (nfds == -1)
			{
				return;
			}

			for (int i = 0; i < nfds; ++i)
			{
				int sockfd = events[i].data.fd;
				if (sockfd == m_socket)
				{
					while ((connfd = accept(m_socket, (struct sockaddr *)&client, &addrlen)) > 0)
					{
	                    char *ipaddr = inet_ntoa(client.sin_addr);
	                    printf("accept a connection from [%s]\n", ipaddr);
						setNonblock(connfd);
						ev.events = EPOLLIN | EPOLLET;
						ev.data.fd = connfd;
						if (epoll_ctl(m_epoll, EPOLL_CTL_ADD, connfd, &ev) == -1)
						{
							return;
						}
					}
					if (connfd == -1)
					{
						if (errno != EAGAIN && errno != ECONNABORTED && errno != EPROTO && errno != EINTR)
							return;
					}
					continue;
				}
				if (events[i].events & EPOLLIN)
				{
					int n = 0, nread = -1;
					while ((nread = read(sockfd, buf + n, BUFSIZ - 1)) > 0)
					{
						n += nread;
					}
					if (nread == -1 && errno != EAGAIN)
					{
						printf("read error");
					}
	                printf("recv from client data [%s]\n", buf);
					ev.data.fd = sockfd;
					ev.events = events[i].events | EPOLLOUT;
					if (epoll_ctl(m_epoll, EPOLL_CTL_MOD, sockfd, &ev) == -1)
					{
						printf("epoll_ctl: mod");
					}
				}
				if (events[i].events & EPOLLOUT)
				{
					snprintf(buf, sizeof(buf), "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\nHello World", 11);
					int nwrite, data_size = strlen(buf);
					int n = data_size;
					while (n > 0)
					{
						nwrite = write(sockfd, buf + data_size - n, n);
						if (nwrite < n)
						{
							if (nwrite == -1 && errno != EAGAIN)
							{
								printf("write error");
							}
							break;
						}
						n -= nwrite;
					}
	                printf("send to client data [%s]\n", buf);
					close(sockfd);
	                events[i].data.fd = -1;
				}
			}
		}

	}
	~ProxyServer()
	{

	}

private:
    void setNonblock(int fd)
    {
		int opts = fcntl(fd, F_GETFL);
		if (opts < 0)
		{
			return;
		}
		opts = (opts | O_NONBLOCK);
		if (fcntl(fd, F_SETFL, opts) < 0)
		{
			return;
		}
    }

	int m_socket;
	int m_epoll;
	char buf[1024];
};
}