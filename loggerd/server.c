#include "server.h"
#include "client.h"
#include "mylog.h"
#include "types.h"
#include <glib.h>
#include <glib/ghash.h>

static void process_signal(int s);
static void set_signal();

static struct epoll_event *events = 0;
EpollClient *clients = 0;

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
  GHashTable *epoll_hash_table = NULL;

  events = calloc(1, sizeof(struct epoll_event) * configData.block_amount);
  if(events == 0){
      mylog(configData.logfile, L_ERR, "calloc events memory failed!");
      configData.stop = 1;
  }

  if(0!=init_epoll_clients()){
      mylog(configData.logfile, L_ERR, "calloc epoll clients memory failed!");
      configData.stop = 1;
  }

  if(0!=init_client_datas()){
      mylog(configData.logfile, L_ERR, "calloc client data memory failed!");
      configData.stop = 1;
  }

  epoll_hash_table = g_hash_table_new(g_direct_hash, g_direct_equal);

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(configData.local_port);

  if (!configData.stop) {
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
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

    if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, listenfd, &ev) <
        0) { // listenfd:等待连接队列的最大长度
      mylog(configData.logfile, L_ERR,
            "epoll set insertion error: fd=%d error(%s)!!", listenfd,
            strerror(errno));
      configData.stop = 1;
    } else {
      curfds = 1;
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
    if (nfds == -1)  continue;

    /* 处理所有事件 */
    for (n = 0; n < nfds; ++n) {
      if (events[n].data.fd == listenfd) {
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &socklen);
        if (connfd < 0) {
          mylog(configData.logfile, L_ERR, "socket accept error(%s)!!",
                strerror(errno));
          continue;
        }

        ++acceptCount;

        if (curfds >= (int)configData.block_amount) {
          mylog(configData.logfile, L_WRN,
                "too many connection, more than %d\n", configData.block_amount);

          close(connfd);
          --acceptCount;
          continue;
        }


        if (setnonblocking(connfd) < 0) {
          mylog(configData.logfile, L_ERR, "setnonblocking error(%s)",
                strerror(errno));
          configData.stop = 1;
          break;
        }

        {
          EpollClient *epoll_client = get_a_free_epoll_client();
          if (epoll_client == NULL) {
            mylog(configData.logfile, L_ERR, "can not get a free epoll client");
            configData.stop = 1;
            break;
          }

          gboolean b = g_hash_table_insert(epoll_hash_table, GINT_TO_POINTER(connfd),
                              GINT_TO_POINTER(epoll_client));
          if(b == FALSE){
              mylog(configData.logfile, L_ERR, "g_hash_table_insert failed!");
              configData.stop = 1;
              break;
          }
          epoll_client->fd = connfd;
          epoll_client->free = false;

          memcpy(&epoll_client->cliaddr, &cliaddr, sizeof(cliaddr));
          int result = accept_fun(connfd, &cliaddr, (void**)&epoll_client);
          if (result) {
            configData.stop = 1;
            close(connfd);
            epoll_client->free = true;
            break;
          }

          ev.events = EPOLLIN | EPOLLET;
          ev.data.fd = connfd;
          if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, connfd, &ev) < 0) {
            mylog(configData.logfile, L_ERR,
                  "add socket '%d' to epoll failed: %s", connfd,
                  strerror(errno));
            configData.stop = 1;
            break;
          }
        }

        curfds++;
        continue;
      } else {
        // 处理客户端请求
        gpointer pt = g_hash_table_lookup(epoll_hash_table,
                                          GINT_TO_POINTER(events[n].data.fd));
        if (pt == NULL) {
          mylog(configData.logfile, L_ERR, "EpollClient* epoll_cleint");
          configData.stop = 1;
          break;
        } else {
          if (read_fun(pt) < 0) {
            close(((EpollClient*)pt)->fd);
            epoll_ctl(kdpfd, EPOLL_CTL_DEL, events[n].data.fd, &ev);
            curfds--;
            ((EpollClient*)pt)->free = true;
          }
        }
      }
    }
  }


  //stop listen
  close(listenfd);
  // wait for all pthread
  sleep(10);
  close_epoll_clients();

  g_hash_table_destroy(epoll_hash_table);
  release_client_datas();
  free(clients);
  free(events);
  return 0;
}

static void process_signal(int s) {
  // printf("got signal %d\n", s);
  mylog(configData.logfile, L_INFO, "got signal %d", s);
  configData.stop = 1;
}

static void set_signal() {
  int i = 1;
  for (; i < 32; i++) {
    signal(i, process_signal);
  }
}
