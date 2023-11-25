//
// Created by zrk on 2023/10/20.
//

#include "tick.h"
#include <iostream>
#include <utility>
//#include <ctime>


void Tick::Start(std::string contextName) {
    context = std::move(contextName);
    _start = std::chrono::high_resolution_clock::now();
//     _start = clock();
}

void Tick::End(double scale)
{
    using namespace std::chrono;
    auto end = high_resolution_clock::now();
    auto delta = duration_cast<nanoseconds> (end - _start);
    auto deltaTime = delta.count() * scale;
//    _end = clock();

//    auto deltaTime = _end - _start;
//    deltaTime = (long)((double)deltaTime * scale);

//    std::cout << "  " << (double)_start / CLOCKS_PER_SEC << "s" << std::endl;
//    std::cout << "  " << (double)_end / CLOCKS_PER_SEC << "s" << std::endl;
//    std::cout << "  " << (double)deltaTime / CLOCKS_PER_SEC << "s" << std::endl;
    if (!context.empty()) {
        std::cout << ">>>>>> " << context << std::endl;
    }
    std::cout << "  " << (double)deltaTime / 1000000000.0 << "s" << std::endl;
    if (!context.empty()) {
        std::cout << "<<<<<<  " << std::endl << std::endl;
    }
}
