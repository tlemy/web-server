#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>

#define PORT 3000
#define BACKLOG 16
#define LEN_SOCK sizeof(struct sockaddr)
#define LEN_SOCK_INET sizeof(struct sockaddr_in)
#define LEN_SOCK_INET6 sizeof(struct sockaddr_in)
#define MAX_LEN_PEER_NAME 64
#define MAX_LEN_REQUEST 4096

int setup(void);
void run(int serverFd);
void quit(int serverFd);

int main(void)
{
    int sockFd {setup()};

    run(sockFd);
    quit(sockFd);
}

int setup(void)
{
    int sockFd {socket(PF_INET, SOCK_STREAM, 0)};

    if (0 > sockFd)
    {
        perror("socket");
        exit(EXIT_FAILURE);
        return -1;
    }

    const int enable = 1;
    if (0 > setsockopt(sockFd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)))
    {
        perror("socket");
        quit(sockFd);
    }

    struct sockaddr_in addrInet {};
    addrInet.sin_family = AF_INET; // host byte order
    addrInet.sin_addr.s_addr = htonl(INADDR_ANY); // network byte order
    addrInet.sin_port = htons(PORT); // network byte order

    struct sockaddr *addr {(struct sockaddr*) &addrInet};
    socklen_t len = LEN_SOCK;

    if (0 > bind(sockFd, addr, LEN_SOCK_INET))
    {
        perror("bind");
        quit(sockFd);
    }

    if (0 > getsockname(sockFd, addr, &len))
    {
        perror("getsockname");
        quit(sockFd);
    }

    // inet_ntoa converts the address from binary to numbers and dots
    std::cout << "server: " << inet_ntoa(addrInet.sin_addr) << ":" << ntohs(addrInet.sin_port) << '\n';

    return sockFd;
}

void run(int serverFd)
{
    if (0 > listen(serverFd, BACKLOG))
    {
        perror("listen");
        quit(serverFd);
    }

    int numAcceptedReqs = 0;

    while (numAcceptedReqs < 1)
    {
        int clientFd {};
        struct sockaddr conn {};
        socklen_t len {LEN_SOCK};

        if (0 > (clientFd = accept(serverFd, &conn, &len)))
        {
            perror("accept");
        }

        if (conn.sa_family == AF_INET)
        {
            struct sockaddr_in connInet {};
            socklen_t lenInet = LEN_SOCK_INET;

            if (0 > getpeername(clientFd, (struct sockaddr*) &connInet, &lenInet))
            {
                perror("getpeername");
            }

            numAcceptedReqs += 1;
            std::cout << "client: " << inet_ntoa(connInet.sin_addr) << ":" << ntohs(connInet.sin_port) << '\n';
        }
        else if (conn.sa_family == AF_INET6)
        {
            struct sockaddr_in6 connInet6 {};
            socklen_t lenInet = LEN_SOCK_INET;

            if (0 > getpeername(clientFd, (struct sockaddr*) &connInet6, &lenInet))
            {
                perror("getpeername");
            }

            socklen_t lenBuff {sizeof(char) * MAX_LEN_PEER_NAME};
            char buff[sizeof(char) * MAX_LEN_PEER_NAME] {};
            numAcceptedReqs += 1;
            std::cout << "client: " << inet_ntop(AF_INET6, (void*) &connInet6.sin6_addr, buff, lenBuff) << ":" << ntohs(connInet6.sin6_port) << '\n';
        }

        size_t lenBuff {sizeof(char) * MAX_LEN_REQUEST};
        char buff[sizeof(char) * MAX_LEN_REQUEST] {};

        if (0 > recv(clientFd, &buff, lenBuff, 0))
        {
            perror("recv");
        }

        std::cout << buff << '\n';

        if (0 > close(clientFd))
        {
            perror("close");
        }
    }
}

void quit(int serverFd)
{
    if (0 > close(serverFd))
    {
        perror("close");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}