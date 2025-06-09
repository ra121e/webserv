/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: athonda <athonda@student.42singapore.sg    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/09 13:04:22 by athonda           #+#    #+#             */
/*   Updated: 2025/06/09 15:56:43 by athonda          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <iostream>
#include <sys/socket.h> // socket
#include <sys/types.h> // accept()
#include <netinet/in.h> // AF_INET, sockaddr_in type struct
#include <errno.h> // perror
#include <stdio.h> // perror
#include <unistd.h> // close(sock)
#include <stdlib.h> // memset
#include <arpa/inet.h>
#include <string.h> // memset

void	printBanner(std::string const &title)
{
	std::cout << "\n--- " << title << " ---" << std::endl;
}

int	main(void)
{
	int	server_fd;
	int	client_fd;
	struct sockaddr_in	serv_addr;
	struct sockaddr_in	client_addr;
	socklen_t	addrlen = sizeof(client_addr);
	char	buf[1024];
	
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
		perror("socket");
	std::cout << "socket return value is " << server_fd << std::endl;

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(12345);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	bind(server_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	listen(server_fd, 5);

	printf("Waiting for connection...\n");

	client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
	printf("Connected from client\n");

	int n = read(client_fd, buf, sizeof(buf) - 1);
	buf[n] = '\0';
	printf("message from client is %s\n", buf);

	close(client_fd);
	close(server_fd);

//	printBanner("test: write() send message to socket");	
//	write(3, "a", 1);
//	close(sock);
	return (0);
}
