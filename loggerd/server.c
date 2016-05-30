#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#include "client.h"
#include "types.h"
#include "mylog.h"
#include "parserthread.h"

ConfData configData;

static void process_signal(int s);
static void set_signal();
static int handle(Client *pClient);

int get_args(int argc, char*argv[])
{
    if(argc!=6)
    {
        printf("path <local port> <thread number> <buffer size(bytes)> <buffer number in block> <block number>\n");
        return -1;
    }

    configData.local_port = atoi(argv[1]);
    configData.thread_amount = atoi(argv[2]);
    configData.size_of_buffer = (size_t)atoi(argv[3]);
    configData.buffer_amount_in_block = atoi(argv[4]);
    configData.block_amount = atoi(argv[5]);
    return 0;
}

int setnonblocking(int sockfd)
{
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK) == -1)
    {
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    set_debug_flag(L_DBG);

    configData.stop = get_args(argc, argv);
    if (configData.stop)
    {
        return -1;
    }

    set_signal();

    if(0 != init_mem())
    {
        configData.stop = 1;
        mylog(configData.logfile, L_ERR, "init mem failed!!!");
        printf("init mem failed\n");
    }

    int listenq = 1024;
    int listenfd, connfd, kdpfd, nfds, n, curfds,acceptCount = 0;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t socklen = sizeof(struct sockaddr_in);
    struct epoll_event ev;
    struct epoll_event *events = calloc(1, sizeof(struct epoll_event)*configData.block_amount);
    Client *clients = calloc(1,configData.block_amount*sizeof(Client));
    init_clients(clients, configData.block_amount);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
    servaddr.sin_port = htons(configData.local_port);

    if(!configData.stop)
    {
        listenfd = socket(AF_INET, SOCK_STREAM, 0);
        if (listenfd == -1)
        {
            perror("can't create socket file");
            mylog(configData.logfile, L_ERR, "create cocket error(%s)!!", strerror(errno));
            configData.stop = 1;
        }
    }

    if (!configData.stop)
    {
        if(setnonblocking(listenfd) < 0)
        {
            mylog(configData.logfile, L_ERR, "set non blocking error(%s)!!", strerror(errno));
            configData.stop = 1;
            perror("setnonblock error");
        }
    }

    if(!configData.stop)
    {
        if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(struct sockaddr)) == -1)
        {
            perror("bind error");
            mylog(configData.logfile, L_ERR, "socket bind error(%s)!!", strerror(errno));
            configData.stop = 1;
        }
    }

    if(!configData.stop)
    {
        if (listen(listenfd, listenq) == -1)
        {
            perror("listen error");
            mylog(configData.logfile, L_ERR, "socket listen on %d error(%s)!!", configData.local_port, strerror(errno));
            configData.stop = 1;
        }
    }

    if(!configData.stop)
    {
        /* 创建 epoll 句柄，把监听 socket 加入到 epoll 集合里 */
        kdpfd = epoll_create(configData.block_amount);
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = listenfd;
        if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, listenfd, &ev) < 0)
        {
            //fprintf(stderr, "epoll set insertion error: fd=%d\n", listenfd);
            mylog(configData.logfile, L_ERR, "epoll set insertion error: fd=%d error(%s)!!",
                  listenfd, strerror(errno));
            configData.stop = 1;
        }
        else
        {
            curfds = 1;

            printf("epollserver startup,port %d, max connection is %d, backlog is %d\n",
                   configData.local_port, configData.block_amount, listenq);

            mylog(configData.logfile, L_INFO, "epollserver startup,port %d, max connection is %d, backlog is %d\n",
                  configData.local_port, configData.block_amount, listenq);
        }
    }

    if(!configData.stop)
    {
        if(0!=create_parse_threads())
        {
            printf("error create_parse_threads failed!!");
            configData.stop = 1;
        }
    }

    for (;!configData.stop;)
    {
        /* 等待有事件发生 */
        nfds = epoll_wait(kdpfd, events, curfds, -1);
        if (configData.stop)
        {
            break;
        }

        if (nfds == -1)
        {
            perror("epoll_wait");
            continue;
        }
        /* 处理所有事件 */
        for (n = 0; n < nfds; ++n)
        {
            if (events[n].data.fd == listenfd)
            {
                connfd = accept(listenfd, (struct sockaddr *)&cliaddr,&socklen);
                if (connfd < 0)
                {
                    perror("accept error");
                    mylog(configData.logfile, L_ERR, "socket accept error(%s)!!", strerror(errno));
                    continue;
                }

                char buf[1024];
                sprintf(buf, "accept form %s:%d\n", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
                printf("%d:%s", ++acceptCount, buf);

                if (curfds >= (int)configData.block_amount)
                {
                    fprintf(stderr, "too many connection, more than %d\n",
                            configData.block_amount);

                    mylog(configData.logfile, L_WRN, "too many connection, more than %d\n",
                          configData.block_amount);

                    close(connfd);
                    continue;
                }

                if (setnonblocking(connfd) < 0)
                {
                    perror("setnonblocking error");
                    mylog(configData.logfile, L_ERR, "setnonblocking error(%s)", strerror(errno));
                }

                Client *pClient = get_one_free_client(clients, configData.block_amount);
                if(pClient == NULL)
                {
                    configData.stop = 1;
                    mylog(configData.logfile, L_ERR, "no more free client to find max=%d", configData.block_amount);
                    return -1;
                }
                strcpy(pClient->szIP, inet_ntoa(cliaddr.sin_addr));
                pClient->nPort = cliaddr.sin_port;
                pClient->nfd = connfd;
                pClient->free = false;

                Block *pBlock = get_free_block();
                if(pBlock == NULL)
                {
                    configData.stop = 1;
                    mylog(configData.logfile, L_ERR, "no more free b block find max=%d", configData.block_amount);
                    return -1;
                }
                pBlock->free = false;
                pClient->pBlock = pBlock;

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = connfd;
                ev.data.ptr = pClient;
                if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, connfd, &ev) < 0)
                {
                    fprintf(stderr, "add socket '%d' to epoll failed: %s\n", connfd, strerror(errno));
                    configData.stop = 1;
                    mylog(configData.logfile, L_ERR, "add socket '%d' to epoll failed: %s", connfd, strerror(errno));
                    break;
                }
                curfds++;
                continue;
            }
            // 处理客户端请求
            if (handle((Client*)events[n].data.ptr) < 0)
            {
                epoll_ctl(kdpfd, EPOLL_CTL_DEL, events[n].data.fd,&ev);
                curfds--;
            }
        }
    }

    close(listenfd);
    close_clients(clients, configData.block_amount);
    free(clients);
    free(events);
    release_mem();
    return 0;
}

