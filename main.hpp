#ifndef MAIN_HPP
#define MAIN_HPP
#include <poll.h>
#include <sys/socket.h> // socket
#include <sys/types.h> // accept()
#include <netinet/in.h> // AF_INET, sockaddr_in type struct, INADDR_ANY
// #include <errno.h> // perror
// #include <stdio.h> // perror
#include <unistd.h> // close(sock)
#include <stdlib.h> // memset
#include <arpa/inet.h>
#include <sys/epoll.h> // epoll
#include <fcntl.h> //fcntl, O_NONBLOCK
// #include <cstddef>
#include "Config.hpp"

void	get_file_config(const char *filename, Config& conf);

#endif
