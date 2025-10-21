#include <QSerialPort>
#include <QtDebug>
#include <QtSerialPort/QSerialPortInfo>
#include <QTextCodec>
#include <QTimer>
#include <QMessageBox>
#include <QPainter>
#include <QBitmap>
#include <QBrush>
#include "compilerport.h"
#include "serial.h"
#include "ota_widget.h"
#include "ui_serial.h"

CompilerPort::CompilerPort()
{

}
//冒泡法，COM从大到小排列
QStringList CompilerPort::compiler_port(QStringList staticList,int len)
{
    int i,j;
    QString temp = "";
    for(i = 0;i < len-1 ; i++)
    {
        for(j = 0;j < len-1-i; j++)
        {
            if((staticList[j].compare(staticList[j+1]))>0)
            {
                temp = staticList[j];
                staticList[j] = staticList[j+1];
                staticList[j+1] = temp;
            }
        }
    }
    return staticList;
}