static void process_signal(int s)
{
    printf("got signal %d\n",s);
    mylog(configData.logfile, L_INFO, "got signal %d", s);
    configData.stop = 1;
}

static void set_signal()
{
    int i = 1;
    for (; i < 32; i++)
    {
        signal(i, process_signal);
    }
}

ssize_t read_socket_to_recv_buffer(Client *in_client, ThreadData *out_thread_data)
{
    out_thread_data->free = false;
    int connfd = in_client->nfd;
    Block *block = in_client->pBlock;
    RecvBuffer *pRecvBuffer = get_next_free_recv_buffer(block);
    if (pRecvBuffer == NULL)
    {
        configData.stop = 1;
        mylog(configData.logfile, L_ERR, "get free recvbuffer failed!!");
        printf("get free recvbuffer failed!!");
        out_thread_data->free = true;
        return -1;
    }

    ssize_t nLeftLength = configData.size_of_buffer-1;
    if (pRecvBuffer->data_end_ptr!=pRecvBuffer->buf_start)
    {
        nLeftLength=nLeftLength-(pRecvBuffer->data_end_ptr-pRecvBuffer->buf_start+1);
    }

    pRecvBuffer->buf_end = '\0';
    uchar *write_ptr = (pRecvBuffer->data_end_ptr==pRecvBuffer->buf_start)?
                (pRecvBuffer->buf_start):(pRecvBuffer->data_end_ptr+1);
    ssize_t nread = read(connfd, write_ptr, nLeftLength);//读取客户端socket流
    if (nread == 0)
    {
        printf("client close the connection\n");
        mylog(configData.logfile, L_INFO, "client close the connection: %s %d",
              in_client->szIP, in_client->nPort);
        in_client->free = true;
        reset_block(block);
        out_thread_data->free = true;

        return -1;
    }

    if (nread < 0 && errno != EINTR)
    {
        perror("read error");
        close(connfd);
        in_client->free = true;
        reset_block(block);
        out_thread_data->free = true;
        return -1;
    }

    uchar *line_start = NULL;
    uchar *line_end = NULL;
    int have_multi_line = 0;
    bool have_half_line = get_end_half_line(pRecvBuffer->buf_start,
                                            write_ptr+nread-1,
                                            &line_start,
                                            &line_end,
                                            &have_multi_line);

    //1.one harf line "111111"
    if(have_multi_line==0 && have_half_line==true)
    {
        pRecvBuffer->data_end_ptr = write_ptr+nread-1;
        out_thread_data->free = true;
        return 0;
    }
    //2.one or more full lines "111\r\n2222\r\n"
    else if(have_multi_line==1 && have_half_line==false)
    {
        block->bufIndexToWrite++;
        if (block->bufIndexToWrite>=configData.buffer_amount_in_block)
        {
            block->bufIndexToWrite = 0;
        }
        pRecvBuffer->data_end_ptr = write_ptr+nread-1;
        out_thread_data->recv_buffer = pRecvBuffer;
        out_thread_data->have_data = true;
    }
    //3.one or more full lines and harf line "111\n22222"
    else
    {
        block->bufIndexToWrite++;
        if (block->bufIndexToWrite>=configData.buffer_amount_in_block)
        {
            block->bufIndexToWrite = 0;
        }

        RecvBuffer *second_recv_buffer = get_next_free_recv_buffer(block);
        if(second_recv_buffer == NULL)
        {
            printf("no more free recv buffer to get!!");
            mylog(configData.logfile, L_ERR, "no more free recv buffer to get!!");
            configData.stop = 1;
            out_thread_data->free = true;
            return -1;
        }
        size_t copy_len = line_end-line_start+1;
        memcpy(second_recv_buffer->data_start_ptr, line_start, copy_len);
        second_recv_buffer->data_end_ptr = second_recv_buffer->data_end_ptr+copy_len-1;
        *line_start = '\0';
        pRecvBuffer->data_end_ptr = pRecvBuffer->data_end_ptr+nread-copy_len;
        out_thread_data->recv_buffer = pRecvBuffer;
        out_thread_data->have_data = true;
    }
    return nread;
}

int handle(Client *pClient)
{
    ThreadData *pThreadData = get_free_thread_data();
    if(pThreadData == NULL)
    {
        configData.stop = 1;
        mylog(configData.logfile, L_ERR, "can not find free thread!!\n");
        return -1;
    }

    return read_socket_to_recv_buffer(pClient, pThreadData);
}

