#include "datalog.hpp"
#include <boost/asio.hpp>
#include "crc32_mpeg.hpp"
#include <thread>
using namespace std::chrono_literals;
DataLogUSB::DataLogUSB(std::string device_repo, int baudrate) : serialPort(io, device_repo), msgDetectStage(0), ifNewMessage(0), curDatalogTask(DATALOG_TASK_FREE), dataSlotLen(0)
{
    if (!DataLogUSB::serialPort.is_open())
    {
        try
        {
            DataLogUSB::serialPort.open(device_repo);
        }
        catch (...) // Problem: Cannot catch exception when USB is disconnected
        {
            std::cout << "Fail to connect to datalog serial port." << std::endl;
        }
    }
    if (DataLogUSB::serialPort.is_open())
    {
        DataLogUSB::serialPort.set_option(boost::asio::serial_port_base::baud_rate(baudrate));
        DataLogUSB::serialPort.set_option(boost::asio::serial_port::flow_control());
        DataLogUSB::serialPort.set_option(boost::asio::serial_port::parity());
        DataLogUSB::serialPort.set_option(boost::asio::serial_port::stop_bits());
        DataLogUSB::serialPort.set_option(boost::asio::serial_port::character_size(8));
        std::cout << "successfully connected to datalog serial port." << std::endl;
    }
    else
    {
        std::cout << "Fail to connect to datalog serial port." << std::endl;
    }
}

void DataLogUSB::ReceiveCargo(void)
{
    if (msgDetectStage < 3)
    {
        if (boost::asio::read(DataLogUSB::serialPort, boost::asio::buffer(DataLogUSB::tempRx, 1)))
        {
            if (msgDetectStage == 0)
            {
                if (tempRx[0] == 0xAA)
                    msgDetectStage = 1;
            }
            else if (msgDetectStage == 1)
            {
                if (tempRx[0] == 0xCC)
                    msgDetectStage = 2;
                else
                    msgDetectStage = 0;
            }
            else if (msgDetectStage == 2)
            {
                bytesToRead = tempRx[0];
                msgDetectStage = 3;
            }
        }
    }
    else if (msgDetectStage == 3)
    {
        if (boost::asio::read(DataLogUSB::serialPort, boost::asio::buffer(DataLogUSB::tempRx, bytesToRead + 5)))
        {
            // std::cout << "Reached crc check stage." << std::endl;
            uint32_t crcCalculate[bytesToRead + 1];
            crcCalculate[0] = (uint32_t)bytesToRead;
            // std::cout << std::hex << (int)crcCalculate[0] << std::endl;////////////////////
            for (uint8_t i = 0; i <= bytesToRead - 1; i++)
            {
                crcCalculate[i + 1] = (uint32_t)tempRx[i];
                // std::cout << std::hex << (int)crcCalculate[i + 1] << std::endl;////////////////////
            }
            uint32_t crcResult = CRC32_32BitsInput(crcCalculate, bytesToRead + 1);
            // std::cout << "CRC calculated result: " << std::hex << (unsigned int)crcResult << std::endl;
            uint32_t crcFromMFEC = tempRx[bytesToRead] | tempRx[bytesToRead + 1] << 8 | tempRx[bytesToRead + 2] << 16 | tempRx[bytesToRead + 3] << 24;

            if (crcResult == crcFromMFEC && tempRx[bytesToRead + 4] == 0x55)
            {
                msgDetectStage = 0;

                memcpy(rxMessageCfrm, tempRx, bytesToRead);
                rxMessageLen = bytesToRead;
                ifNewMessage = 1;

                // std::cout<<"Cargo Received!"<<std::endl;
                // std::cout<<"Message is :" << std::endl;
                // for (uint8_t i = 0; i <= bytesToRead - 1; i++)
                // {
                //     std::cout.fill('0');
                //     std::cout.width(2);
                //     std::cout << std::hex<< (int)rxMessageCfrm[i] << " ";
                // }
                // std::cout << std::endl;
            }
            else
            {
                std::cout << "Invalid Cargo!" << std::endl;
            }

            // std::cout << "CRC real result: " << std::hex << (unsigned int)crcFromMFEC << std::endl;
        }
    }
}

