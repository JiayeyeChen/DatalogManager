#include <iostream>
#include <boost/asio.hpp>
#include <main.hpp>
#include "datalog.hpp"
#include "crc32_mpeg.hpp"
#include <chrono>
#include <thread>
#include <fstream>

int main(int argc, char **argv)
{
    using namespace std::chrono_literals;
    // std::ofstream fileStream;
    
    DataLogUSB datalogUSB(argv[1], 921600);

    uint8_t ifDatalogStarted = 0;

    datalogUSB.StartDataLog(argv[2]);
    if (std::cin.get() == '\n');
    while (1)
    {
        datalogUSB.ReceiveCargo();

        datalogUSB.DataLogManager();

        // std::this_thread::sleep_for(1000ms);
    }
}
