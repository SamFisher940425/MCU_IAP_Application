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

private:
    Ui::ota_widget *ui;
};

#endif // OTA_WIDGET_H
