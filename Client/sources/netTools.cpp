#include "netTools.h"

int connectTCP(const std::string host, const std::string service)
{
    return connectsock(host, service, "tcp");
}

int connectUDP(const std::string host, const std::string service)
{
    return connectsock(host, service, "udp");
}

int connectsock(const std::string host, const std::string service, const std::string transport)
{
    int s{};        // socket
    int type{};     // socket type
    int protocol{}; // protocol

    hostent *hp{};  // host information
    servent *sp{};  // service information
    protoent *pp{}; // protocol information

    sockaddr_in sin{};            // an Internet endpoint address
    memset(&sin, 0, sizeof(sin)); // clear the structure

    sin.sin_family = AF_INET; // address family

    // Map host name to IP address, allowing for dotted decimal
    if (hp = gethostbyname(host.c_str())) // lookup host's IP address
    {
        memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);
    }
    else if ((sin.sin_addr.s_addr = inet_addr(host.c_str())) == INADDR_NONE)
    {
        errexit("Can't get \"%s\" host entry\n", host.c_str());
    }

    // Map service name to port number
    if (sp = getservbyname(service.c_str(), transport.c_str()))
    {
        sin.sin_port = sp->s_port;
    }
    else if ((sin.sin_port = htons((unsigned short int)atoi(service.c_str()))) == 0)
    {
        errexit("Can't get \"%s\" service entry\n", service.c_str());
    }

    // Map transport protocol name to protocol number
    if ((pp = getprotobyname(transport.c_str())) == 0)
    {
        errexit("Can't get \"%s\" protocol entry\n", transport.c_str());
    }
    protocol = pp->p_proto;
    if (strcmp(transport.c_str(), "tcp") == 0)
    {
        type = SOCK_STREAM;
    }
    else if (strcmp(transport.c_str(), "udp") == 0)
    {
        type = SOCK_DGRAM;
    }
    else
    {
        errexit("Bad protocol \"%s\"\n", transport.c_str());
    }

    // Allocate a socket
    s = socket(PF_INET, type, protocol);
    if (s < 0)
    {
        errexit("Can't create socket: %s\n", strerror(errno));
    }
    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        errexit("Connect to %s.%s failed: %s\n", host.c_str(), service.c_str(), strerror(errno));
    }

    return s;
}

int listenTCP(const std::string service, int maxListen)
{
    return listensock(service, "tcp", maxListen);
}

int listenUDP(const std::string service, int maxListen)
{
    return listensock(service, "udp", maxListen);
}

int listensock(const std::string service, const std::string transport, int maxListen)
{
    int s{};        // socket
    int type{};     // socket type
    int protocol{}; // protocol

    servent *sp{};  // service information
    protoent *pp{}; // protocol information

    sockaddr_in sin{};            // an Internet endpoint address
    memset(&sin, 0, sizeof(sin)); // clear the structure

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;

    // Map service name to port number
    if ((pp = getprotobyname(transport.c_str())) == 0)
    {
        errexit("Can't get \"%s\" protocol entry\n", transport.c_str());
    }
    // get port number
    protocol = pp->p_proto;

    if (sp = getservbyname(service.c_str(), transport.c_str()))
    {
        sin.sin_port = sp->s_port;
    }
    // get IP address
    else if ((sin.sin_port = htons((unsigned short int)atoi(service.c_str()))) == 0)
    {
        errexit("Can't get \"%s\" service entry\n", service.c_str());
    }

    if (strcmp(transport.c_str(), "tcp") == 0)
    {
        type = SOCK_STREAM;
    }
    else if (strcmp(transport.c_str(), "udp") == 0)
    {
        type = SOCK_DGRAM;
    }
    else
    {
        errexit("Bad protocol \"%s\"\n", transport.c_str());
    }

    if ((s = socket(PF_INET, type, protocol)) < 0)
    {
        errexit("Can't create socket: %s\n", strerror(errno));
    }
    if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        errexit("Can't bind to %s.%s: %s\n", inet_ntoa(sin.sin_addr), service.c_str(), strerror(errno));
    }
    if (type == SOCK_STREAM && listen(s, maxListen) < 0)
    {
        errexit("Can't listen on %s.%s: %s\n", inet_ntoa(sin.sin_addr), service.c_str(), strerror(errno));
    }

    return s;
}

int errexit(const char *format, ...)
{
    va_list args{};
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(1);
}