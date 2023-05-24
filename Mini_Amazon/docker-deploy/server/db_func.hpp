#ifndef __DB_FUNC_H__
#define __DB_FUNC_H__

#include<iostream>
#include<string>
#include<pqxx/pqxx>
#include<fstream>
#include<chrono>
#include<ctime>
#include<assert.h>
#include<mutex>
#include<pqxx/pqxx>

typedef long long ll;


inline pqxx::connection * connectDB() {
    pqxx::connection *C;
    try{
        //C = new pqxx::connection("dbname=db user=postgres password=passw0rd hostaddr=127.0.0.1 port=5432");
        C = new pqxx::connection("dbname=db user=postgres password=passw0rd host=db port=5432");    
        if(C->is_open()){
            std::cout<<"Connected to database successfully: "<< C->dbname()<<std::endl;
        }else{
            std::cerr<<"Can't open database"<<std::endl;
            exit(EXIT_FAILURE);
        }
    }catch(const std::exception &e){
        std::cerr<<e.what()<<std::endl;
        exit(EXIT_FAILURE);
    }
    return C;
}

inline void Update_Order_Status(long long order_id, std::string status){
    std::stringstream command;
    pqxx::connection *C = connectDB();
    pqxx::work W(*C);
    try {
        command<<"SELECT * FROM orders_order WHERE id = "<<W.quote(order_id)<<" FOR UPDATE;";
        W.exec(command.str());
        command.str(""); // Clear the stringstream for the next command

        command<<"UPDATE orders_order SET status = "<< W.quote(status)
        <<" WHERE id = "<<W.quote(order_id)<<";";
        W.exec(command.str());
        W.commit();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        W.abort();
        return;
    }
    std::cout<<"Amazon: order "<<order_id<<" status updated to "<<status<<std::endl;

    if(C->is_open()){
        C->disconnect();
        std::cout << "Closed database successfully" << std::endl;
    }

}


#endif