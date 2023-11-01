#include"ThreadPool.h"
#include"server.hpp"
#include<sys/socket.h>
#include <unistd.h> 
#include<arpa/inet.h> 
#include<thread>
#include<cstdlib>
#include<cstring>
#include<iostream>
#include<thread>
#include <pqxx/pqxx>



int main(int argc, char *argv[]) {

    Init_warehouse();
    memset(&send_acks,0,sizeof(send_acks));
    memset(&recv_ups_acks,0,sizeof(recv_ups_acks));
    memset(&recv_world_acks,0,sizeof(recv_world_acks));

    int world_id;
    for(;;){
        if((world_sock=Amazon_connect_to_world(23456,world_id))==-1){
            std::cout<<"Amazon: Failed to connect to world"<<std::endl;
            sleep(5);
            continue;
        }
        else{
            std::cout<<"Amazon: Connected to world"<<std::endl;
        }
        //break;
        if((ups_sock=Amazon_wait_for_UPS(5688,world_id))==-1){
            std::cout<<"Amazon: Failed to connect to UPS"<<std::endl;
            sleep(5);
            continue;
        }
        else{
            std::cout<<"Amazon: Connected to UPS"<<std::endl;
            break;
        }
        

    }
//    taskflow.emplace(WorldHandler());
//    taskflow.emplace([=](){Amazon_connect_frontend(front_end_port);});
    // pool.enqueue(UPSHandler());
  // pool.enqueue(FrontendHandler());
    // taskflow.emplace([](){

    //         test_send_Apurchasemore();
    //     }
    // );
    std::thread t1(WorldHandler);
    std::thread t2(FrontendHandler,front_end_port);
    std::thread t3(UPSHandle);
    t1.join();
    t2.join();
    t3.join();

    // executor.run(taskflow).wait(); 
    for (;;) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}


