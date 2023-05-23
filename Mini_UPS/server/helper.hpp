#ifndef HELPER_HPP
#define HELPER_HPP
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <mutex>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <vector>
#include <map>
#include <pqxx/pqxx>
#include <thread>
#include <algorithm>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "threadpool.hpp"
#include "socket.hpp"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//this is adpated from code that a Google engineer posted online 
template<typename T>
bool sendMesgTo(const T & message, google::protobuf::io::FileOutputStream *out) { 
{     //extra scope: make output go away before out->Flush()
    // We create a new coded stream for each message.
    // Don’t worry, this is fast. 
    google::protobuf::io::CodedOutputStream output(out); // Write the size.
    const int size = message.ByteSize();
    output.WriteVarint32(size);
    uint8_t* buffer = output.GetDirectBufferForNBytesAndAdvance(size);
    if (buffer != NULL) {
        // Optimization: The message fits in one buffer, so use the faster 
        // direct-to-array serialization path. 
        message.SerializeWithCachedSizesToArray(buffer);
    }
    else {
    // Slightly-slower path when the message is multiple buffers. 
    message.SerializeWithCachedSizes(&output);
    if (output.HadError()) {
    return false; }
} }
    out->Flush();
    return true; 
}

//this is adpated from code that a Google engineer posted online 
template<typename T>
bool recvMesgFrom(T & message, google::protobuf::io::FileInputStream * in ){ 
    google::protobuf::io::CodedInputStream input(in);
    uint32_t size;
    if (!input.ReadVarint32(&size)) {
        return false; 
    }
    // Tell the stream not to read beyond that size. 
    google::protobuf::io::CodedInputStream::Limit limit = input.PushLimit(size);
    // Parse the message.
    if (!message.MergeFromCodedStream(&input)) {
        return false; 
    }
    if (!input.ConsumedEntireMessage()) { 
        return false;
    }
    // Release the limit. 
    input.PopLimit(limit); 
    return true;
}

void execute(std::string sql,pqxx::connection* C) {
    pqxx::work W(*C);
    W.exec(sql);
    W.commit();
}

pqxx::result selectvalue(std::string sql,pqxx::connection* C){
    pqxx::work W(*C);
    pqxx::result R(W.exec(sql));
    W.commit();
    return R;
}

class helper{
public:
  std::string hostnamea;
  std::string hostnamew;
  std::string portw;
  std::string porta;
  std::string dbname;
  std::string user;
  std::string password;
  int socket_fd;
  int email_fd;
  int world_fd;
  int amazon_fd;
  int seqNum=0;
  threadpool pool;
  threadpool* POOL;
  std::mutex seqnum_mtx;
  std::unordered_set<int> store;
  std::unordered_set<int> recvs;
  std::unordered_set<int> recvs_amazon;
  //std::unordered_map<int,std::pair<std::pair<int,int>,int>> truck_details;
  ~helper(){
    close(world_fd);
    close(amazon_fd);
  }
  int getSeqNum() {
    std::lock_guard<std::mutex> server_lk(seqnum_mtx);
    return ++seqNum;
  }
  static helper& get_instance() {
      static helper instance;
      return instance;
  }
  pqxx::connection* connectDB() {
    std::string sql="host=db port=5432 dbname=postgres user=postgres password=passw0rd";
    pqxx::connection* C =new pqxx::connection(sql);
    if (C->is_open()) {
        // cout << "Opened database successfully: " << C->dbname() << endl;
    } else {
        std::cerr<<"Can't open database."<<std::endl;
    }
    return C;
}
void disConnectDB(pqxx::connection* C) {
    C->disconnect();
}
  helper(const helper&) = delete;
  helper& operator=(const helper) = delete;
  private:
  helper():hostnamea("vcm-27827.vm.duke.edu"),hostnamew("vcm-27827.vm.duke.edu"),portw("12345"),porta("5688"),dbname("postgres"),user("postgres"),password("passw0rd"){
    POOL=pool.get_pool();
    Socket S;
    socket_fd=S.BuildSocket();
    struct sockaddr_storage connector_addr;
    socklen_t addr_len = sizeof(connector_addr);
    email_fd= accept(socket_fd, (struct sockaddr *)&connector_addr, &addr_len);
    if(email_fd==-1){
        std::cout<<"cannot accept"<<std::endl;
    }
    world_fd=S.ConnectTo(hostnamew,portw);
    amazon_fd=S.ConnectTo(hostnamea,porta);
  }
};
class temp1{
public:
int pick_truck(int pkg_id,pqxx::connection* C){
    int truck_id;
    while(true){
    std::string sql = "SELECT truckid FROM user_trucks WHERE status = 'idle';";
    pqxx::result res = selectvalue(sql,C);
    if(!res.empty()) {
        truck_id = res[0][0].as<int>();
        std::string sql3 = "UPDATE user_packages SET truckid_id = "+ std::to_string(truck_id) +" WHERE packageid = " + std::to_string(pkg_id) + "; ";
        execute(sql3,C);
        break;
    }
    std::cout<<"waiting for truck"<<std::endl;
    sleep(3);
    /*else{
        std::string sql1 = "SELECT truckid FROM user_trucks WHERE status = 'arrive warehouse' OR status='delivering';";
        pqxx::result res1 = selectvalue(sql1,C);
        if(!res.empty()){
            truck_id = res1[0][0].as<int>();
            std::string sql2 = "UPDATE user_packages SET truckid_id = "+ std::to_string(truck_id) +" WHERE packageid = " + std::to_string(pkg_id) + "; ";
            execute(sql2,C);
            break;
        }
    }*/
    }
    return truck_id;
}//有问题
};
#endif
