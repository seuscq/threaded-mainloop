/*
 * Copyright 2019 Realsil Corp.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>

#include "mainloop.h"

uint8_t buffer[128];

struct thread_data {
	int cfd;
};

void read_cb(int fd, uint32_t events, void *user_data)
{
	int ret;
	if (events & EPOLLIN) {
		ret = read(fd, buffer, 128);
		printf("EPOLLIN: tid %ld recv %d byte\n", pthread_self(), ret);
		if (!ret)
			mainloop_remove_fd(fd);
	}
}

static void timeout_func(int id, void *user_data)
{
	printf("3s eclapsed\n");
	mainloop_modify_timeout(id, 3000);
}

static void *start_routine(void *data)
{
	struct thread_data *thread_data = data;
	
	mainloop_init();
	mainloop_add_fd(thread_data->cfd, EPOLLIN, read_cb, NULL, NULL);
	mainloop_add_timeout(3000, timeout_func, NULL, NULL);
	mainloop_run();
	return NULL;
}

int main()
{
	int lfd, cfd, ret;
	int reuse;
	struct sockaddr_in sockaddr_in;

	lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd < 0) {
		perror("socket error");
		exit(1);
	}

	reuse = 1;
	ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	if (ret < 0) {
		perror("setsockopt error");
		exit(6);
	}


	memset(&sockaddr_in, 0, sizeof(sockaddr_in));
	sockaddr_in.sin_family = AF_INET;
	sockaddr_in.sin_port = htons(6543);
	sockaddr_in.sin_addr.s_addr = htonl(INADDR_ANY);
	ret = bind(lfd, (struct sockaddr*)&sockaddr_in, sizeof(sockaddr_in));
	if (ret < 0) {
		perror("bind error");
		exit(2);
	}

	ret = listen(lfd, 5);
	if (ret < 0) {
		perror("listen error");
		exit(3);
	}
	
	printf("listening...\n");
	while(1) {
		struct sockaddr sockaddr_c;
		socklen_t len = sizeof(sockaddr_c);
		struct thread_data *thread_data;
		pthread_t tid;

		cfd = accept(lfd, &sockaddr_c, &len);
		if (cfd < 0) {
			perror("accept error");
			exit(4);
		}
		printf("connection accepted.\n");

		thread_data = malloc(sizeof(thread_data));
		memset(thread_data, 0, sizeof(thread_data));
		thread_data->cfd = cfd;
		ret = pthread_create(&tid, NULL, start_routine, thread_data);
		if (ret) {
			perror("pthread_create error");
			exit(5);
		}
		printf("starting thread %ld\n", tid);
	}
}
