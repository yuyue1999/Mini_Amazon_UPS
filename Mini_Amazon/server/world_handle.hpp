#ifndef WORLD_HANDLE_HPP
#define WORLD_HANDLE_HPP

#include<mutex>
#include<vector>
#include<thread>
#include<memory>
#include<cstdlib>
#include<exception>
#include <utility>
#include<bitset>
#include<mutex>
#include"ThreadPool.h"
#include"GPB_message.hpp"
#include"protobuf/world_amazon.pb.h"
#include"protobuf/amazon_ups.pb.h"
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "common.hpp"
#include "UPS_handle.hpp"

std::mutex world_sock_mutex;
std::mutex purchase_mutex[20];
std::vector<APurchaseMore> purchase_request_list[20];

//--------------------send message to world--------------------


int Send_command_to_world(ACommands acommands,int seq_num){
   // std::cout<<"Get into send command to world"<<std::endl;
    while(true){
        // whether continue looping
        if(seq_num!=-1 && send_acks[seq_num]==true){

            if(seq_num!=-1){
                std::cout<<"Amazon: Received ack from world, stop sending seq_num:" << seq_num<<std::endl;
                break;
            }
        }
        //std::cout<<"pass check"<<std::endl;
        //print ApurchaseMore in acommands
        // for(auto &apurchasemore:acommands.buy()){
        //     std::cout<<"apurchasemore in sending: "<<std::endl;
        //     std::cout<<"whnum: "<<apurchasemore.whnum()<<std::endl;
        //     for(auto &p:apurchasemore.things()){
        //         std::cout<<"product: "<<std::endl;
        //         std::cout<<"id: "<<p.id()<<std::endl;
        //         std::cout<<"description: "<<p.description()<<std::endl;
        //         std::cout<<"count: "<<p.count()<<std::endl;
        //     }
        // }
       // std::cout<<"seq num in sending: "<<seq_num<<std::endl;

        try{
         //   std::lock_guard<std::mutex> lock(world_sock_mutex);
            //send to world
            //std::cout<<"prepare sending"<<std::endl;
            std::unique_ptr<GPBFileOutputStream> output(new GPBFileOutputStream(world_sock));
            if(sendMesgTo(acommands,output.get())!=true){
                throw std::runtime_error("Amazon: send command to world failed");
            }
            //std::cout<<"send success"<<std::endl;
        }catch(std::exception &e){
            std::cout<<"Amazon: send command to world failed"<<std::endl;
            return -1;
        }
        if(seq_num!=-1)
            std::cout<<"Amazon: send command to world seqnum: "<<seq_num<<std::endl;
        else
            std::cout<<"Amazon: send ack to world"<<std::endl;

        // seq_num==-1 means send ack back, no need to wait for ack
        if(seq_num==-1) break;

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }       
    

    return 0;
}

int send_ApurchaseMore_to_world(int wh_id,std::vector<AProduct> &products,\
        long long packageid,std::string accountname,int deliver_x,int deliver_y){
    APurchaseMore apurchasemore;
    apurchasemore.set_whnum(wh_id);
    for(auto &p:products){
        auto apurchase = apurchasemore.add_things();
        *apurchase=std::move(p);
    }
    long long seq_num=get_server_seq_num();
    apurchasemore.set_seqnum(seq_num);
    send_acks[seq_num]=false;
    ACommands acommands;
    acommands.add_buy()->CopyFrom(apurchasemore);
    OrderInfo orderinfo{packageid,accountname,deliver_x,deliver_y};
    seqnum_to_orderinfo[seq_num]=orderinfo;
    shipid_to_whid[packageid]=wh_id;
    
    std::lock_guard<std::mutex> lock(purchase_mutex[wh_id]);
    purchase_request_list[wh_id].emplace_back(apurchasemore);
    
    //std::lock_guard<std::mutex> lock(world_sock_mutex);
    std::cout<<"Amazon: send APurchaseMore to world shipid: "<<packageid<<" seq_num: "<<seq_num<<std::endl;
    // std::cout << "Before enqueueing Send_command_to_world" << std::endl;
    // std::cout<<"world sock in sending "<<world_sock<<std::endl;
    //pool.enqueue(Send_command_to_world,acommands,seq_num);
    //Send_command_to_world(acommands,seq_num);
   // taskflow.emplace([=]() { Send_command_to_world(std::move(acommands),seq_num); });
    //executor.run(taskflow).wait(); 
        // Create a new taskflow and add the task to it
    std::thread sending_thread(Send_command_to_world,acommands,seq_num);
    sending_thread.detach(); 
    //std::cout << "After enqueuing Send_command_to_world" << std::endl;
    return 0;
}

