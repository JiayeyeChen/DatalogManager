#include <main.hpp>

bool ifExit = false;

void USB_RxCargo(USBCommunicationHandle* husbcom)
{
    while(1)
    {
        husbcom->ReceiveCargo();
    }
}

void Keyboard(void)
{
    std::cin.get();
    ifExit = true;
}
//argv[1]: USB device address. argv[2]: Datalog filename.
int main(int argc, char** argv)
{
    USBCommunicationHandle hUSBCom(argv[1], 921600, argv[2]);
    std::thread Thread_USB_RxCargo(USB_RxCargo, &hUSBCom);
    std::thread Thread_KeyboardInput(Keyboard);
    while(1)
    {
        hUSBCom.DataLogManager();


        if(ifExit)
            break;
    }
    return 0;
}
