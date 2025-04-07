#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>

using namespace std;

// mutex to output 
mutex coutMutex;

void testFunction(const string& threadName) {
    // local variable:
    // initialized each time when function is called
    int normalVar = 0;
    
    // static variable:
    // initialized only once when the program starts
    static int staticVar = 0;
    
    // thread_local variable:
    // initialized once for each thread: each thread has its own copy
    thread_local int tlVar = 0;
    
    // static thread_local variable:
    // same to thread_local
    static thread_local int stlVar = 0;
    
    normalVar++;
    staticVar++;
    tlVar++;
    stlVar++;

    // use lock_guard to lock the mutex
    lock_guard<mutex> lock(coutMutex);
    cout << "Thread " << threadName << " @[" << this_thread::get_id() << "] : "
         << "normalVar = " << normalVar 
         << ", staticVar = " << staticVar 
         << ", thread_local var = " << tlVar 
         << ", static thread_local var = " << stlVar 
         << endl;
}

void worker(const string& threadName) {
    for (int i = 0; i < 5; i++) {
        testFunction(threadName);
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

int test_cpp_var() {
    thread t1(worker, "t1");
    thread t2(worker, "t2");

    t1.join();
    t2.join();

    return 0;
}