void DataLogUSB::TransmitCargo(uint8_t *data, uint8_t len)
{
    uint8_t txBuf[len + 8];
    txBuf[0] = 0xBB;
    txBuf[1] = 0xCC;
    txBuf[2] = len;
    memcpy(&txBuf[3], data, len);
    uint32_t crcCalculate[len + 1];
    crcCalculate[0] = (uint32_t)len;
    for (uint8_t i = 1; i <= len; i++)
    {
        crcCalculate[i] = (uint32_t) * (data + i - 1);
    }
    uint32_t crc = CRC32_32BitsInput(crcCalculate, len + 1);
    // std::cout<<"tx crc is: "<<std::hex<<(unsigned int)crc<<std::endl;
    txBuf[len + 3] = (uint8_t)(crc & 0x000000FF);
    txBuf[len + 4] = (uint8_t)(crc >> 8 & 0x000000FF);
    txBuf[len + 5] = (uint8_t)(crc >> 16 & 0x000000FF);
    txBuf[len + 6] = (uint8_t)(crc >> 24 & 0x000000FF);
    txBuf[len + 7] = 0x88;

    // for (uint8_t i = 0; i <= sizeof(txBuf) - 1; i++)
    // {
    //     std::cout<<std::hex<<(unsigned int)txBuf[i]<<" ";
    // }
    // std::cout<<std::endl;
    DataLogUSB::serialPort.write_some(boost::asio::buffer(txBuf, len + 8));
}

void DataLogUSB::SendText(std::string text)
{
    TransmitCargo((uint8_t *)text.data(), text.length());
}

void DataLogUSB::StartDataLog(std::string filename)
{
    SendText("Datalog start");
    curDatalogTask = DATALOG_TASK_RECEIVE_DATA_SLOT_LEN;
    fileStream.open(filename.data());
    fileStream << "Index," << "Time (ms),";
}

void DataLogUSB::DataLogManager(void)
{
    if (ifNewMessage)
    {
        std::string msg((const char*)rxMessageCfrm, rxMessageLen);
        if (curDatalogTask == DATALOG_TASK_RECEIVE_DATA_SLOT_LEN)
        {
            if (rxMessageLen <= 2)
            {
                dataSlotLen = atoi(msg.data());
                dataSlotLabellingCount = dataSlotLen;
                curDatalogTask = DATALOG_TASK_RECEIVE_DATA_SLOT_MSG;
                SendText("Roger that");
                std::cout<<"Received Data Slot Length: "<<std::dec<<(unsigned int)dataSlotLen<<std::endl;
            }
        }
        else if (curDatalogTask == DATALOG_TASK_RECEIVE_DATA_SLOT_MSG)
        {
            fileStream << msg.data() << ",";
            dataSlotLabellingCount--;
            std::cout<<"Received dataslot label: " << msg.data() << std::endl;
            std::cout<<"dataSlotLabellingCount is: "<<std::dec<<(unsigned int)dataSlotLabellingCount<<std::endl;
            if (dataSlotLabellingCount == 0)
            {
                std::this_thread::sleep_for(500ms);
                SendText("Roger that");
                fileStream << std::endl;
                curDatalogTask = DATALOG_TASK_DATALOG;
            }
        }
        else if (curDatalogTask == DATALOG_TASK_DATALOG)
        {
            std::cout<<"reached datalog"<<std::endl;
            memcpy(&index, rxMessageCfrm, 4);
            memcpy(&systemTime, rxMessageCfrm + 4, 4);
            fileStream << index << "," << systemTime << ",";
            for (uint8_t i = 1 ; i <= dataSlotLen; i++)
            {
                float data = 0.0f;
                memcpy(&data, rxMessageCfrm + 4 * (i + 1), 4);
                fileStream << data << ",";
                std::cout << "get data: "<< data<<std::endl;
            }
            fileStream << std::endl;
        }
        else if (curDatalogTask == DATALOG_TASK_END)
        {
        }
    }
    ifNewMessage = 0;
}
