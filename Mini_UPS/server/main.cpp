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
#include "protobuf/world_ups.pb.h"
//#include "threadpool.hpp"
//#include "socket.hpp"
#include "amazonreq.hpp"
#include "worldreq.hpp"
//truck status:idle:0,traveling:1,arrive warehouse:2, loading:3,delivering:4
//package status:created 0,picked up 1,out for delivery 3,delivered:5
pqxx::connection *C;
class parameter{
public:
    int world_fd;
    int amazon_fd;
    //int *seq_num;
    AUCommands message;
    std::unordered_set<int> *store;
    parameter(int wfd,int afd,AUCommands msg,std::unordered_set<int> *tstore):world_fd(wfd),amazon_fd(afd),message(msg),store(tstore){

    }
    ~parameter(){

    }
};
class parameterw{
public:
    int world_fd;
    int amazon_fd;
    //int *seq_num;
    UResponses message;
    std::unordered_set<int> *store;
    parameterw(int wfd,int afd,UResponses msg,std::unordered_set<int> *tstore):world_fd(wfd),amazon_fd(afd),message(msg),store(tstore){

    }
    ~parameterw(){

    }
};


void* handleworld(void *info){
  helper& global=helper::get_instance();
  parameterw *pw=(parameterw*) info;
  UResponses message=pw->message;
  std::vector<std::thread> threads;
  //insert ack!!!
  for(int i=0;i<message.acks_size();i++){
    int ack = message.acks(i);
    global.store.insert(ack);
    std::cout<<"receive world ack: "<<ack<<std::endl;
  }
  //send sequence back!!!!!!
  UCommands ucom;
  for(int i = 0; i < message.completions_size(); ++i) {
    ucom.add_acks(message.completions(i).seqnum());
  }
  for(int i = 0; i < message.delivered_size(); ++i) {
    ucom.add_acks(message.delivered(i).seqnum());
  }
  for(int i=0;i<message.truckstatus_size();i++){
    ucom.add_acks(message.truckstatus(i).seqnum());
  }
  for(int i=0;i<message.error_size();i++){
    ucom.add_acks(message.error(i).seqnum());
  }
  {
  std::unique_ptr<google::protobuf::io::FileOutputStream> out(new google::protobuf::io::FileOutputStream(pw->world_fd));
    if (sendMesgTo<UCommands>(ucom, out.get()) == false) {
        std::cout << "Cannot send ack to world." << std::endl;
    }
  }
  if(message.delivered_size()!=0){
    for(int i=0;i<message.delivered_size();i++){
      UDeliveryMade handle=message.delivered(i);
      if(global.recvs.find(handle.seqnum()) != global.recvs.end()) {
        continue;
      }else{
        global.recvs.insert(handle.seqnum());
      }
      //global.POOL->assign_task(std::bind(handleDelivery,handle));
      //std::thread tempthread(handleDelivery,handle);
      //tempthread.detach();
      threads.emplace_back(handleDelivery,handle);
    }
  }
  if(message.completions_size()!=0){
    for(int i=0; i<message.completions_size(); i++){
      UFinished handle = message.completions(i);
      if(global.recvs.find(handle.seqnum()) != global.recvs.end()) {
        continue;
      }else{
        global.recvs.insert(handle.seqnum());
      }
      if(handle.status()=="IDLE"){
        //packageAllFinished(ursp);
        //WR.handleIdle(handle);
        //global.POOL->assign_task(std::bind(handleIdle,handle));
        threads.emplace_back(handleIdle,handle);
      }else{
        //global.POOL->assign_task(std::bind(dealUFinished,handle));
        threads.emplace_back(dealUFinished,handle);
      }
    }
  }
  if(message.truckstatus_size()>0){
    for(int i=0;i<message.truckstatus_size();i++){
      const UTruck& ps = message.truckstatus(i);
      std::cout << "Get truck status for trick id=" << ps.truckid()<< " status=" << ps.status() << std::endl;
    }
  }
  if (message.has_finished()) {
    std::cout << "Finished!!!!!"<<std::endl;
  }

  if(message.error_size()>0){
    for (int i = 0; i < message.error_size(); ++i) {
      const UErr& aerr = message.error(i);
      std::cerr << "Got an error message for seq=" << aerr.originseqnum() << " msg: " << aerr.err() << std::endl;
    }
  }
  for (auto& t : threads) {
    t.join();
  }
  delete pw;
  return nullptr;
}



