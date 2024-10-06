/*------------------------------------
 *          user
 *------------------------------------*/
#include <include.h>

Serial::Serial(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Serial)
{
    ui->setupUi(this);
    //用户添加
    Serial::setWindowTitle(tr("MCU_IAP_Assistant"));
    Timer0_Init();
    Timer1_Init();
    systemInit();
}
Serial::~Serial()
{
    delete ui;
}
/*----------------------------------------------------------
 *          function
 *----------------------------------------------------------*/
void Serial::systemInit()
{
    globlePort.setParity(QSerialPort::NoParity);
    globlePort.setDataBits(QSerialPort::Data8);
    globlePort.setStopBits(QSerialPort::OneStop);
    //端口设定
    ui->baudRateBox->setCurrentIndex(4);
    ui->sendTxtButton->setEnabled(false);
    ui->iapButton->setEnabled(false);
    //信号绑定到槽
    connect(ui->openButton,&QPushButton::clicked,this,&Serial::ButtonOpenPort);//打开串口信号
    connect(ui->sendTxtButton,&QPushButton::clicked,this,&Serial::ButtonSendPort);//发送文本信号
    connect(ui->pushButton_3,&QPushButton::clicked,this,&Serial::ButtonStopShow);//暂停显示文本信号
    connect(&globlePort,&QSerialPort::readyRead,this,&Serial::ReciveDate);//串口打开就接收消息
    connect(ui->pushButton_2,&QPushButton::clicked,this,&Serial::ButtonClear);//清空文本框信号
    connect(ui->checkBox,&QCheckBox::stateChanged,this,&Serial::AutoClear);//自动清除

    connect(&globlePort, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),  this, &Serial::handleSerialError);
}
void Serial::handleSerialError(QSerialPort::SerialPortError error)
{
    qDebug()<<"发生错误";
    if (error == QSerialPort::ResourceError) {
        //QMessageBox::critical(this, tr("Error"), "串口连接中断，请检查是否正确连接！");
        //my_SerialPort->close();
        //ui->label_status->setText("未连接");
        //m_pTimer->start(1000);

    }
}
void Serial::on_textBrowser_textChanged()
{
   //当文本框到底的时候自动下滑
   ui->textBrowser->moveCursor(QTextCursor::End);
}
/*--------------------------
 *      手动清除文本
 * ------------------------*/
void Serial::ButtonClear()

{
   ui->textBrowser->clear();
}
/*--------------------------
 *      暂停、开始显示文本
 * ------------------------*/
void Serial::ButtonStopShow()
{
    if(ui->pushButton_3->text() == QString("暂停显示"))
    {
        ui->pushButton_3->setText(QString("开始显示"));
    }
    else if(ui->pushButton_3->text() == QString("开始显示"))
    {
        ui->pushButton_3->setText(QString("暂停显示"));
    }
}
/*--------------------------
 *      自动清理
 * ------------------------*/
void Serial::AutoClear()
{
    if(ui->checkBox->isChecked()==true)
    {
        myTimer1->start();

    }
    else if(ui->checkBox->isChecked()==false)
    {
        myTimer1->stop();
    }
}
/*--------------------------
 *      供外部调用的发送函数
 * ------------------------*/
void Serial::DataSendSlot(QByteArray data_array)
{
    globlePort.write(data_array);
}
/*----------------------------------------------------------
 *          定时器
 *----------------------------------------------------------*/
void Serial::Timer0_Init()
{
    myTimer0 = new QTimer();
    myTimer0->stop();
    myTimer0->setInterval(1000);//1ms定时器，类似于单片机的中断函数
    connect(myTimer0,SIGNAL(timeout()),this,SLOT(Timer0_Task()));
    //当记满1000时执行Timer0_Task
    myTimer0->start();
}
void Serial::Timer0_Task()
{
    Read_Serial_Connect_Success();
}
void Serial::Timer1_Init()
{
    myTimer1 = new QTimer();
    myTimer1->stop();
    myTimer1->setInterval(4000);//1ms定时器，类似于单片机的中断函数
    connect(myTimer1,SIGNAL(timeout()),this,SLOT(Timer1_Task()));
    //当记满100时执行Timer1_Task
}
void Serial::Timer1_Task()
{    
    qDebug()<<"定时器1";
    ui->textBrowser->clear();
}
/*----------------------------------------------------------
 *          COM自动识别
 *----------------------------------------------------------*/
