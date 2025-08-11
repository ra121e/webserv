/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgoh <cgoh@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/09 13:04:22 by athonda           #+#    #+#             */
/*   Updated: 2025/08/11 18:17:52 by cgoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main.hpp"
#include <cerrno>
#include <csignal>
#include <exception>
#include <cstring>
#include <sys/epoll.h>
#include <vector>
#include "TmpDirCleaner.hpp"

// Signal-safe run flag
static volatile sig_atomic_t g_run = 1;

extern "C" void handle_sigint_c(int signum)
{
	(void)signum;
	g_run = 0;
}

int	main(int argc, char **argv)
{
	// Ensure tmp/ files are cleaned on exit (normal stop or exceptions)
	TmpDirCleaner cleanupGuard("tmp");
	Config	conf;

	if (argc != 2)
	{
		std::cerr << "Usage: ./webserv [configuration file]\n";
		return 1;
	}
	try
	{
		get_file_config(argv[1], conf);
		conf.setupServers();
		Epoll	epoll(epoll_create1(EPOLL_CLOEXEC));
		for (std::vector<Server*>::const_iterator s_it = conf.getServers().begin(); s_it != conf.getServers().end(); ++s_it)
		{
			for (std::vector<Network*>::const_iterator n_it = (*s_it)->getNetworks().begin(); n_it != (*s_it)->getNetworks().end(); ++n_it)
			{
				epoll.addEventListener(*s_it, (*n_it)->getFd());
			}
		}
		std::signal(SIGINT, handle_sigint_c);
		while (g_run != 0)
		{
			epoll.handleEvents();
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << "\n";
		return 1;
	}
}