int send_APack_to_world(int wh_id,google::protobuf::RepeatedPtrField<AProduct> products,long long shipid){
    std::cout<<"Get into send APack to world"<<std::endl;
    APack apack;
    apack.set_whnum(wh_id);
    for(auto &p:products){
        auto thing = apack.add_things();
        *thing=std::move(p);
    }
    apack.set_shipid(shipid);
    long long seq_num=get_server_seq_num();
    apack.set_seqnum(seq_num);
    send_acks[seq_num]=false;
    ACommands acommands;
    acommands.add_topack()->CopyFrom(apack);
    std::cout<<"Amazon: send APack to world shipid: "<<shipid<<" seqnum: "<<seq_num<<std::endl;
    // pool.enqueue(Send_command_to_world,acommands,seq_num);
    std::thread sending_thread(Send_command_to_world,acommands,seq_num);
    sending_thread.detach();
    return 0;
}

int send_APutOnTruck_to_world(int wh_id,int truckid, int shipid){
    APutOnTruck aputontruck;
    aputontruck.set_whnum(wh_id);
    aputontruck.set_truckid(truckid);
    aputontruck.set_shipid(shipid);
    long long seq_num=get_server_seq_num();
    aputontruck.set_seqnum(seq_num);
    send_acks[seq_num]=false;
    ACommands acommands;
    acommands.add_load()->CopyFrom(aputontruck);
    std::cout<<"Amazon: send APutOnTruck to world shipid: "<<shipid<<" seqnum: "<<seq_num<<std::endl;
    //pool.enqueue(Send_command_to_world,acommands,seq_num);
    std::thread sending_thread(Send_command_to_world,acommands,seq_num);
    sending_thread.detach();
    return 0;
}

// int send_AQuery_to_world(int wh_id,std::vector<AQuery> &queries){

// }

int send_acks_to_world(int ack){
    ACommands acommands;
    acommands.add_acks(ack);
    std::cout<<"Amazon: send received ack to world: "<<ack<<std::endl;
    // pool.enqueue(Send_command_to_world,acommands,-1);
    std::thread sending_thread(Send_command_to_world,acommands,-1);
    sending_thread.detach();
    return 0;
}







//--------------------receive message from world--------------------

//parsing Aresponse
//TO-DO: finished, packagestatus

int Process_Arrived(APurchaseMore now_arrived){
    std::cout<<"Amazon: receive APurchaseMore arrived from world seqnum: "<<now_arrived.seqnum()<<std::endl;
    if(recv_acks[now_arrived.seqnum()]==true){
        send_acks_to_world(now_arrived.seqnum());
    }
    recv_acks[now_arrived.seqnum()]=true;

    std::lock_guard<std::mutex> lock(purchase_mutex[now_arrived.whnum()]);
    //iterate through the purchase map
    for(auto &p:purchase_request_list[now_arrived.whnum()]){
        //check wharehouse id and all the product info
        if(p.whnum()==now_arrived.whnum() && p.things_size()==now_arrived.things_size()){
            bool flag=true;
            for(int i=0;i<p.things_size();i++){
                if(p.things(i).id()!=now_arrived.things(i).id() || p.things(i).description()!=now_arrived.things(i).description() || p.things(i).count()!=now_arrived.things(i).count()){
                    flag=false;
                    break;
                }
            }
            if(flag==true){
                //find order info according to seqnum stored in p
                auto it = seqnum_to_orderinfo.find(p.seqnum());
                if(it==seqnum_to_orderinfo.end()){
                    std::cout<<"Amazon: seqnum_to_orderinfo not found"<<std::endl;
                    return -1;
                }
                std::cout<<"Amazon: find order info according to original seqnum: "<<p.seqnum()<<std::endl;
                OrderInfo &now_orderinfo=it->second;
                send_APack_to_world(now_arrived.whnum(),now_arrived.things(),now_orderinfo.package_id);

                // //TO-DO： update order status to be packing
                 Update_Order_Status(now_orderinfo.package_id,"packing");
                AUDeliveryLocation audeliverylocation;
                audeliverylocation.set_x(now_orderinfo.delivery_x);
                audeliverylocation.set_y(now_orderinfo.delivery_y);
                audeliverylocation.set_packageid(now_orderinfo.package_id);
                //FORTEST
                Send_AUInitPickUP_to_UPS(now_arrived.whnum(),now_orderinfo.account_name,\
                    audeliverylocation,now_arrived.things());
                //remove stored order info
                //purchase_request_list[now_arrived.whnum()].erase();
            }
        }
    }

    // OrderInfo &now_orderinfo=it->second;
    // send_APack_to_world(now_arrived.whnum(),now_arrived.things(),now_orderinfo.package_id);




    
    send_acks_to_world(now_arrived.seqnum());
    return 0;
}

