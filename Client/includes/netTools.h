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
#include <string>

// host: name of the host to resolve
// service: name of the service to resolve
int connectTCP(const std::string host, const std::string service);

// host: name of the host to resolve
// service: name of the service to resolve
int connectUDP(const std::string host, const std::string service);

// host: name of the host to resolve
// service: name of the service to resolve
// transport: "tcp" or "udp"
int connectsock(const std::string host, const std::string service, const std::string transport);

// service: name of the service to resolve
int listenTCP(const std::string service, int maxListen);

// service: name of the service to resolve
int listenUDP(const std::string service, int maxListen);

// service: name of the service to resolve
// transport: "tcp" or "udp"
int listensock(const std::string service, const std::string transport, int maxListen);

// extern int errno;
int errexit(const char *format, ...);

#endif