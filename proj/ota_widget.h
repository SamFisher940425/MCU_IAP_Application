#ifndef OTA_WIDGET_H
#define OTA_WIDGET_H

#include <QDialog>

namespace Ui {
class ota_widget;
}

class ota_widget : public QDialog
{
    Q_OBJECT

public:
    explicit ota_widget(QWidget *parent = nullptr);
    ~ota_widget();

    void iapWidgetShow(); //用于外部触发窗口显示的槽函数
    void DataRecvSlot(QByteArray data_array);

signals:
    void DataSendSignal(QByteArray data_array);

private slots:
    void on_startButton_clicked();
    void on_fileSelectButton_clicked();

private:
    Ui::ota_widget *ui;
    QString binFileName;
};

#endif // OTA_WIDGET_H