int Process_APacked(APacked now_packed){

    std::cout<<"Amazon: receive APacked from world shipid: "<<now_packed.shipid()<<" seqnum: "<<now_packed.seqnum()<<std::endl;
    if(recv_acks[now_packed.seqnum()]==true){
        send_acks_to_world(now_packed.seqnum());
    }
    recv_acks[now_packed.seqnum()]=true;
    Update_Order_Status(now_packed.shipid(),"packed");
    while(shipid_to_truckid.find(now_packed.shipid())==shipid_to_truckid.end()){
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
       /// FORTEST
       std::cout<<"Amazon: wait for UATruckArrived"<<std::endl;
    }
    Update_Order_Status(now_packed.shipid(),"loading");
    send_APutOnTruck_to_world(shipid_to_whid[now_packed.shipid()],shipid_to_truckid[now_packed.shipid()],now_packed.shipid());
   
    // send_APutOnTruck_to_world(1,1,1);
   
    send_acks_to_world(now_packed.seqnum());
    return 0;
}

int Process_Aresponse(AResponses aresponses){
    //Parchase more arrived received
    //Amazon: No.8 send Apack to world
    //DB: update order status to be packing
    //Amazon: No.8 send AUInitPickUp to UPS
    for(auto &now_arrived:aresponses.arrived()){
        // pool.enqueue(Process_Arrived,now_arrived);
        Process_Arrived(now_arrived);
    }
    //Apacked ready received
    //check whether UATruckArrived received
    //if yes, send APutOnTruck to world
    //else wait for UATruckArrived
    //DB: update order status to be packed
    for(auto &now_packed:aresponses.ready()){
        // pool.enqueue(Process_APacked,now_packed);
        std::thread processing_thread(Process_APacked,now_packed);
        processing_thread.detach();
    }

    //ALoaded loaded received
    //Send AULoaded to UPS
    for(auto &now_loaded:aresponses.loaded()){
        std::cout<<"Amazon: receive ALoaded from world shipid: "<<now_loaded.shipid()<<" seqnum: "<<now_loaded.seqnum()<<std::endl;
        if(recv_acks[now_loaded.seqnum()]==true){
            send_acks_to_world(now_loaded.seqnum());
        }
        recv_acks[now_loaded.seqnum()]=true;
        Update_Order_Status(now_loaded.shipid(),"delivering");
        Send_AULoaded_to_UPS(now_loaded.shipid());
        send_acks_to_world(now_loaded.seqnum());
    }

    //AErr error received
    //Print error message 
    for(auto &now_error:aresponses.error()){
        std::cout<<"Amazon: world error: "<<now_error.err()<<" originseqnum: "<<now_error.originseqnum()<<std::endl;
        send_acks_to_world(now_error.seqnum());
    }

    //acks received from world
    //update bitset
    for(auto &now_ack:aresponses.acks()){
        std::cout<<"Amazon: receive ack from world: "<<now_ack<<std::endl;
        send_acks[now_ack]=true;
    }
    return 0;
}

int receive_Aresponse_from_world(){
    // std::cout<<"world sock in receiving "<<world_sock<<std::endl;
   // std::cout<<"world receiver running"<<std::endl; 
    AResponses aresponses;
    try{ 
       // std::lock_guard<std::mutex> lock(world_sock_mutex);
        std::unique_ptr<GPBFileInputStream> input(new GPBFileInputStream(world_sock));
        if(recvMesgFrom(aresponses,input.get())!=true){
            throw std::runtime_error("Amazon: receive Aresponse from world failed");
        }
    }catch(std::exception &e){
        std::cout<<"Amazon: receive Aresponse from world failed"<<std::endl;
        return -1;
    }
    std::cout<<"Amazon: receive Aresponse from world"<<std::endl;
    // if(Process_Aresponse(aresponses)!=0){
    //     std::cout<<"Amazon: process Aresponse failed"<<std::endl;
    //     return -1;
    // }
    // tf::Taskflow newtaskflow;
    // newtaskflow.emplace([=](){Process_Aresponse(std::move(aresponses));});
    // executor.run(newtaskflow).wait();
    Process_Aresponse(std::move(aresponses));
    return 0;
}
    void WorldHandler() {
        while(true){
            receive_Aresponse_from_world();
            // std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
        }
    }
// class WorldHandler {
// public:
//     WorldHandler() {}
//     void operator()() {
//         while(true){
//             receive_Aresponse_from_world();
//             std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
//         }
//     }    
// };


#endif