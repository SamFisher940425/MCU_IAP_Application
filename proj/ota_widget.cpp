#include "ota_widget.h"
#include "ui_ota_widget.h"
#include "QFileDialog"
#include "QFile"
#include "QMessageBox"
#include "QTimer"
#include "QDataStream"

ota_widget::ota_widget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ota_widget)
{
    ui->setupUi(this);

    head = QByteArray::fromHex(ui->headLineEdit->text().toLatin1().data());
    src_id = QByteArray::fromHex(ui->srcID_LineEdit->text().toLatin1().data());
    dst_id = QByteArray::fromHex(ui->dstID_LineEdit->text().toLatin1().data());
    func_code_len = ui->funcCodeLenComboBox->currentIndex() + 1;
    data_len_len = ui->dataLenLenComboBox->currentIndex() + 1;
    tail = QByteArray::fromHex(ui->tailLineEdit->text().toLatin1().data());

    timer0 = new QTimer();
    timer0->stop();
    timer0->setInterval(5);
    connect(timer0,SIGNAL(timeout()),this,SLOT(timer0_task()));
    //失能开始升级按键
    ui->startButton->setEnabled(false);
}

ota_widget::~ota_widget()
{
    delete ui;
}

void ota_widget::iapWidgetShow()
{
    this->show();
}

void ota_widget::DataRecvSlot(QByteArray data_array)
{
    static QByteArray recvBuf;

    recvBuf += data_array;

    if(recvBuf.indexOf(head) != -1)
    {
        int head_pos = recvBuf.indexOf(head);
        if(recvBuf.indexOf(tail,head_pos) != -1)
        {
            uint16_t func_code_temp = 0;
            int tail_pos = recvBuf.indexOf(tail,head_pos);
            int func_code_pos = head_pos + head.size() + src_id.size() + dst_id.size();

            if (func_code_len == 1)
            {
                func_code_temp = recvBuf[func_code_pos];
            }
            else if (func_code_len == 2)
            {
                func_code_temp = recvBuf[func_code_pos+1];
                func_code_temp = func_code_temp << 8;
                func_code_temp |= recvBuf[func_code_pos];
            }

            switch (iap_step)
            {
            case 1:
                if(func_code_temp == 0x10)
                {
                    reply_state = 1;
                }
                break;
            case 3:
                if(func_code_temp == 0x11)
                {
                    reply_state = 1;
                }
                break;
            case 5:
                if(func_code_temp == 0x13)
                {
                    reply_state = recvBuf[func_code_pos + func_code_len + 1];
                }
                break;

            default:
                break;
            }
            QByteArray temp = recvBuf.mid(head_pos,(tail_pos-head_pos)+tail.size());
            textBrowserAppend(temp,0);
            recvBuf.clear();
        }
    }
}

uint16_t ota_widget::Calc_CRC16(char *usPtr, uint32_t usLen)
{
    uint8_t *pucFrame = (uint8_t *)usPtr;
    uint8_t ucCRCHi = 0xFF;
    uint8_t ucCRCLo = 0xFF;
    uint16_t iIndex;
    while (usLen--)
    {
        iIndex = ucCRCLo ^ *(pucFrame++);
        ucCRCLo = (uint8_t)(ucCRCHi ^ aucCRCHi[iIndex]);
        ucCRCHi = aucCRCLo[iIndex];
    }
    return (uint16_t)(ucCRCHi << 8 | ucCRCLo);
}

