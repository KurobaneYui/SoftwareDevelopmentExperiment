#include "netTools.hpp"

int connectTCP(const char *host, const char *service)
{
    return connectsock(host, service, "tcp");
}

int connectUDP(const char *host, const char *service)
{
    return connectsock(host, service, "udp");
}

int connectsock(const char *host, const char *service, char *transport)
{
    int s;        // socket
    int type;     // socket type
    int protocol; // protocol

    struct hostent *hp;  // host information
    struct servent *sp;  // service information
    struct protoent *pp; // protocol information

    struct sockaddr_in sin;       // an Internet endpoint address
    memset(&sin, 0, sizeof(sin)); // clear the structure

    sin.sin_family = AF_INET; // address family

    // Map host name to IP address, allowing for dotted decimal
    if (hp = gethostbyname(host)) // lookup host's IP address
    {
        memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);
    }
    else if ((sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
    {
        errexit("Can't get \"%s\" host entry\n", host);
    }

    // Map service name to port number
    if (sp = getservbyname(service, transport))
    {
        sin.sin_port = sp->s_port;
    }
    else if ((sin.sin_port = htons((unsigned short int)atoi(service))) == 0)
    {
        errexit("Can't get \"%s\" service entry\n", service);
    }

    // Map transport protocol name to protocol number
    if ((pp = getprotobyname(transport)) == 0)
    {
        errexit("Can't get \"%s\" protocol entry\n", transport);
    }
    protocol = pp->p_proto;
    if (strcmp(transport, "tcp") == 0)
    {
        type = SOCK_STREAM;
    }
    else if (strcmp(transport, "udp") == 0)
    {
        type = SOCK_DGRAM;
    }
    else
    {
        errexit("Bad protocol \"%s\"\n", transport);
    }

    // Allocate a socket
    s = socket(PF_INET, type, protocol);
    if (s < 0)
    {
        errexit("Can't create socket: %s\n", strerror(errno));
    }
    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        errexit("Connect to %s.%s failed: %s\n", host, service, strerror(errno));
    }
    
    return s;
}

int listenTCP(const char *service, int maxListen)
{
    return listensock(service, "tcp", maxListen);
}

int listenUDP(const char *service, int maxListen)
{
    return listensock(service, "udp", maxListen);
}

int listensock(const char *service, const char *transport, int maxListen)
{
    int s;        // socket
    int type;     // socket type
    int protocol; // protocol

    struct servent *sp;  // service information
    struct protoent *pp; // protocol information

    struct sockaddr_in sin;       // an Internet endpoint address
    memset(&sin, 0, sizeof(sin)); // clear the structure

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;

    if((pp = getprotobyname(transport)) == 0)
    {
        errexit("Can't get \"%s\" protocol entry\n", transport);
    }
    protocol = pp->p_proto;

    if(sp = getservbyname(service, transport))
    {
        sin.sin_port = sp->s_port;
    }
    else if((sin.sin_port = htons((unsigned short int)atoi(service)))==0)
    {
        errexit("Can't get \"%s\" service entry\n", service);
    }

    if(strcmp(transport,"tcp")==0)
    {
        type = SOCK_STREAM;
    }
    else if(strcmp(transport,"udp")==0)
    {
        type = SOCK_DGRAM;
    }
    else
    {
        errexit("Bad protocol \"%s\"\n", transport);
    }

    if((s = socket(PF_INET, type, protocol))<0)
    {
        errexit("Can't create socket: %s\n", strerror(errno));
    }
    if(bind(s, (struct sockaddr *)&sin, sizeof(sin))<0)
    {
        errexit("Can't bind to %s.%s: %s\n", inet_ntoa(sin.sin_addr), service, strerror(errno));
    }
    if(type==SOCK_STREAM && listen(s, maxListen)<0)
    {
        errexit("Can't listen on %s.%s: %s\n", inet_ntoa(sin.sin_addr), service, strerror(errno));
    }

    return s;
}

int errexit(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(1);
}