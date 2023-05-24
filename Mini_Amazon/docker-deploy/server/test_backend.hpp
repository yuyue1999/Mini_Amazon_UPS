#ifndef TEST_BACKEND_HPP
#define TEST_BACKEND_HPP
#include "common.hpp"
#include "world_handle.hpp"

int test_send_Apurchasemore(){

    std::cout<<"Amazon: testing send order to world"<<std::endl;
    std::vector<AProduct> products;
    AProduct aproduct;
    aproduct.set_id(1);
    aproduct.set_description("apple");
    aproduct.set_count(10);
    products.push_back(aproduct);
    AProduct aproduct2;
    aproduct2.set_id(2);
    aproduct2.set_description("banana");
    aproduct2.set_count(20);
    products.push_back(aproduct2);
    send_ApurchaseMore_to_world(1,products,1,"test",1,1);
    return 0;
}

int test_send_AUInitPickUP_to_UPS(){
    std::cout<<"Amazon: testing send AUInitPickUP to UPS"<<std::endl;
    google::protobuf::RepeatedPtrField<AProduct> products;
    AProduct aproduct;
    aproduct.set_id(1);
    aproduct.set_description("apple");
    aproduct.set_count(10);
    products.Add()->CopyFrom(aproduct);

    AProduct aproduct2;
    aproduct2.set_id(2);
    aproduct2.set_description("banana");
    aproduct2.set_count(20);
    products.Add()->CopyFrom(aproduct2);

    AUDeliveryLocation audeliverylocation;
    audeliverylocation.set_x(1);
    audeliverylocation.set_y(1);
    audeliverylocation.set_packageid(1);

    Send_AUInitPickUP_to_UPS(1,"test",audeliverylocation,products);
    return 0;
}



#endif