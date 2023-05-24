#ifndef COMMON_HPP
#define COMMON_HPP
#include<mutex>
#include<vector>
#include<thread>
#include<memory>
#include<cstdlib>
#include<bitset>
#include"ThreadPool.h"
#include"GPB_message.hpp"
#include"protobuf/world_amazon.pb.h"
#include"protobuf/amazon_ups.pb.h"
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "db_func.hpp"

extern long long server_seq_num;
extern std::mutex server_seq_num_mutex;
extern ThreadPool pool;

extern const char *world_ip;
extern const char *ups_ip;
extern const char *front_end_ip;
extern const int world_port;
extern const int ups_port;
extern const int front_end_port;

extern int world_sock;
extern int ups_sock;

typedef struct warehouse{
    int id;
    int x;
    int y;
    warehouse(int id,int x,int y):id(id),x(x),y(y){}
}Warehouse;

extern std::vector<Warehouse> w_list;

extern std::bitset<100000> send_acks; //whether receiver ack for sent message
extern std::bitset<100000> recv_ups_acks; //whether finished received message
extern std::bitset<100000> recv_world_acks; //whether finished received message

typedef struct OrderInfo{
    long long package_id;
    std::string account_name;
    int delivery_x;
    int delivery_y;
    OrderInfo()=default;
    OrderInfo(long long package_id,std::string account_name,int delivery_x,int delivery_y):\
    package_id(package_id),account_name(account_name),delivery_x(delivery_x),delivery_y(delivery_y){}
}OrderInfo;


//For ApurchaseMore to retrive back orderinfo, update by send_ApurchaseMore_to_world
extern std::map<long long, OrderInfo> seqnum_to_orderinfo; //

//For Apack to retrive back truckid, update by Process_UAresponse (UATruckArrived)
extern std::map<long long, int> shipid_to_truckid; //
//Set in send_ApurchaseMore_to_world
extern std::map<long long, int> shipid_to_whid; //

extern const int num_of_warehouses;


static long long get_server_seq_num(){
    std::lock_guard<std::mutex> lock(server_seq_num_mutex);
    return server_seq_num++;
}




#endif