void ota_widget::timer0_task()
{
    static uint16_t wait_cnt = 0;
    static uint8_t check = 0;
    static int offset = 0;
    static int offset_last = -1;

    ui->startButton->setEnabled(false);

    switch (iap_step) {
    case 0:
        sendBuf.clear();
        sendBuf.append(head);
        sendBuf.append(src_id);
        sendBuf.append(dst_id);
        if (func_code_len == 1)
        {
            sendBuf.append(0x10);
        }
        else if(func_code_len == 2)
        {
            sendBuf.append(0x10);
            sendBuf.append(1,0x00);
        }
        if (data_len_len == 1)
        {
            sendBuf.append(0x04);
        }
        else if (data_len_len == 2)
        {
            sendBuf.append(0x04);
            sendBuf.append(1,0x00);
        }
        fileSize_arrry.resize(4);
        sendBuf.append(fileSize_arrry);
        check = 0;
        for (int i = 0;i < sendBuf.size() ; i++)
        {
         check += sendBuf[i];
        }
        sendBuf.append(check);
        sendBuf.append(tail);
        emit DataSendSignal(sendBuf);
        textBrowserAppend(sendBuf,1);
        reply_state = 0;
        wait_cnt = 0;
        iap_step++;
        break;
    case 1:
        if(reply_state == 0)
        {
            wait_cnt++;
            if (wait_cnt >= 600) // 3s
            {
                wait_cnt = 0;
                iap_step--;
            }
        }
        else
        {
            wait_cnt = 0;
            offset = 0;
            offset_last = -1;
            iap_step++;
        }
        break;
    case 2:
        if (offset > offset_last)
        {
            ui->infoLabel->setText("刷写中...");
            int percent = (int)(offset * 100.0F / fileSize);
            ui->progressBar->setValue(percent);
            sendBuf.clear();
            sendBuf.append(head);
            sendBuf.append(src_id);
            sendBuf.append(dst_id);
            if (func_code_len == 1)
            {
                sendBuf.append(0x11);
            }
            else if(func_code_len == 2)
            {
                sendBuf.append(0x11);
                sendBuf.append(1,0x00);
            }
            if (data_len_len == 1)
            {
                sendBuf.append(0x44);
            }
            else if (data_len_len == 2)
            {
                sendBuf.append(0x44);
                sendBuf.append(1,0x00);
            }
            uint32_t temp_offset = offset;
            QByteArray temp_offset_array;
            QDataStream offsetStream(&temp_offset_array,QIODevice::WriteOnly);
            offsetStream.setByteOrder(QDataStream::LittleEndian);
            offsetStream << temp_offset;
            sendBuf.append(temp_offset_array);
            for (int i = 0; i < 64; i++)
            {
                int pos_temp = offset+i;
                if (pos_temp < fileData.size())
                {
                    sendBuf.append(fileData[pos_temp]);
                }
                else
                {
                    sendBuf.append(0xFF);
                }
            }
            check = 0;
            for (int i = 0;i < sendBuf.size() ; i++)
            {
             check += sendBuf[i];
            }
            sendBuf.append(check);
            sendBuf.append(tail);
            emit DataSendSignal(sendBuf);
            textBrowserAppend(sendBuf,1);
            offset_last = offset;
            reply_state = 0;
            wait_cnt = 0;
            iap_step++;
        }
        else
        {
            emit DataSendSignal(sendBuf);
            textBrowserAppend(sendBuf,1);
            reply_state = 0;
            wait_cnt = 0;
            iap_step++;
        }
        break;
    case 3:
        if(reply_state == 0)
        {
            wait_cnt++;
            if (wait_cnt >= 600) // 3s
            {
                wait_cnt = 0;
                iap_step--;
            }
        }
        else
        {
            wait_cnt = 0;
            offset += 64;
            if(offset < fileData.size())
            {
               iap_step--;
            }
            else
            {
               iap_step++;
            }
        }
        break;
    case 4:
        ui->infoLabel->setText("校验中...");
        sendBuf.clear();
        sendBuf.append(head);
        sendBuf.append(src_id);
        sendBuf.append(dst_id);
        if (func_code_len == 1)
        {
            sendBuf.append(0x13);
        }
        else if(func_code_len == 2)
        {
            sendBuf.append(0x13);
            sendBuf.append(1,0x00);
        }
        if (data_len_len == 1)
        {
            sendBuf.append(0x02);
        }
        else if (data_len_len == 2)
        {
            sendBuf.append(0x02);
            sendBuf.append(1,0x00);
        }
        sendBuf.append(crcResult_array);
        check = 0;
        for (int i = 0;i < sendBuf.size() ; i++)
        {
         check += sendBuf[i];
        }
        sendBuf.append(check);
        sendBuf.append(tail);
        emit DataSendSignal(sendBuf);
        textBrowserAppend(sendBuf,1);
        reply_state = 0;
        wait_cnt = 0;
        iap_step++;
        break;
    case 5:
        if(reply_state == 0)
        {
            wait_cnt++;
            if (wait_cnt >= 600) // 3s
            {
                wait_cnt = 0;
                iap_step--;
            }
        }
        else
        {
            wait_cnt = 0;
            if (reply_state == 3) //升级成功
            {
                ui->infoLabel->setText("升级成功，等待复位");
                ui->textBrowser->append("升级成功，等待复位");
                iap_step++;
            }
            else if (reply_state == 4) //升级失败
            {
                ui->infoLabel->setText("校验无效，升级失败");
                ui->textBrowser->append("校验无效，升级失败");
                iap_step = 255;
                timer0->stop();
            }
        }
        break;
    case 6:
        ui->infoLabel->setText("升级完成，复位中...");
        sendBuf.clear();
        sendBuf.append(head);
        sendBuf.append(src_id);
        sendBuf.append(dst_id);
        if (func_code_len == 1)
        {
            sendBuf.append(0x14);
        }
        else if(func_code_len == 2)
        {
            sendBuf.append(0x14);
            sendBuf.append(1,0x00);
        }
        if (data_len_len == 1)
        {
            sendBuf.append(1,0x00);
        }
        else if (data_len_len == 2)
        {
            sendBuf.append(2,0x00);
        }
        check = 0;
        for (int i = 0;i < sendBuf.size() ; i++)
        {
         check += sendBuf[i];
        }
        sendBuf.append(check);
        sendBuf.append(tail);
        emit DataSendSignal(sendBuf);
        textBrowserAppend(sendBuf,1);
        ui->textBrowser->append("升级完成，复位中...");
        ui->progressBar->setValue(100);
        reply_state = 0;
        wait_cnt = 0;
        iap_step++;
        timer0->stop();
        break;

    default:
        break;
    }
}

