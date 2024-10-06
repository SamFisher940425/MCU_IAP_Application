#include "ota_widget.h"
#include "ui_ota_widget.h"
#include "QFileDialog"

ota_widget::ota_widget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ota_widget)
{
    ui->setupUi(this);
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

}

void ota_widget::on_startButton_clicked()
{
    QByteArray array;
    array.append(0x55);
    array.append(0xAA);
    array.append(0xFF);
    emit DataSendSignal(array);
}

void ota_widget::on_fileSelectButton_clicked()
{
    binFileName = QFileDialog::getOpenFileName(this,tr("Open File"),"",tr("bin Files (*.bin);;"));
    ui->fileNameLineEdit->setText(binFileName);
}

