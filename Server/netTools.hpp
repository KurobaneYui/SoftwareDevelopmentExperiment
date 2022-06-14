#ifndef __NETTOOLS_HPP__
#define __NETTOOLS_HPP__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cerrno>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <cstdio>
#include <unistd.h>

// host: name of the host to resolve
// service: name of the service to resolve
int connectTCP(const char *host, const char *service);

// host: name of the host to resolve
// service: name of the service to resolve
int connectUDP(const char *host, const char *service);

// host: name of the host to resolve
// service: name of the service to resolve
// transport: "tcp" or "udp"
int connectsock(const char *host, const char *service, char *transport);

// service: name of the service to resolve
int listenTCP(const char *service, int maxListen);

// service: name of the service to resolve
int listenUDP(const char *service, int maxListen);

// service: name of the service to resolve
// transport: "tcp" or "udp"
int listensock(const char *service, const char *transport, int maxListen);

//extern int errno;
int errexit(const char *format, ...);

#endif