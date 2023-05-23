#include "protobuf/world_ups.pb.h"
#include "helper.hpp"
#include <string>
#include <pqxx/pqxx>
void pickup(AUInitPickUp temp){
    helper& global=helper::get_instance();
    std::unique_ptr<pqxx::connection> C(global.connectDB());
    UCommands toworld;
    int packageid=temp.deliverylocation().packageid(); //do i need to check the packageid is unique or not?
    std::string accountname=temp.accountname();
    int warehouse_id=temp.whid();
    AUDeliveryLocation destination=temp.deliverylocation();
    int userx=destination.x();
    int usery=destination.y();
    temp1 t;
    int truck_id=t.pick_truck(packageid,C.get()); 
    //bool empty=false;
    try{
        pqxx::nontransaction N(*(C.get()));
        std::string sql00="SELECT COUNT(*) FROM user_users WHERE username = "+N.quote(accountname)+";";
        std::cout<<sql00<<std::endl;
        pqxx::result R(N.exec(sql00));
        N.commit();
        if (R[0][0].as<int>()==0) {
          //empty=true;
          throw pqxx::argument_error("");
        }
    }catch(const pqxx::pqxx_exception &e){
        std::cout<<"The database is empty, insert user!"<<std::endl;
        std::string sqltemp="INSERT INTO user_users (username, password, email) VALUES ('"+accountname+"', '0000','yy373@duke.edu')";
        std::cout<<sqltemp<<std::endl;
        execute(sqltemp,C.get());
    }
    {
        //pqxx::work W(*(C.get()));
        //std::string sql22 = "INSERT INTO user_packages(packageid, warehouseid, userx, usery, truckid, status, finish, change, username) VALUES (" + std::to_string(packageid) + ", " + std::to_string(warehouse_id) + ", "+std::to_string(userx)+", "+std::to_string(usery)+", "+ std::to_string(truck_id)+", "+ "'created', 0, 0, '" +W.quote(accountname)+ "' );";
        std::string sql2 = "INSERT INTO user_packages(packageid, warehouseid, userx, usery, status, finish, change, truckid_id, user_id) VALUES (" + std::to_string(packageid) + ", " + std::to_string(warehouse_id) + ", "+std::to_string(userx)+", "+std::to_string(usery)+", "+ "'created', 0, 0, " + std::to_string(truck_id)+", '"+accountname+ "' );";
        std::cout<<sql2<<std::endl;
        execute(sql2,C.get());
    }
    for(int i=0;i<temp.product_size();i++){
        AUProduct product=temp.product(i);
        int product_id=product.id();
        std::string description=product.description();
        int count=product.count();
        std::string sql0="INSERT INTO user_product(productid, description, count, packageid_id) VALUES ("+std::to_string(product_id)+", '"+description+"', "+std::to_string(count)+", "+std::to_string(packageid)+")";
        execute(sql0,C.get());
        std::cout<<sql0<<std::endl;
    }
    UGoPickup * pickUps = toworld.add_pickups();
    pickUps->set_truckid(truck_id);
    pickUps->set_whid(warehouse_id);
    pickUps->set_seqnum(global.getSeqNum());
    int sequence=pickUps->seqnum();
    std::string sql3 = "UPDATE user_trucks SET status = 'traveling' WHERE truckid = " + std::to_string(truck_id) + ";";
    std::cout<<sql3<<std::endl;
    execute(sql3,C.get());
    global.disConnectDB(C.get());
    std::unique_ptr<google::protobuf::io::FileOutputStream> tempoutput(new google::protobuf::io::FileOutputStream(global.world_fd));
    if (sendMesgTo<UCommands>(toworld, tempoutput.get()) == false) {
        std::cout<< "Cannot receive AU_commands from Amazon.";
    }
    std::cout<<"jerry"<<std::endl;
    sleep(3);
    while(global.store.find(sequence)==global.store.end()){
        if (sendMesgTo<UCommands>(toworld, tempoutput.get()) == false) {
        std::cout<< "Cannot receive AU_commands from Amazon.";
        }
        sleep(3);
        std::cout<<"waiting for ack from world"<<sequence<<std::endl;
    }
}
void parseloaded(AULoaded temp){//有点小逻辑问题，再改！！！一部小车可能去很多地方送货！！！！！
    helper& global=helper::get_instance();
    std::unique_ptr<pqxx::connection> C(global.connectDB());
    UCommands ucom;
    int packageid = temp.shipid();
    std::string sql = "SELECT truckid_id FROM user_packages WHERE packageid = " + std::to_string(packageid) + " FOR UPDATE;";//要不要考虑status的值？？？回答：应该不需要
    pqxx::result res = selectvalue(sql,C.get());
    int truck_id=0;
    if(!res.empty()) {
        truck_id = res[0][0].as<int>();//get truck_id
    }
    UGoDeliver * ugd =ucom.add_deliveries();
    ugd->set_seqnum(global.getSeqNum());
    int sequence=ugd->seqnum();
    ugd->set_truckid(truck_id);
    UDeliveryLocation * spkg = ugd->add_packages();
    std::string sql0="SELECT userx, usery FROM user_packages WHERE packageid = "+std::to_string(packageid) + " FOR UPDATE;";
    std::cout<<sql0<<std::endl;
    pqxx::result res1 = selectvalue(sql0,C.get());
    int user_x = res1[0][0].as<int>();
    int user_y = res1[0][0].as<int>();
    spkg->set_packageid(packageid);
    spkg->set_x(user_x);
    spkg->set_y(user_y);
    std::string sql_pkg_dest = "UPDATE user_packages SET status = 'out for delivery' WHERE packageid = "+std::to_string(packageid)+";";
    std::cout<<sql_pkg_dest<<std::endl;
    execute(sql_pkg_dest,C.get());
    std::string sql_truck = "UPDATE user_trucks SET status = 'delivering' WHERE truckid = " + std::to_string(truck_id) + ";";
    std::cout<<sql_truck<<std::endl;
    execute(sql_truck,C.get());
    global.disConnectDB(C.get());
    std::unique_ptr<google::protobuf::io::FileOutputStream> tempoutput(new google::protobuf::io::FileOutputStream(global.world_fd));
    if (sendMesgTo<UCommands>(ucom, tempoutput.get()) == false) {
        std::cout<< "Cannot receive AU_commands from Amazon.";
    }
    sleep(3);
    while(global.store.find(sequence)==global.store.end()){
        if (sendMesgTo<UCommands>(ucom, tempoutput.get()) == false) {
        std::cout<< "Cannot receive AU_commands from Amazon.";
        }
        sleep(3);
        std::cout<<"waiting for ack from world"<<sequence<<std::endl;
    }
}