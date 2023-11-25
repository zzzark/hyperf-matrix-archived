//
// Created by zrk on 2023/10/20.
//

#ifndef DOP_TICK_H
#define DOP_TICK_H
#include <chrono>
#include <string>
//#include <ctime>


class Tick
{
private:
    std::string context;

//    clock_t _start;
//    clock_t _end;
    std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> _start;
//    std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> _end;
public:
    void Start(std::string contextName="");
    void End(double scale=1.0);
};


#endif //DOP_TICK_H
