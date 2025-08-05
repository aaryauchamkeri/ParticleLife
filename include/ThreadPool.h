#pragma once

#include <queue>
#include <thread>
#include <mutex> 
#include <condition_variable>
#include <atomic>
#include <functional>

class ThreadPool { 
private:
    int size;
    std::atomic<int> available_threads;
    std::mutex mutex;
    std::queue<std::function<void()>> tasks;
    std::vector<std::thread> workers;
    std::condition_variable wait_condition;
    std::atomic<bool> run{true};
public:
    ThreadPool(int size);

    void thread_loop();

    void submitTask(std::function<void()> task);

    void stop();

    ~ThreadPool();
};