/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: athonda <athonda@student.42singapore.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/09 15:29:12 by athonda           #+#    #+#             */
/*   Updated: 2025/06/09 17:47:34 by athonda          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h> // socket
#include <netinet/in.h> // sockaddr_in, sockaddr
#include <arpa/inet.h> // inet_pton()

int main()
{
	int sockfd;
	struct sockaddr_in serv_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(12345);
	inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

	connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	write(sockfd, "Hello, server!", 15);
	close(sockfd);

	return 0;
}
