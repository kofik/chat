#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __linux__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include "debugutil.h"
#elif _WIN32 /* {{{ */
#include <winsock2.h>
#endif /* }}} */

#ifdef __linux__
#define DEFAULT_PORT	49152
#elif _WIN32 /* {{{ */
#define DEFAULT_PORT	5001
#endif /* }}} */
#define NBACKLOG	5

#define RECV_BUFSIZE    8192

#ifdef _WIN32 /* {{{ */
#define ERR_BUFSIZE     128
#endif /* }}} */


int 
main()
{
#ifdef __linux__
	int sock, s;     /* sock: :コネクション受け付け用のファイルディスクリプタ */
	                 /*    s: :メッセージ送受信用のファイルディスクリプタ */
#elif _WIN32 /* {{{ */
	WSADATA wsaData;
	SOCKET sock, s;
#endif /* }}} */
	struct sockaddr_in addr;         /* server's ipv4 addr and app port */
	unsigned int addr_len;           /* size of `addr'; use by accept() */
	u_short port = DEFAULT_PORT;     /* listen port */
	int backlog = NBACKLOG;

	char recv_buf[RECV_BUFSIZE];
	int errsv = 0;
#ifdef _WIN32 /* {{{ */
	char err_buf[ERR_BUFSIZE];
#endif /* }}} */



#ifdef __linux__
#elif _WIN32 /* {{{ */
	WSAStartup(MAKEWORD(2, 0), &wsaData);
#endif /* }}} */

#ifdef __linux__
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		errsv = errno;
		strerror(errsv);
		return EXIT_FAILURE;
	}
#elif _WIN32 /* {{{ */
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		errsv = errno;
		strerror_s(err_buf, ERR_BUFSIZE, errsv);
		return EXIT_FAILURE;
	}
#endif /* }}} */

	memset(&addr, 0x0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;      /* UDP, TCP, etc. (WinSock.h) */
	addr.sin_port = htons(port);    /* Conversion host to network byte order */
#ifdef __linux__
	// addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
#elif _WIN32 /* {{{ */
	// addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addr.sin_addr.S_un.S_addr = htonl(INADDR_LOOPBACK);
#endif /* }}} */

#ifdef __linux__
	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
dbgerr("*");
		errsv = errno;
		strerror(errsv);
		return EXIT_FAILURE;
	}
#elif _WIN32 /* {{{ */
	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
		errsv = errno;
		strerror_s(err_buf, ERR_BUFSIZE, errsv);
		return EXIT_FAILURE;
	}
#endif /* }}} */

#ifdef __linux__
	if (listen(sock, backlog) == -1) {
		errsv = errno;
		strerror(errsv);
		return EXIT_FAILURE;
	}
#elif _WIN32 /* {{{ */
	if (listen(sock, backlog) == SOCKET_ERROR) {
		errsv = errno;
		strerror_s(err_buf, ERR_BUFSIZE, errsv);
		return EXIT_FAILURE;
	}
#endif /* }}} */

#ifdef __linux__
	addr_len = sizeof(addr);
	/* accept､ﾎﾊﾖ､・ﾍ､ﾏ･皈ﾃ･ｻ｡ｼ･ｸｼｮﾍﾑ､ﾎ･ﾕ･｡･､･・ﾇ･｣･ｹ･ｯ･・ﾗ･ｿ､鬢ｷ､､ */
	if ((s = accept(sock, (struct sockaddr *)&addr, &addr_len)) == -1) {
		errsv = errno;
		strerror(errsv);
		return EXIT_FAILURE;
	}
#elif _WIN32 /* {{{ */
	addr_len = sizeof(addr);
	if ((s = accept(sock, (struct sockaddr *)&addr, &addr_len))
		                                                  == INVALID_SOCKET) {
		errsv = errno;
		strerror_s(err_buf, ERR_BUFSIZE, errsv);
		return EXIT_FAILURE;
	}
#endif /* }}} */

	printf("Connected from '%s'\n", inet_ntoa(addr.sin_addr));

	while (1) {
		int i;
#ifdef _WIN32
		int recv_retval;
#endif
		recv_buf[0] = '\0';
		for (i=0; i<RECV_BUFSIZE-1; ++i) {
#ifdef __linux__
			if (recv(s, &recv_buf[i], 1, 0) <= 0) {
				errsv = errno;
				strerror(errsv);
				// NOTE: need to make function or to use `goto' ?
				// break;
				return EXIT_FAILURE;
			}
#elif _WIN32
			if ((recv_retval = recv(s, &recv_buf[i], 1, 0)) == SOCKET_ERROR
				 || recv_retval == 0) {
				errsv = errno;
				strerror_s(err_buf, ERR_BUFSIZE, errsv);
				return EXIT_FAILURE;
			}
#endif
			if (recv_buf[i] == '\n') {
				break;
			}
		}
		recv_buf[i] = '\0';
		printf("%s\n", recv_buf);
		fflush(stdout);
	}

#ifdef __linux__
	if (close(sock) == -1) {
		fprintf(stderr, "failed to closesocket()\n");
		return EXIT_FAILURE;
	}
#elif _WIN32 /* {{{ */
	if (closesocket(sock) == SOCKET_ERROR) {
		fprintf(stderr, "failed to closesocket()\n");
		return EXIT_FAILURE;
	}
#endif /* }}} */

#ifdef __linux__
#elif _WIN32 /* {{{ */
	WSACleanup();
#endif /* }}} */

	return EXIT_SUCCESS;
}

/* vim: set sw=4 ts=4 foldmethod=marker: */
