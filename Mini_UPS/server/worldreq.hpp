#include "protobuf/world_ups.pb.h"
#include "helper.hpp"
#include <string>
#include <pqxx/pqxx>
#include <vector>
void handleDelivery(UDeliveryMade handle){
  UACommands commands;
  helper& global=helper::get_instance();
  std::unique_ptr<pqxx::connection> C(global.connectDB());
  int truck_id=handle.truckid();
  int package_id=handle.packageid();
  std::string sql1 = "SELECT userx, usery FROM user_packages WHERE packageid = " + std::to_string(package_id) + " FOR UPDATE;";
  pqxx::result res = selectvalue(sql1,C.get());//可能没找到？？？？
  int userx = res[0][0].as<int>();
  int usery = res[0][1].as<int>();
  std::string sql2="UPDATE user_trucks SET truckx = " + std::to_string(userx) + ", trucky = " + std::to_string(usery) + " WHERE truckid = " + std::to_string(truck_id) + ";";
  execute(sql2,C.get());
  std::string sql3 = "UPDATE user_packages SET status = 'delivered',finish = 1 WHERE packageid = " + std::to_string(package_id)  + "; ";
  execute(sql3,C.get());
  std::string sql111="SELECT user_id FROM user_packages WHERE packageid="+std::to_string(package_id)+";";
  pqxx::result R=selectvalue(sql111,C.get());
  std::string username=R[0][0].as<std::string>();
  std::string sqle="SELECT email FROM user_users WHERE username='"+username+"';";
  pqxx::result R1=selectvalue(sqle,C.get());
  std::string email=R1[0][0].as<std::string>();
  //if(email!="yy@gmail.com"){
    std::string sendtopy="email:"+email+"$packageid:"+std::to_string(package_id)+"$username:"+username;
    send(global.email_fd,sendtopy.c_str(),sendtopy.size(),0);
  //}
  global.disConnectDB(C.get());
  UADelivered* temp=commands.add_delivered();
  temp->set_packageid(package_id);
  temp->set_truckid(truck_id);
  temp->set_seqnum(global.getSeqNum());
  int sequence=temp->seqnum();

  //global.store.insert(sequence);
  //std::unique_ptr<google::protobuf::io::FileInputStream> tempinput(new google::protobuf::io::FileInputStream(global.amazon_fd));
  std::unique_ptr<google::protobuf::io::FileOutputStream> tempoutput(new google::protobuf::io::FileOutputStream(global.amazon_fd));
  if (sendMesgTo<UACommands>(commands, tempoutput.get()) == false) {
    std::cout<< "Cannot receive AU_commands from Amazon.";
  }
  sleep(3);
  while(global.store.find(sequence)==global.store.end()){
    if (sendMesgTo<UACommands>(commands, tempoutput.get()) == false) {
      std::cout<< "Cannot receive AU_commands from Amazon.";
    }
    std::cout<<"Waiting for ack from amazon: "<<sequence<<std::endl;
    sleep(3);
  }
}
void dealUFinished(UFinished handle){
  helper& global=helper::get_instance();
  std::unique_ptr<pqxx::connection> C(global.connectDB());
  UACommands commands;
  int truck_id=handle.truckid();
  int truck_x=handle.x();
  int truck_y=handle.y();
  std::string status=handle.status();
  std::string sql_truck = "UPDATE user_trucks SET truckx = " + std::to_string(truck_x) + ", trucky = " + std::to_string(truck_y) + ", status = 'arrive warehouse' WHERE truckid = " + std::to_string(truck_id) + ";";
  execute(sql_truck,C.get());
  std::cout<<sql_truck<<std::endl;
  std::string sql_package = "UPDATE user_packages SET status = 'picked up' WHERE truckid_id = " + std::to_string(truck_id) + " AND status='created'"+";";//到时候再改！！！
  execute(sql_package,C.get());//有可能包裹在pickup阶段刚刚被insert创建，另一个线程就把其更改了status，不过倒也没事；
  std::cout<<sql_package<<std::endl;
  std::string sqlfind = "SELECT packageid FROM user_packages WHERE truckid_id = " + std::to_string(truck_id) + " AND STATUS = 'picked up' FOR UPDATE;";//有很多个package！！！！有问题
  pqxx::result res = selectvalue(sqlfind,C.get());
  std::cout<<sqlfind<<std::endl;
  global.disConnectDB(C.get());
  int package_id=res[0][0].as<int>();//可能找不到？？？？？有逻辑问题，一个truck可能有好多个package！！！！
  UATruckArrived *temp=commands.add_truckarrived();
  temp->set_shipid(package_id);
  temp->set_truckid(truck_id);
  temp->set_seqnum(global.getSeqNum());
  int sequence=temp->seqnum();
  //std::unique_ptr<google::protobuf::io::FileInputStream> tempinput(new google::protobuf::io::FileInputStream(global.amazon_fd));
  std::unique_ptr<google::protobuf::io::FileOutputStream> tempoutput(new google::protobuf::io::FileOutputStream(global.amazon_fd));
  if (sendMesgTo<UACommands>(commands, tempoutput.get()) == false) {
    std::cout<< "Cannot receive AU_commands from Amazon.";
  }
  sleep(3);
  while(global.store.find(sequence)==global.store.end()){
    if (sendMesgTo<UACommands>(commands, tempoutput.get()) == false) {
    std::cout<< "Cannot receive AU_commands from Amazon.";
    }
    std::cout<<"Waiting for ack from amazon: "<<sequence<<std::endl;
    sleep(3);
  }
}
void handleIdle(UFinished handle){
  helper& global=helper::get_instance();
  std::unique_ptr<pqxx::connection> C(global.connectDB());
  int truck_id=handle.truckid();
  int truck_x=handle.x();
  int truck_y=handle.y();
  std::string status=handle.status();
  std::string sql = "UPDATE user_trucks SET truckx = " + std::to_string(truck_x) + ", trucky = " + std::to_string(truck_y) + ", "+"status = 'idle' WHERE truckid = " + std::to_string(truck_id) + ";";
  std::cout<<"handleIdle"<<std::endl;
  execute(sql,C.get());
  global.disConnectDB(C.get());
}