void* handleamazon(void *info){
  helper& global=helper::get_instance();
  parameter *p=(parameter*) info;
  AUCommands message=p->message;
  std::vector<std::thread> threads;
  std::cout<<message.pickupreq_size()<<std::endl;
  std::cout<<message.loaded_size()<<std::endl;
  for(int i=0;i<message.acks_size();i++){
    std::cout<<"received ack from amazon:"<<message.acks(i)<<std::endl;
  }
  //amazonreq AR(C,p->world_fd,p->amazon_fd,p->seq_num,p->store);
  UACommands commands;
  for(int i=0;i<message.acks_size();i++){
    global.store.insert(message.acks(i));
  }
  for(int i=0;i<message.pickupreq_size();i++){
    commands.add_acks(message.pickupreq(i).seqnum());
  }
  for(int i=0;i<message.loaded_size();i++){
    commands.add_acks(message.loaded(i).seqnum());
  }
  {
  std::unique_ptr<google::protobuf::io::FileOutputStream> out(new google::protobuf::io::FileOutputStream(global.amazon_fd));
    if (sendMesgTo<UACommands>(commands, out.get()) == false) {
        std::cout << "Cannot send ack to world." << std::endl;
    }
  }
  if(message.pickupreq_size()>0){
    for(int i=0;i<message.pickupreq_size();i++){
      AUInitPickUp handle=message.pickupreq(i);
      if(global.recvs_amazon.find(handle.seqnum()) != global.recvs_amazon.end()) {
        continue;
      }else{
        global.recvs_amazon.insert(handle.seqnum());
      }
      //global.POOL->assign_task(std::bind(pickup,handle));
      threads.emplace_back(pickup,handle);
    }
  }
  if(message.loaded_size()>0){
    for(int i=0;i<message.loaded_size();i++){
      AULoaded handle=message.loaded(i);
      if(global.recvs_amazon.find(handle.seqnum()) != global.recvs_amazon.end()) {
        continue;
      }else{
        global.recvs_amazon.insert(handle.seqnum());
      }
      //global.POOL->assign_task(std::bind(parseloaded,handle));
      threads.emplace_back(parseloaded,handle);
    }
  }
  for (auto& t : threads) {
    t.join();
  }
  delete p;
  return nullptr;
}
/*
void dealquery(int truckid){
  helper& global=helper::get_instance();
  std::unique_ptr<pqxx::connection> C(global.connectDB());
  UCommands command;
  UQuery* tempquery= command.add_queries();
  tempquery->set_truckid(truckid);
}
class helperchange{
public:
  std::string message;
  int socket_fd;
  helperchange(std::string target,int target_fd):message(target),socket_fd(target_fd){

  }
};
*/
/*
void* changeDestination(void *info){
  helperchange* h=(helperchange*) info;
  std::string message=h->message;
  int socket_fd=h->socket_fd;
  size_t position1=message.find('\n');
  size_t position2=message.find('\n',position1+1);
  int packageid=stoi(message.substr(position1+1,position2-position1-1));//可能是64位！！！！！最后再改
  size_t position3=message.find('\n',position2+1);
  int userx=stoi(message.substr(position2+1,position3-position2-1));
  size_t position4=message.find('\n',position3+1);
  int usery=stoi(message.substr(position3+1,position4-position3-1));
  helper& global=helper::get_instance();
  std::unique_ptr<pqxx::connection> C(global.connectDB());
  try{
  std::string sql="UPDATE user_packages SET userx = "+std::to_string(userx) + ", usery = "+std::to_string(usery) + ",change=1 WHERE packageid = "+std::to_string(packageid)+" AND (status='created' OR status='picked up');";
  //execute(sql,C.get());
  pqxx::work txn(*(C.get()));
  pqxx::result result = txn.exec(sql);
  std::string temp="Change Successfully!";
  send(socket_fd,temp.c_str(),temp.size(),0);
  }catch(const std::exception& e) {
    std::string temp="Lost the race!Changing destination failed!";
    send(socket_fd,temp.c_str(),temp.size(),0);
  }
  global.disConnectDB(C.get());
  return nullptr;
}


void* dealwithfront(void* info){
  helper& global=helper::get_instance();
  while(true){
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    int front_end_fd =accept(global.socket_fd, (struct sockaddr*)&socket_addr,&socket_addr_len);
    if (front_end_fd == -1) {
      std::cerr<<"Cannot connect with front end!!!"<<std::endl;
    }
    std::vector<char> tempstore(512,0);
    int len = recv(front_end_fd, tempstore.data(), tempstore.size(), 0);
    close(front_end_fd);
    std::string msg(tempstore.data());
    std::unique_ptr<helperchange> h(new helperchange(msg,front_end_fd));
    size_t position1=msg.find('\n');
    std::string type=msg.substr(0,position1);
    if(type=="1"){//"1" is change user destination, filter the 
      //global.POOL->assign_task(std::bind(changeDestination,msg));
      std::thread myThread(changeDestination,(void*)h.get());
      //pthread_t thread1;
      //pthread_create(&thread1, NULL, changeDestination, (void*)h.get());
      myThread.join();
      close(front_end_fd);
    }else if(type=="2"){
      //type==2 is cancel!!!!!!!!
      
    }
    
    
  }
  return nullptr;
}*/

