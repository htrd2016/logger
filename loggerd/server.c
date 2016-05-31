#include "server.h"
#include "client.h"
#include "mylog.h"
#include "types.h"

static void process_signal(int s);
static void set_signal();

static struct epoll_event *events = 0;
Client *clients = 0;

int setnonblocking(int sockfd) {
  if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK) == -1) {
    return -1;
  }
  return 0;
}

int server(accept_callback accept_fun, read_callback read_fun) {
  int listenq = 1024;
  int listenfd, connfd, kdpfd, nfds, n, curfds, acceptCount = 0;
  struct sockaddr_in servaddr, cliaddr;
  socklen_t socklen = sizeof(struct sockaddr_in);
  struct epoll_event ev;
  events = calloc(1, sizeof(struct epoll_event) * configData.block_amount);
  init_clients();

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(configData.local_port);

  if (!configData.stop) {
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
      perror("can't create socket file");
      mylog(configData.logfile, L_ERR, "create cocket error(%s)!!",
            strerror(errno));
      configData.stop = 1;
    }
  }

  if (!configData.stop) {
    if (setnonblocking(listenfd) < 0) {
      mylog(configData.logfile, L_ERR, "set non blocking error(%s)!!",
            strerror(errno));
      configData.stop = 1;
      perror("setnonblock error");
    }
  }

  if (!configData.stop) {
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) ==
        -1) {
      perror("bind error");
      mylog(configData.logfile, L_ERR, "socket bind error(%s)!!",
            strerror(errno));
      configData.stop = 1;
    }
  }

  if (!configData.stop) {
    if (listen(listenfd, listenq) == -1) {
      perror("listen error");
      mylog(configData.logfile, L_ERR, "socket listen on %d error(%s)!!",
            configData.local_port, strerror(errno));
      configData.stop = 1;
    }
  }

  if (!configData.stop) {
    /* 创建 epoll 句柄，把监听 socket 加入到 epoll 集合里 */
    kdpfd = epoll_create(configData.block_amount);
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = listenfd;
    if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, listenfd, &ev) < 0) {
      // fprintf(stderr, "epoll set insertion error: fd=%d\n", listenfd);
      mylog(configData.logfile, L_ERR,
            "epoll set insertion error: fd=%d error(%s)!!", listenfd,
            strerror(errno));
      configData.stop = 1;
    } else {
      curfds = 1;

      printf(
          "epollserver startup,port %d, max connection is %d, backlog is %d\n",
          configData.local_port, configData.block_amount, listenq);

      mylog(
          configData.logfile, L_INFO,
          "epollserver startup,port %d, max connection is %d, backlog is %d\n",
          configData.local_port, configData.block_amount, listenq);
    }
  }

  set_signal();

  for (; !configData.stop;) {
    /* 等待有事件发生 */
    nfds = epoll_wait(kdpfd, events, curfds, -1);
    if (configData.stop) {
      break;
    }

    if (nfds == -1) {
      perror("epoll_wait");
      continue;
    }
    /* 处理所有事件 */
    for (n = 0; n < nfds; ++n) {
      if (events[n].data.fd == listenfd) {
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &socklen);
        if (connfd < 0) {
          perror("accept error");
          mylog(configData.logfile, L_ERR, "socket accept error(%s)!!",
                strerror(errno));
          continue;
        }

        ++acceptCount;

        char buf[1024];
        sprintf(buf, "accept form %s:%d\n", inet_ntoa(cliaddr.sin_addr),
                cliaddr.sin_port);
        printf("%d:%s", acceptCount, buf);

        if (curfds >= (int)configData.block_amount) {
          fprintf(stderr, "too many connection, more than %d\n",
                  configData.block_amount);

          mylog(configData.logfile, L_WRN,
                "too many connection, more than %d\n", configData.block_amount);

          close(connfd);
          continue;
        }

        if (setnonblocking(connfd) < 0) {
          perror("setnonblocking error");
          mylog(configData.logfile, L_ERR, "setnonblocking error(%s)",
                strerror(errno));
        }

        {
          void *userdata;
          int result = accept_fun(connfd, &cliaddr, &userdata);
          if (result) {
            configData.stop = 1;
            break;
          }

          ev.events = EPOLLIN | EPOLLET;
          ev.data.ptr = userdata;
          if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, connfd, &ev) < 0) {
            fprintf(stderr, "add socket '%d' to epoll failed: %s\n", connfd,
                    strerror(errno));
            configData.stop = 1;
            mylog(configData.logfile, L_ERR,
                  "add socket '%d' to epoll failed: %s", connfd,
                  strerror(errno));
            break;
          }
        }

        curfds++;
        continue;
      }
      // 处理客户端请求
      if (read_fun(connfd, &cliaddr, events[n].data.ptr) < 0) {
        epoll_ctl(kdpfd, EPOLL_CTL_DEL, events[n].data.fd, &ev);
        curfds--;
      }
    }
  }

  // wait for all pthread
  close(listenfd);
  close_clients();
  free(clients);
  free(events);
  return 0;
}

static void process_signal(int s) {
  printf("got signal %d\n", s);
  mylog(configData.logfile, L_INFO, "got signal %d", s);
  configData.stop = 1;
}

static void set_signal() {
  int i = 1;
  for (; i < 32; i++) {
    signal(i, process_signal);
  }
}