static int Read_Serial_Port_Info()
{

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        qDebug() << "Name : " << info.portName();
        qDebug() << "Description :"<<info.description();
        if(info.portName() != NULL)
            return true;
        else
            return false;
    }
    return false;
}
void Serial::Read_Serial_Connect_Success()
{
    qDebug() << "寻找串口 返回值："<<Read_Serial_Port_Info() << endl;
    if(Read_Serial_Port_Info())
    {
        Serial_Connect_Success_Label_Text();
    }
    else
        Serial_Connect_NG_Label_Text();
}
void Serial::Serial_Connect_Success_Label_Text()
{
    serialStrList.clear();
    serialStrList = scanSerial();
    for(int z = 0;z < serialStrList.size();z++)
    {
        qDebug()<<"链表1为"<<serialStrList[z];
    }
    qDebug()<<"size:"<<serialStrList.size();
    //如果有新增的COM口，则scanSerial()会变为："COMx",这样将以前的字符串添加在新增的前面即可
    for(int i = 0;i<staticList.size();i++)
    {
        serialStrList.insert(i, staticList[i]);
    }
    if(staticList != serialStrList)
    {
        for(int q = 0;q < serialStrList.size();q++)
            qDebug()<<"链表2为"<<serialStrList[q];
        staticList = serialStrList;
        staticList = compilerport.compiler_port(staticList,staticList.size());
        for(int c = 0;c < staticList.size();c++)
            qDebug()<<"链表3为"<<staticList[c];
        ui->portBox->clear();
        for (int i=0; i<staticList.size(); i++)
        {
            if(staticList[i] != "")
            ui->portBox->addItem(staticList[i]); // 在comboBox那添加串口号
        }
    }
    QFont font ( "已发现", 12, 70); //第一个属性是字体（微软雅黑），第二个是大小，第三个是加粗（权重是75）
    ui->label_1->setFont(font);
    QPalette pe;
    pe.setColor(QPalette::WindowText,Qt::darkGreen);
    ui->label_1->setPalette(pe);
    ui->label_1->setText("已发现");
}
void Serial::Serial_Connect_NG_Label_Text()
{
    ui->portBox->clear();//当所有串口都拔出是清空
    staticList.clear();
    QFont font ( "搜索中", 12, 70); //第一个属性是字体（微软雅黑），第二个是大小，第三个是加粗（权重是75）
    ui->label_1->setFont(font);
    QPalette pe;
    pe.setColor(QPalette::WindowText,Qt::red);
    ui->label_1->setPalette(pe);
    ui->label_1->setText("搜索中");
}
/*----------------------------------------------------------
 *          判定串口是否重复
 *----------------------------------------------------------*/
QStringList Serial::scanSerial()
{
    //读取串口信息
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        int judge = 0;//标志位；
        //1.利用portBox->count()统计portBox里标签的数量
        //2.利用for(i=0;count;)循环，以此判定标签里有没有重复
        //3.若没有重复则 标志位为0，否则为1.只有为0才会append()
        globlePort.setPort(info);
        for(int i=0;i<ui->portBox->count();i++)
        {
            if(globlePort.portName() == ui->portBox->itemText(i))
            {
                judge++;
            }
//            qDebug()<<judge<<"key测试";
        }
        if(judge == 0)
        {
            serialStrList.append(globlePort.portName());
        }     
    }
    return serialStrList;
}
/*----------------------------------------------------------
 *          十六进制输入合法检查
 *----------------------------------------------------------*/
void Serial::validateHEX()
{
    QString text = ui->textEdit->toPlainText();		//获取QTextEdit中的纯文本
    QTextCursor cur = ui->textEdit->textCursor();	//获取光标，默认获取的光标在最后
    int pos = cur.position() - 1;	//QString索引从0开始，因此减1是为了获取最后一个位置的字符的索引
    if(pos >= 0)	//pos为-1说明当前QTextEdit中没有文本
    {
//        qDebug() << text.at(pos);
        QChar ch = text.at(pos);
        if((ch >= '0' && ch <= '9')		//判断是否符合条件
                || (ch >= 'A' && ch <= 'F')
                || (ch >= 'a' && ch <= 'f')
                || (ch == ' '))
        {
            return;
        }
        else
        {
            cur.deletePreviousChar();   //删除光标所在位置的前一个字符
            QMessageBox messagebox(this);	//弹出提示框
            messagebox.setWindowTitle("提示");
            messagebox.setText("请输入16进制0-9,，A-F!\n按照AE 0D 3F 93的格式输入");
            messagebox.addButton(new QPushButton("确定"), QMessageBox::AcceptRole);
            messagebox.exec();  //阻塞GUI线程
        }
    }
}
/*----------------------------------------------------------
 *          slots
 *----------------------------------------------------------*/