int main(){
    //int* sequence;
    //*sequence=0;
    std::unordered_set<int> store;
    std::string hostname="vcm-30639.vm.duke.edu";
    std::string portw="12345";
    std::string porta="5688";
    helper& global=helper::get_instance();
    /*
    std::cout<<"tom"<<std::endl;
    std::vector<char> store1(1024,0);
    int size=recv(global.email_fd,store1.data(),store1.size(),0);
    if(size<=0){
      std::cout<<"error"<<std::endl;
    }
    std::string temp1(store1.data());
    std::cout<<temp1<<std::endl;
    std::cout<<"tom1"<<std::endl;*/
    bool first=false;
    try{
        std::string sql="host=db port=5432 dbname=postgres user=postgres password=passw0rd";
        C = new pqxx::connection(sql);
        if (C->is_open()) {
            std::cout << "Opened database successfully: " << C->dbname() << std::endl;
        } else {
            std::cout << "Cannot open database" << std::endl;
        }
    }catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    try{
        pqxx::nontransaction N(*C);
        //std::string SQLreq="SELECT EXISTS(SELECT 1 FROM information_schema.tables WHERE table_name='trucks');";
        std::string SQLreq="SELECT COUNT(*) FROM USER_TRUCKS;";
        pqxx::result R(N.exec(SQLreq));
        N.commit();
        if (R[0][0].as<int>()==0) {
          first=true;
          std::cout<<"The database is empty, insert trucks right now!"<<std::endl;
        }
    }catch(const pqxx::pqxx_exception &e){
        first=true;
        std::cout<<"The database is empty, insert trucks right now!"<<std::endl;
    }
    AUInitConnect ConnectfromA;
    std::unique_ptr<google::protobuf::io::FileInputStream> inputA(new google::protobuf::io::FileInputStream(global.amazon_fd));
    if (recvMesgFrom<AUInitConnect>(ConnectfromA, inputA.get()) == false) {
        std::cerr<< "Error: cannot receive from Amazon."<<std::endl;
        return EXIT_FAILURE;
    }
    int worldid=ConnectfromA.worldid();
    UConnect world;
    UInitTruck trucks;
    int truckNum = 10;
    for(int i = 0; i < truckNum; ++i) {
        UInitTruck * trucks = world.add_trucks();
        trucks->set_id(i);
        trucks->set_x(i);
        trucks->set_y(i);
        //global.truck_details[i]=std::pair<std::pair<int,int>,int>(std::pair<int,int>(i,i),0);
        if(first){
            std::string sql = "INSERT INTO user_trucks (TRUCKID, TRUCKX, TRUCKY, STATUS) VALUES("+std::to_string(i) + "," + std::to_string(i) + "," + std::to_string(i) + "," + "'idle'" + ");";
            pqxx::work W(*C);
            W.exec(sql);
            W.commit();
        }
    }
    world.set_worldid(worldid);
    world.set_isamazon(false);
    std::unique_ptr<google::protobuf::io::FileOutputStream> output(new google::protobuf::io::FileOutputStream(global.world_fd));
    if (sendMesgTo<UConnect>(world, output.get()) == false) {
        std::cerr<< "Cannot send UConnect to world." << std::endl;
        return EXIT_FAILURE;
    }
    UConnected worlded;
    std::unique_ptr<google::protobuf::io::FileInputStream> input(new google::protobuf::io::FileInputStream(global.world_fd));
    if (recvMesgFrom<UConnected>(worlded, input.get()) == false) {
        std::cerr<< "Error: cannot receive UConnected from world."<<std::endl;
        return EXIT_FAILURE;
    }
    std::string result = worlded.result();
    if(result != "connected!"){
        std::cerr<< "Cannot connect to world."<<std::endl;
    }
    UAConfirmConnected confirmed;
    confirmed.set_worldid(worldid);
    confirmed.set_connected(result=="connected!");
    std::unique_ptr<google::protobuf::io::FileOutputStream> outputA(new google::protobuf::io::FileOutputStream(global.amazon_fd));
    if (sendMesgTo<UAConfirmConnected>(confirmed, outputA.get()) == false) {
        std::cerr<< "Cannot send to Amazon." << std::endl;
        return EXIT_FAILURE;
    }
    C->disconnect();
    int id = worlded.worldid();
    fd_set read_fds;
    int nfds = std::max(global.world_fd, global.amazon_fd) + 1;
    while(true){
        FD_ZERO(&read_fds);
        FD_SET(global.world_fd, &read_fds);
        FD_SET(global.amazon_fd, &read_fds);
        int rv = select(nfds, &read_fds, NULL, NULL, NULL);
        if(rv == -1) {
            std::cerr<<"Cannot select"<<std::endl;
            break;
        }
        if(rv>0){
            if(FD_ISSET(global.world_fd, &read_fds)){
                UResponses ursp;
                std::unique_ptr<google::protobuf::io::FileInputStream> tempinput(new google::protobuf::io::FileInputStream(global.world_fd));
                if (recvMesgFrom<UResponses>(ursp, tempinput.get()) == false) {
                    std::cout << "Cannot receive UResponses from world.";
                    continue;
                }
                parameterw *Pw=new parameterw(global.world_fd,global.amazon_fd,ursp,&store);
                //std::cout<<"goooooood"<<std::endl;
                global.POOL->assign_task(std::bind(handleworld,(void *)Pw));
                std::cout<<"enter handleworld"<<std::endl;
            }
            if(FD_ISSET(global.amazon_fd, &read_fds)){
                AUCommands message;       
                std::unique_ptr<google::protobuf::io::FileInputStream> tempinput(new google::protobuf::io::FileInputStream(global.amazon_fd));
                if (recvMesgFrom<AUCommands>(message, tempinput.get()) == false) {
                    std::cout<< "Cannot receive AU_commands from Amazon.";
                    continue;
                }
                parameter *P=new parameter(global.world_fd,global.amazon_fd,message,&store);
                global.POOL->assign_task(std::bind(handleamazon,(void *)P));
                std::cout<<"enter handlewamazon"<<std::endl;
            }
    }
  }
    return EXIT_SUCCESS;
}
