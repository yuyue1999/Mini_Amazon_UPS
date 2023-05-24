#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include <thread>
#include <sstream>
#include <random>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <unistd.h>
#include <atomic>
#include <mutex>
#include <memory>
#include <cstdlib>
#include "GPB_message.hpp"
#include "protobuf/world_amazon.pb.h"
#include "protobuf/amazon_ups.pb.h"
#include "common.hpp"
#include "ThreadPool.h"
#include "world_handle.hpp"
#include "UPS_handle.hpp"
#include "frontend_handle.hpp"
#include "test_backend.hpp"

void Init_warehouse(){
    // w_list.push_back(Warehouse(1,1,1));
    // w_list.push_back(Warehouse(2,2,2));
    // w_list.push_back(Warehouse(3,3,3));
    // w_list.push_back(Warehouse(4,4,4));
    // w_list.push_back(Warehouse(5,5,5));
    // w_list.push_back(Warehouse(6,6,6));
    // w_list.push_back(Warehouse(7,7,7));
    // w_list.push_back(Warehouse(8,8,8));
    // w_list.push_back(Warehouse(9,9,9));
    // w_list.push_back(Warehouse(10,10,10));
    for(int i=1;i<=num_of_warehouses;i++){
        w_list.push_back(Warehouse(i,i,i));
    }
}
int connect_to_address(const std::string &address, int port)
{
    struct addrinfo hints, *res;
    int sockfd;

    // Prepare hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // Resolve the domain name
    if (getaddrinfo(address.c_str(), std::to_string(port).c_str(), &hints, &res) != 0)
    {
        std::cerr << "Error: Failed to resolve address" << std::endl;
        return -1;
    }

    // Create the socket
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0)
    {
        std::cerr << "Error: Failed to create socket" << std::endl;
        freeaddrinfo(res);
        return -1;
    }

    // Connect to the server
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0)
    {
        std::cerr << "Error: Failed to connect" << std::endl;
        close(sockfd);
        freeaddrinfo(res);
        return -1;
    }

    // Free the addrinfo structure
    freeaddrinfo(res);

    return sockfd;
}

int Amazon_connect_to_world(int port,int& world_id){
    std::string web_address = "vcm-32242.vm.duke.edu";
    int sock = connect_to_address(web_address, port);
    if (sock != -1)
    {
        std::cout << "Connected to " << web_address << " on port " << port << std::endl;
        // close(sock);
    }
    else
    {
        std::cerr << "Failed to connect to " << web_address << std::endl;
    }

    // return 0;
// }
//     char * to_ip = gethostbyname(world_ip);
//     struct sockaddr_in serv_addr;
//     memset(&serv_addr, 0, sizeof(serv_addr)); 
//     serv_addr.sin_family = AF_INET; 
//     serv_addr.sin_addr.s_addr = inet_addr("vcm-32242.vm.duke.edu"); 
//     serv_addr.sin_port = htons(port); //port for Amazon

//     int sock = socket(AF_INET, SOCK_STREAM, 0);
//     if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))!=0){
//         std::cout<<"Amazon: connect world failed"<<std::endl;
//         return -1;
//     } 

    //create a new world
    AConnect aconnect;
    for(auto &w:w_list){
        auto ainitwarehouse = aconnect.add_initwh();
        ainitwarehouse->set_id(w.id);
        ainitwarehouse->set_x(w.x);
        ainitwarehouse->set_y(w.y);
    }
    aconnect.set_isamazon(true);

    try{
        //send to world
        std::unique_ptr<GPBFileOutputStream> output(new GPBFileOutputStream(sock));
        if(sendMesgTo(aconnect,output.get())!=true){
            std::cout<<"Amazon: send init world failed"<<std::endl;
            return -1;
        }
        //receive from world
        std::unique_ptr<GPBFileInputStream> input(new GPBFileInputStream(sock));
        AConnected aconnected;
        if(recvMesgFrom(aconnected,input.get())!=true){
            std::cout<<"Amazon: receive message form world failed"<<std::endl;
            return -1;
        }
        std::string result = aconnected.result();
        if(result.find("connected!")==std::string::npos){
            std::cout<<"Amazon: connect world failed"<<std::endl;
            std::cout<<"Amazon: "<<result<<std::endl;
            return -1;
        }
        std::cout<<"Amazon: received world id: "<<aconnected.worldid()<<std::endl;
        world_id = aconnected.worldid();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what()<<std::endl;
        return -1;
    }

    return sock;
}

int Amazon_wait_for_UPS(int port,int& world_id){\
    struct sockaddr_in serv_addr;
    int Connect;
    int Server = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in ServerAddr;
    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_port = htons(port);
    ServerAddr.sin_addr.s_addr = INADDR_ANY;
    bind(Server, (sockaddr*)&ServerAddr, sizeof(ServerAddr));
    listen(Server, SOMAXCONN);

    sockaddr_in ClientAddr;
    socklen_t client_len = sizeof(ClientAddr);
    Connect = accept(Server, (sockaddr*)&ClientAddr, &client_len);

    AUInitConnect auinitconnect;
    auinitconnect.set_worldid(world_id);

    std::cout<<"Amazon: UPS world id "<<auinitconnect.worldid()<<std::endl;
    try{
        std::unique_ptr<GPBFileOutputStream> output(new GPBFileOutputStream(Connect));
        if(sendMesgTo(auinitconnect,output.get())!=true){
            std::cout<<"Amazon: send connection to UPS failed"<<std::endl;
            return -1;
        }

        UAConfirmConnected uaconfrimconnected;
        std::unique_ptr<GPBFileInputStream> input(new GPBFileInputStream(Connect));
        if(recvMesgFrom(uaconfrimconnected,input.get())!=true){
            std::cout<<"Amazon: receive from UPS failed"<<std::endl;
            return -1;
        }
        if(uaconfrimconnected.connected()==false){
            std::cout<<"Amazon: UPS connected to world failed"<<std::endl;
            return -1;
        }

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what()<<std::endl;
        return -1;
    }
    

    return Connect;
}

#endif