void Serial::ButtonOpenPort(bool)
{
    if(ui->openButton->text() == QString("打开串口"))  //串口未打开
        {
            //设置端口号
            globlePort.setPortName(ui->portBox->currentText());
            //设置波特率
            globlePort.setBaudRate(ui->baudRateBox->currentText().toInt());
            //设置数据位
            qDebug()<<"数据位"<<ui->DateBitsBox->currentText().toInt();
            switch (ui->DateBitsBox->currentText().toInt())
            {             
                case 8: globlePort.setDataBits(QSerialPort::Data8); break;
                case 7: globlePort.setDataBits(QSerialPort::Data7); break;
                case 6: globlePort.setDataBits(QSerialPort::Data6); break;
                case 5: globlePort.setDataBits(QSerialPort::Data5); break;
                default: break;
            }
            //设置停止位
            qDebug()<<"停止位"<<ui->stopBitsBox->currentText().toInt();
            switch (ui->stopBitsBox->currentText().toInt())
            {

                case 1: globlePort.setStopBits(QSerialPort::OneStop);break;
                case 2: globlePort.setStopBits(QSerialPort::TwoStop);break;
                default:break;
            }
            //设置校验方式
            switch (ui->ParityBox->currentIndex())
            {
                case 0: globlePort.setParity(QSerialPort::NoParity);break;
                default:break;
            }
            //设置流控制模式
            globlePort.setFlowControl(QSerialPort::NoFlowControl);
            //打开串口出现错误
            if(globlePort.open(QIODevice::ReadWrite)==false)
            {
                qDebug()<<"出现问题"<<ui->portBox->currentText();
                for(int i = 0;i < ui->portBox->count() ;i++)
                {
                    if( ui->portBox->itemText(i) == ui->portBox->currentText())
                    {
                        ui->portBox->removeItem(i);
                        staticList[i].remove(0,4);
                    }

                }

                QMessageBox::warning(NULL , "提示", "串口打开失败！");
                return;
            }
            // 失能串口设置控件
            ui->portBox->setEnabled(false);
            ui->ParityBox->setEnabled(false);
            ui->baudRateBox->setEnabled(false);
            ui->DateBitsBox->setEnabled(false);
            ui->stopBitsBox->setEnabled(false);
            //ui->search_Button->setEnabled(false);
            //使能发送按钮
            ui->sendTxtButton->setEnabled(true);
            ui->iapButton->setEnabled(true);
            //调整串口控制按钮的文字提示
            ui->openButton->setText(QString("关闭串口"));
        }
        else       //串口已经打开
        {
            globlePort.close();
            // 使能串口设置控件
            ui->portBox->setEnabled(true);
            ui->ParityBox->setEnabled(true);
            ui->baudRateBox->setEnabled(true);
            ui->DateBitsBox->setEnabled(true);
            ui->stopBitsBox->setEnabled(true);
            //ui->search_Button->setEnabled(true);
            //失能发送按钮
            ui->sendTxtButton->setEnabled(false);
            ui->iapButton->setEnabled(false);
            //调整串口控制按钮的文字提示
            ui->openButton->setText(QString("打开串口"));
         }
}
void Serial::ButtonSendPort(bool)
{
    QString dataStr = ui->textEdit->toPlainText();
    if(dataStr.isEmpty())
        return;

    if(ui->checkBoxHexTx->isChecked() == false)
    {

        if(dataStr.indexOf("\n") != -1)
        {
            dataStr.replace("\n","\r\n");
        }
        QByteArray array = dataStr.toLatin1();
        globlePort.write(array);
    }
    else
    {
        QByteArray array = QByteArray::fromHex(dataStr.toLatin1().data());
        globlePort.write(array);
    }
}
/*----------------------------------------------------------
 *          recive Date
 *----------------------------------------------------------*/
//读取接收到的数据
void Serial::ReciveDate()
{   
    static QByteArray Serial_buff;//定义static，否则会被清理
    Serial_buff += globlePort.readAll();
    emit DataRecvSignal(Serial_buff);

    if(ui->pushButton_3->text() == QString("暂停显示"))
    {
        //判断标志结束
        if(ui->checkBoxHex->isChecked() == false)
        {
            if(Serial_buff.isEmpty() == false)
            {
//                qDebug()<<Serial_buff;
                QString string = QString::fromLocal8Bit(Serial_buff);//中文字符输出
                ui->textBrowser->insertPlainText(string);
                Serial_buff.clear();//数据清理
            }
        }
        else if(ui->checkBoxHex->isChecked() == true)
        {
            if(Serial_buff.isEmpty() == false)
            {
                QString strDis;
                QByteArray hexData = Serial_buff.toHex();
                hexData = hexData.toUpper ();//转换为大写
                for(int i = 0;i<hexData.length ();i+=2)//填加空格
                {
                    QString st = hexData.mid (i,2);
                    strDis += st;
                    strDis += "  ";
                }
                Serial_buff.clear();//先清空，以防已接收的部分变成乱码
                Serial_buff.append(strDis);
                ui->textBrowser->insertPlainText(Serial_buff);
                Serial_buff.clear();//数据清理
            }
        }
    }
    else if(ui->pushButton_3->text() == QString("开始显示"))
    {
        Serial_buff.clear();
    }
}

void Serial::on_checkBoxHexTx_stateChanged(int arg1)
{
    ui->textEdit->clear();
    if(arg1 == 0)
    {
        disconnect(ui->textEdit, &QTextEdit::textChanged, this, &Serial::validateHEX);
    }
    else if (arg1 == 2)
    {
        connect(ui->textEdit, &QTextEdit::textChanged, this, &Serial::validateHEX);
    }
}


void Serial::on_iapButton_clicked()
{
    emit iapWidgetOpen();
}

