/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: athonda <athonda@student.42singapore.sg    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/09 15:29:12 by athonda           #+#    #+#             */
/*   Updated: 2025/06/10 11:32:06 by athonda          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <vector>
#include <string>
#include <iostream>
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

	std::cout << "type message here. if you want to finish, type EOM." << std::endl;
	std::string	input;
	while (std::getline(std::cin, input))
	{
		std::vector<char> str(input.begin(), input.end());
		write(sockfd, &str[0], str.size());
		if (input == "EOM")
			break ;
	}
	close(sockfd);
	return 0;
}
