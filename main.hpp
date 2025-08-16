#ifndef MAIN_HPP
#define MAIN_HPP
#include <poll.h>
#include <sys/socket.h> // socket
#include <sys/types.h> // accept()
#include <netinet/in.h> // AF_INET, sockaddr_in type struct, INADDR_ANY
#include <unistd.h> // close(sock)
#include <stdlib.h> // memset
#include <arpa/inet.h>
#include <sys/epoll.h> // epoll
#include <fcntl.h> //fcntl, O_NONBLOCK

extern "C" void handle_sigint_c(int signum);

#endif
