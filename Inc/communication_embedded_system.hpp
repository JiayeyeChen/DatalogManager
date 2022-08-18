#ifndef COMMUNICATION_EMBEDDED_SYSTEM_HPP
#define COMMUNICATION_EMBEDDED_SYSTEM_HPP

#include <boost/asio.hpp>
#include "crc32_mpeg.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include "main.hpp"

enum DataLogTask
{
    DATALOG_TASK_FREE,
    DATALOG_TASK_START,
    DATALOG_TASK_RECEIVE_DATA_SLOT_LEN,
    DATALOG_TASK_RECEIVE_DATA_SLOT_MSG,
    DATALOG_TASK_DATALOG,
    DATALOG_TASK_END
};

class USBCommunicationHandle
{
    public:
            boost::asio::io_service io;
            boost::asio::serial_port serialPort;
            USBCommunicationHandle(std::string device_repo, int baudrate, std::string file_name);
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
            /*Datalog control*/
            enum DataLogTask curDatalogTask;
            void DataLogManager(void);
            uint8_t dataSlotLen;
            uint8_t dataSlotLabellingCount;
            uint8_t ifDatalogStarted;
            std::ofstream fileStream;
            std::string curDatalogFilename;
            void StartDataLogActive(std::string filename);
            void StartDataLogPassive(std::string filename);
            bool ifNewMsgIsThisString(std::string str);
            std::string fileName;
            /*Received MCU Values*/
            uint32_t    systemTime, index;
    private:

};


#endif
