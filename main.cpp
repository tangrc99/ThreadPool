
#include "ThreadPool.h"
#include <iostream>

std::string nums(int n){
    return std::to_string(n)+'2';
}

int main() {
    ThreadPool threadPool(2);
    auto task1 = threadPool.addTask(std::bind(nums,1));
    auto task2 = threadPool.addTask(std::bind(nums,2));
    auto task3 = threadPool.addTask(std::bind(nums,3));
    std::cout<<task1->getResult();
    std::cout<<task2->getResult();
    std::cout<<task3->getResult();


    return 0;
}
