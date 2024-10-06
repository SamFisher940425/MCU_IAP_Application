#include "ota_widget.h"
#include "ui_ota_widget.h"

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
