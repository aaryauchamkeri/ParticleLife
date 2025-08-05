#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include "ThreadPool.h"

ThreadPool::ThreadPool(int size) { 
    this->size = size;
    this->available_threads = size;
    workers = std::vector<std::thread>(size);
    for(int i = 0; i < size; ++i) {
        workers[i] = std::thread(&ThreadPool::thread_loop, this);
    }
}

void ThreadPool::thread_loop() {
    while(run) {
        std::unique_lock<std::mutex> lock(mutex);
        wait_condition.wait(lock, [this]{return !tasks.empty() || !run;});
        if(tasks.size()) {
            auto task = tasks.front();
            tasks.pop();
            lock.unlock();
            available_threads--;
            task();
            available_threads++;    
        }
        if(!run) return;
    }
}

void ThreadPool::submitTask(std::function<void()> task) {
    std::unique_lock<std::mutex> lock(mutex);
    tasks.push(task);
    lock.unlock();
    wait_condition.notify_one();    
}

void ThreadPool::stop() {
    run = false;
    wait_condition.notify_all();
    for(auto& worker : workers) {
        if(worker.joinable()) {
            worker.join();
        }
    }
}

ThreadPool::~ThreadPool() {
    stop();
}