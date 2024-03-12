#include "sockets.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>


#ifndef _WIN32
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
typedef int socklen_t;
#endif

/**
 * @brief Issues an error message and aborts the program.
 *
 * @param s Error message.
 */
#define panic(s)   \
	do {           \
		perror(s); \
		exit(-1);  \
	} while (0);

/**
 * @brief By using the server parameters provided in @p ws_srv,
 * create a socket and bind it accordingly with the server
 * configurations.
 *
 * @param ws_srv Web Socket configurations.
 *
 * @return Returns the socket file descriptor.
 */
int do_bind_socket(char const *host, uint16_t port)
{
	struct addrinfo hints, *results, *try;
	char service[8] = {0};
	int reuse;
	int sock;

	reuse = 1;

	/* Prepare the getaddrinfo structure. */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	/* Port. */
	snprintf(service, sizeof(service) - 1, "%d", port);

	if (getaddrinfo(host, service, &hints, &results) != 0)
		panic("getaddrinfo() failed");

	/* Try to create a socket with one of the returned addresses. */
	for (try = results; try != NULL; try = try->ai_next) {
		/* try to make a socket with this setup */
		if ((sock = socket(try->ai_family, try->ai_socktype,
		     try->ai_protocol)) < 0) {
			continue;
		}

		/* Reuse previous address. */
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse,
		    sizeof(reuse)) < 0) {
			panic("setsockopt(SO_REUSEADDR) failed");
		}

		/* Bind. */
		if (bind(sock, try->ai_addr, try->ai_addrlen) < 0)
			panic("Bind failed");

		/* if it worked, we're done. */
		break;
	}

	freeaddrinfo(results);

	/* Check if binded with success. */
	if (try == NULL)
		panic("couldn't find a port to bind to");

	return (sock);
}

int net_accept(int sock, int timeout_ms)
{
	struct timeval time;        /* Client socket timeout. */
	struct sockaddr_storage sa; /* Client.                */
	socklen_t salen;            /* Length of sockaddr.    */
	int new_sock;               /* New opened connection. */
	/* Accept. */
	new_sock = accept(sock, (struct sockaddr *)&sa, &salen);
	if (new_sock < 0)
		panic("Error on accepting connections..");

	if (timeout_ms) {
		time.tv_sec = timeout_ms / 1000;
		time.tv_usec = (timeout_ms % 1000) * 1000;

		/*
		 * Socket timeout
		 * This feature seems to be supported on Linux, Windows,
		 * macOS and FreeBSD.
		 *
		 * See:
		 *   https://linux.die.net/man/3/setsockopt
		 */
		setsockopt(new_sock, SOL_SOCKET, SO_SNDTIMEO, &time, sizeof(struct timeval));
	}
	return new_sock;
}

/**
 * @brief Sets the IP address relative to a client connection opened
 * by the server and save inside the client structure.
 *
 * @param client Client connection.
 */
void net_get_address(int sock, char ip[1025], char port[32])
{
	struct sockaddr_storage addr;
	socklen_t hlen = sizeof(addr);

	//if (!CLIENT_VALID(client))
		//return;

	memset(ip, 0, 1025);
	memset(port, 0, 32);

	if (getpeername(sock, (struct sockaddr *)&addr, &hlen) < 0)
		return;

	getnameinfo((struct sockaddr *)&addr, hlen, ip, 1025, port, 32, NI_NUMERICHOST | NI_NUMERICSERV);
}


/**
 * @brief Shutdown and close a given socket.
 *
 * @param fd Socket file descriptor to be closed.
 *
 * @attention This is part of the internal API and is documented just
 * for completeness.
 */
void net_close_socket(int fd)
{
#ifndef _WIN32
	shutdown(fd, SHUT_RDWR);
	close(fd);
#else
	closesocket(fd);
#endif
}