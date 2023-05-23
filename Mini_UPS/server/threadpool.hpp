#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP
#include <functional>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

class threadpool
{
private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()> > m_tasks;
    int max_threads;
    int max_tasks;
    std::mutex m_lock;
    std::condition_variable has_task;
    bool run_flag;
public:
    threadpool(int max_thds=100, int max_tasks=500):max_threads(max_thds),max_tasks(max_tasks),run_flag(true){
        if (threads.size()==0){
            std::unique_lock<std::mutex> lck(m_lock);
            if (threads.size()==0){
                init_threads();
            }
        }
    }

    threadpool * get_pool(){
        return this;
    }
    bool assign_task(std::function<void()> task){
        {
            std::unique_lock<std::mutex> lck(m_lock);
            if (m_tasks.size()<(unsigned int)max_tasks){
                m_tasks.push(task);
            }
            else{
                return false;
            }
            has_task.notify_one();
        }
        return true;
    }
    ~threadpool(){
        {
        std::unique_lock<std::mutex> ul(m_lock);
        run_flag=false;
        }
        has_task.notify_all();
        for (auto &t:threads){
            t.join();
        }
        threads.clear();
    }
private:
    bool init_threads(){
        for (int i=0;i<max_threads;i++){
            threads.push_back(std::thread(&threadpool::run_task,this));
        }
        return true;
    }
    void run_task(){
        while(run_flag==true||!m_tasks.empty()){
            std::function<void()> task;
            std::unique_lock<std::mutex> lck(m_lock);
            if (run_flag == false && m_tasks.empty()) {
                return;
            }
            while(m_tasks.empty() && run_flag == true){
                has_task.wait(lck);
            }
            if (m_tasks.empty()) {
                return;
            }
            task=m_tasks.front();
            m_tasks.pop();
            lck.unlock();
            task();
        }
    }



};
#endif