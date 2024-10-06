#include "serial.h"
#include "ota_widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Serial serial_w;
    ota_widget ota_w;
    serial_w.show();
    ota_w.show();
    return a.exec();
}