void ota_widget::textBrowserAppend(QByteArray array, uint8_t dir)
{
    QString strDis;
    QByteArray hexData = array.toHex().toUpper();
    for(int i = 0;i<hexData.length ();i+=2)//填加空格
    {
        QString st = hexData.mid (i,2);
        strDis += st;
        strDis += "  ";
    }
    if(dir)
    {
        ui->textBrowser->append("tx:"+strDis);
    }
    else
    {
        ui->textBrowser->append("rx:"+strDis);
    }
}

void ota_widget::on_startButton_clicked()
{
    ui->headLineEdit->setEnabled(false);
    ui->srcID_LineEdit->setEnabled(false);
    ui->dstID_LineEdit->setEnabled(false);
    ui->funcCodeLenComboBox->setEnabled(false);
    ui->dataLenLenComboBox->setEnabled(false);
    ui->tailLineEdit->setEnabled(false);

    iap_step = 0;
    reply_state = 0;
    timer0->start();
    ui->infoLabel->setText("开始升级");
}

void ota_widget::on_fileSelectButton_clicked()
{
    binFileName = QFileDialog::getOpenFileName(this,tr("Open File"),"",tr("bin Files (*.bin);;"));

    QFile file(binFileName);
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox messagebox(this);	//弹出提示框
        messagebox.setWindowTitle("提示");
        messagebox.setText("文件打开失败，请重新选择");
        messagebox.addButton(new QPushButton("确定"), QMessageBox::AcceptRole);
        messagebox.exec();  //阻塞GUI线程
        return;
    }

    fileData = file.readAll();
    fileSize = file.size();
    crcResult = Calc_CRC16(fileData.data(),fileSize);
    file.close();

    QDataStream sizeStream(&fileSize_arrry,QIODevice::WriteOnly);
    sizeStream.setByteOrder(QDataStream::LittleEndian);
    sizeStream << fileSize;

    QDataStream crcStream(&crcResult_array,QIODevice::WriteOnly);
    crcStream.setByteOrder(QDataStream::LittleEndian);
    crcStream << crcResult;

    ui->fileNameLineEdit->setText(binFileName);
    ui->fileSizeLabel->setText(QString::number(fileSize,16).toUpper());
    ui->crcResultLabel->setText(QString::number(crcResult,16).toUpper());
    //使能开始升级按键
    ui->startButton->setEnabled(true);
}


void ota_widget::on_headLineEdit_textChanged(const QString &arg1)
{
    head = QByteArray::fromHex(arg1.toLatin1().data());
}


void ota_widget::on_srcID_LineEdit_textChanged(const QString &arg1)
{
    src_id = QByteArray::fromHex(arg1.toLatin1().data());
}


void ota_widget::on_dstID_LineEdit_textChanged(const QString &arg1)
{
    dst_id = QByteArray::fromHex(arg1.toLatin1().data());
}


void ota_widget::on_funcCodeLenComboBox_currentIndexChanged(int index)
{
    func_code_len = index + 1;
}


void ota_widget::on_dataLenLenComboBox_currentIndexChanged(int index)
{
    data_len_len = index + 1;
}


void ota_widget::on_tailLineEdit_textChanged(const QString &arg1)
{
    tail = QByteArray::fromHex(arg1.toLatin1().data());
}


void ota_widget::on_buttonBox_clicked(QAbstractButton *button)
{
    timer0->stop();
    iap_step = 0;
    reply_state = 0;
    ui->infoLabel->setText("请设定IAP参数");
    ui->startButton->setEnabled(false);

    fileData.clear();
    fileSize = 0;
    fileSize_arrry.clear();
    crcResult = 0;
    crcResult_array.clear();
    sendBuf.clear();

    ui->fileNameLineEdit->clear();
    binFileName.clear();
    ui->fileSizeLabel->setText("00000000");
    ui->crcResultLabel->setText("0000");
    ui->textBrowser->clear();

    ui->headLineEdit->setEnabled(true);
    ui->srcID_LineEdit->setEnabled(true);
    ui->dstID_LineEdit->setEnabled(true);
    ui->funcCodeLenComboBox->setEnabled(true);
    ui->dataLenLenComboBox->setEnabled(true);
    ui->tailLineEdit->setEnabled(true);
    ui->progressBar->setValue(0);
}

