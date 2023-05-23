#ifndef SOCKET_HPP
#define SOCKET_HPP
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>


#ifndef socket_hpp
#define socket_hpp

#include <stdio.h>

#endif /* socket_hpp */

class Socket{
public:
int ConnectTo(std::string &host, std::string &port){
    int socketfd;
    struct addrinfo hints, *servinfo;
    int rv;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if ((rv = getaddrinfo(host.c_str(), port.c_str(), &hints, &servinfo)) != 0) {
        std::cerr << "Cannot get address information" <<std::endl;
        return EXIT_FAILURE;
    }
    
    socketfd = socket(servinfo->ai_family,
                      servinfo->ai_socktype,
                      servinfo->ai_protocol);
    if (socketfd == -1) {
        std::cerr << "Cannot create socket" << std::endl;
        return EXIT_FAILURE;
    }
    rv = connect(socketfd, servinfo->ai_addr, servinfo->ai_addrlen);
    if(rv == -1){
        std::cerr<<"Cannot connect"<<std::endl;
        return EXIT_FAILURE;
    }
    freeaddrinfo(servinfo);
    return socketfd;
}
int BuildSocket(){
    int proxy_fd, new_fd;
    int rv;
    int yes=1;
    struct addrinfo hints, *servinfo;
    char hostname[1024];
    std::string PORT="8866";
    memset(hostname, 0, sizeof(hostname));
    if (gethostname(hostname, sizeof(hostname)) == -1) {
        std::cerr << "Cannot get hostname" << std::endl;
        exit(EXIT_FAILURE);
    }
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    if ((rv = getaddrinfo(hostname, PORT.c_str(), &hints, &servinfo)) != 0) {
        std::cerr << "Cannot get address information" << std::endl;
        exit(EXIT_FAILURE);
    }
    proxy_fd = socket(servinfo->ai_family,
                          servinfo->ai_socktype,
                          servinfo->ai_protocol);
    if (proxy_fd == -1) {
        std::cerr << "Cannot create socket" << std::endl;
        exit(EXIT_FAILURE);
    }
    rv = setsockopt(proxy_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (rv == -1) {
        std::cerr << "setsockopt" << std::endl;
        exit(EXIT_FAILURE);
    }
    rv = bind(proxy_fd, servinfo->ai_addr, servinfo->ai_addrlen);
    if (rv == -1) {
        close(proxy_fd);
        std::cerr << "Cannot bind" << std::endl;
        exit(EXIT_FAILURE);
    }
    rv= listen(proxy_fd, 120);
    if (rv == -1) {
        std::cerr << "Cannot listen on socket" << std::endl;
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(servinfo);
    return proxy_fd;
}
};
#endif
