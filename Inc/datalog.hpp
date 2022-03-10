#ifndef DATALOG_HPP
#define DATALOG_HPP
#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <fstream>

enum DataLogTask
{
    DATALOG_TASK_FREE,
    DATALOG_TASK_START,
    DATALOG_TASK_RECEIVE_DATA_SLOT_LEN,
    DATALOG_TASK_RECEIVE_DATA_SLOT_MSG,
    DATALOG_TASK_DATALOG,
    DATALOG_TASK_END
};

union FloatUInt8
{
    float   f;
    uint8_t b8[4];
};

class DataLogUSB
{
    public:
            boost::asio::io_service io;
            boost::asio::serial_port serialPort;
            DataLogUSB(std::string device_repo, int baudrate);
            void ReceiveCargo(void);
            /*Rx message*/
            uint8_t ifNewMessage;
            uint8_t tempRx[264];
            uint8_t msgDetectStage;
            uint8_t bytesToRead;
            uint8_t rxMessageCfrm[256];
            uint8_t rxMessageLen;
            /*Tx message*/
            void TransmitCargo(uint8_t *data, uint8_t len);
            void SendText(std::string text);
            void StartDataLog(std::string filename);
            /*Datalog control*/
            enum DataLogTask curDatalogTask;
            void DataLogManager(void);
            uint8_t dataSlotLen;
            uint8_t dataSlotLabellingCount;
            std::ofstream fileStream;
            /*Received MCU Values*/
            uint32_t    systemTime, index;
    private:

};

#endif
