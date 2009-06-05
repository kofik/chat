#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __linux__
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#elif _WIN32 /* {{{ */
#include <winsock2.h>
#endif /* }}} */

#ifdef __linux__
#define DEFAULT_PORT	49152
#elif _WIN32 /* {{{ */
#define DEFAULT_PORT	5001
#endif /* }}} */

#define MESSAGE_BUFSIZE 8192
// #define IPADDR_SIZE		33
/* IPADDR_SIZE is allocated the size of ipv4 addr and '\0', shortly 33 bytes. */
#ifdef __linux__
#define IPADDR_SIZE     ( (0x1<<sizeof(in_addr_t)) + 1 )
#elif _WIN32
#define IPADDR_SIZE     ( (0x1<<sizeof(u_long)) + 1 )
#endif
#ifdef _WIN32 /* {{{ */
/* ERR_BUFSIZE: 
 * 一般的にバッファサイズはいくつとるのだろうか？
 * エラー番号11のEAGAINの71文字が一番多いと思う
 */
#define ERR_BUFSIZE     128
#endif /* }}} */


/* arguments order on command-line */
enum {
	CMD_NAME = 0,       /* command name */
	DST_IP,             /* server's IP */
	// DST_PORT,        /* XXX: may set port on command-line */
};



int 
main(int argc, char *argv[])
{
#ifdef __linux__
	int sock;
#elif _WIN32 /* {{{ */
	WSADATA wsaData;
	SOCKET sock;
#endif /* }}} */
	struct sockaddr_in addr;         /* server's ipv4 addr and app port */
#ifdef __linux__
	in_addr_t dst_ip;                /* server's addr(temporary) */
#elif _WIN32
	u_long dst_ip;
#endif
	char ipaddr[IPADDR_SIZE];        /* server's addr dotted decimal notation */
	u_short port = DEFAULT_PORT;     /* server's listening port */

	char send_buf[MESSAGE_BUFSIZE];  /* message buffer */
	int errsv = 0;
#ifdef _WIN32 /* {{{ */
	char err_buf[ERR_BUFSIZE];
#endif /* }}} */



#ifdef _WIN32 /* {{{ */
	WSAStartup(MAKEWORD(2, 0), &wsaData);
#endif /* }}} */

	/* convert ip addr dotted notation to binary */
	if ((dst_ip = inet_addr(argv[DST_IP])) == INADDR_NONE) {
		fprintf(stderr, "invalid address: %s\n", argv[DST_IP]);
		return EXIT_FAILURE;
	}

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
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
#ifdef __linux__
	addr.sin_addr.s_addr = dst_ip;
#elif _WIN32 /* {{{ */
	addr.sin_addr.S_un.S_addr = dst_ip;
#endif /* }}} */

#ifdef __linux__
	if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		errsv = errno;
		strerror(errsv);
		return EXIT_FAILURE;
	}
#elif _WIN32 /* {{{ */
	if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
		errsv = errno;
		strerror_s(err_buf, ERR_BUFSIZE, errsv);
		return EXIT_FAILURE;
	}
#endif /* }}} */


#ifdef __linux__
	snprintf(ipaddr, sizeof(ipaddr) - 1, "%s", inet_ntoa(addr.sin_addr));
#elif _WIN32
	sprintf_s(ipaddr, sizeof(ipaddr), "%s", inet_ntoa(addr.sin_addr));
#endif
	printf("Connected to '%s'\n", ipaddr);

	while (1) {
		printf("%s> ", ipaddr);
		fgets(send_buf, MESSAGE_BUFSIZE, stdin);

#ifdef __linux__
		if (send(sock, (char *)send_buf, strlen(send_buf), 0) == -1) {
			errsv = errno;
			strerror(errsv);
			break;
		}
#elif _WIN32
		if (send(sock, (char *)send_buf, strlen(send_buf), 0) == SOCKET_ERROR) {
			errsv = errno;
			strerror_s(err_buf, ERR_BUFSIZE, errsv);
			// WSAGetLastErrorの方が詳しいエラー(ifの外で確認した方がいいかな？)
			break;
		}
#endif
	}

#ifdef __linux__
	if (close(sock) == -1) {
		errsv = errno;
		strerror(errsv);
		return EXIT_FAILURE;
	}
#elif _WIN32 /* {{{ */
	if (closesocket(sock) == SOCKET_ERROR) {
		errsv = errno;
		strerror_s(err_buf, ERR_BUFSIZE, errsv);
		return EXIT_FAILURE;
	}
#endif /* }}} */


#ifdef _WIN32 /* {{{ */
	WSACleanup();
#endif /* }}} */

	return EXIT_SUCCESS;
}

/* vim: set sw=4 ts=4 foldmethod=marker: */
