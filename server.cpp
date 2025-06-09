/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: athonda <athonda@student.42singapore.sg    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/09 13:04:22 by athonda           #+#    #+#             */
/*   Updated: 2025/06/09 13:55:38 by athonda          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <iostream>
#include <sys/socket.h> // socket
#include <netinet/in.h> // AF_INET, sockaddr_in
#include <errno.h> // perror
#include <stdio.h> // perror
#include <unistd.h> // close(sock)

void	printBanner(std::string const &title)
{
	std::cout << "\n--- " << title << " ---" << std::endl;
}

int	main(void)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		perror("socket");
	std::cout << "socket return value is " << sock << std::endl;

	printBanner("test: write() send message to socket");	
	write(3, "a", 1);
	close(sock);
	return (0);
}
