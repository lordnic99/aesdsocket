#include <arpa/inet.h>
#include <bits/pthreadtypes.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <sys/types.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

struct client_thread {
  pthread_t client_thread;
  TAILQ_ENTRY(client_thread) next;
};

struct sock_thread_args {
  int *client_fd;
  char *ip;
};
/********************************/
TAILQ_HEAD(head_s, client_thread) head;
const char *file_path = "/var/tmp/aesdsocketdata";
int write_fd__ = 0;
int read_fd__ = 0;
int server_fd = 0;
pthread_mutex_t write_mutex;
pid_t daemon__;
pid_t time_daemon;
/********************************/
int socket_init() {
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    perror("socket");
    exit(-1);
  }
  return socket_fd;
}

void socket_bind(int socket_fd, const uint16_t port) {
  struct sockaddr_in server;
  memset(&server, 0, sizeof(server));
  server.sin_port = htons(port);
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  int bind_status = bind(socket_fd, (struct sockaddr *)&server, sizeof(server));
  if (bind_status < 0) {
    perror("bind");
    exit(-1);
  }
}

void write_file(const char *__string, char **return__) {
  pthread_mutex_lock(&write_mutex);
  write(write_fd__, __string, strlen(__string));
  lseek(read_fd__, 0, SEEK_SET);
  int size = lseek(read_fd__, 0, SEEK_END);
  lseek(read_fd__, 0, SEEK_SET);
  read(read_fd__, *return__, size);
  pthread_mutex_unlock(&write_mutex);
}

void signal_handler(int signo) {
  unlink(file_path);
  syslog(LOG_INFO, "%s", "Caught signal, exiting!");
  struct client_thread *__client = NULL;
  TAILQ_FOREACH(__client, &head, next) {
    pthread_join(__client->client_thread, NULL);
  }
  while (!TAILQ_EMPTY(&head)) {
    __client = TAILQ_FIRST(&head);
    TAILQ_REMOVE(&head, __client, next);
    free(__client);
    __client = NULL;
  }
  close(read_fd__);
  close(write_fd__);
  close(server_fd);
  exit(0);
}

void sock_re_use(int sock_fd) {
  int opt = 1;
  setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  return;
}

void *client_handler(void *args) {
  struct sock_thread_args *args__ = (struct sock_thread_args *)args;
  int *fd__ = args__->client_fd;
  char *ip = args__->ip;
  syslog(LOG_INFO, "%s%s", "Accepted connection from ", ip);
  char *buffer = malloc(30000);
  char *buffer_rec_ = malloc(30000);
  memset(buffer, 0, 30000);
  memset(buffer_rec_, 0, 30000);
  read(*fd__, buffer, 30000);
  write_file(buffer, &buffer_rec_);
  send(*fd__, buffer_rec_, strlen(buffer_rec_), 0);
  syslog(LOG_INFO, "%s%s", "Closed connection from ", ip);
  close(*fd__);
  free(fd__);
  free(ip);
  free(args__);
  free(buffer_rec_);
  free(buffer);
}

int main(int argc, char *argv[]) {
  write_fd__ = open(file_path, O_WRONLY | O_CREAT | O_EXCL, 0666);
  if (write_fd__ < 0) {
    perror("write file");
  }
  read_fd__ = open(file_path, O_RDONLY);
  if (read_fd__ < 0) {
    perror("read file");
  }
  /* Signal handler */
  signal(SIGTERM, signal_handler);
  signal(SIGINT, signal_handler);
  TAILQ_INIT(&head);
  server_fd = socket_init();
  sock_re_use(server_fd);
  socket_bind(server_fd, 9000);
  listen(server_fd, 50);
  daemon__ = fork();
  if (daemon__ < 0) {
    printf("ERROR: Failed to start in daemon mode!\n");
  } else if (daemon__ == 0) {
    while (1) {
      struct sockaddr_in client;
      socklen_t lens = sizeof(client);
      int client_fd = accept(server_fd, (struct sockaddr *)&client, &lens);
      char *__ip = (char *)malloc(INET_ADDRSTRLEN);
      inet_ntop(AF_INET, &client.sin_addr, __ip, INET_ADDRSTRLEN);
      int *client__ = (int *)malloc(sizeof(int));
      *client__ = client_fd;
      pthread_t thread_connection;
      struct client_thread *__client_ =
          (struct client_thread *)malloc(sizeof(*__client_));
      __client_->client_thread = thread_connection;
      TAILQ_INSERT_TAIL(&head, __client_, next);
      struct sock_thread_args *args =
          (struct sock_thread_args *)malloc(sizeof(struct sock_thread_args));
      args->client_fd = client__;
      args->ip = __ip;
      pthread_create(&thread_connection, NULL, (void *)client_handler,
                     (void *)args);
      // int fd = client_fd;
      // char buffer[1024];
      // memset(buffer, 0, sizeof(buffer));
      // read(fd, buffer, sizeof(buffer));
      // printf("Receive: %s", buffer);
      // send(fd, buffer, strlen(buffer), 0);
      // close(fd);
      // pthread_t lala;
      // each_client = malloc(sizeof(struct client_thread));
      // each_client->client_thread = lala;
      // int *client_fd__ = malloc(sizeof(int));
      // *client_fd__ = client_fd;
      // pthread_create(&lala, NULL,(void *) &client_handler, (int
      // *)client_fd__); pthread_join(lala, NULL); TAILQ_INSERT_TAIL(&head,
      // each_client, next);
    }
  }
  time_daemon = fork();
  if (time_daemon < 0) {
    perror("timer");
  } else if (time_daemon == 0) {
    /***********************************/
    time_t rawtime;
    struct tm *info;
    /***********************************/
    char time_buffer__[100];
    char buffer__[120];
    while (1) {
      sleep(10);
      time(&rawtime);
      info = localtime(&rawtime);
      memset(buffer__, 0, 120);
      memset(time_buffer__, 0, 100);
      strftime(time_buffer__, 100, "%a, %d %b %Y %T %z", info);
      sprintf(buffer__, "%s%s", "timestamp:", time_buffer__);
      strcat(buffer__, "\n");
      write(write_fd__, buffer__, strlen(buffer__));
    }
  }
  return 0;
}
