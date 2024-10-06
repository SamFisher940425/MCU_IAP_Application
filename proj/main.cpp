#include "serial.h"
#include "ota_widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Serial serial_w;
    ota_widget ota_w;
    QObject::connect(&serial_w,&Serial::iapWidgetOpen,&ota_w,&ota_widget::iapWidgetShow);
    QObject::connect(&ota_w,&ota_widget::DataSendSignal,&serial_w,&Serial::DataSendSlot);
    QObject::connect(&serial_w,&Serial::DataRecvSignal,&ota_w,&ota_widget::DataRecvSlot);
    serial_w.show();
    return a.exec();
}
