#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = 0);
    ~ConfigDialog();

signals:
    void sendPortName(QString);
    void sendBaud(QString);
    void sendDataBits(int);
    void sendParityBits(int);
    void sendStopBits(int);
    void sendFlowControl(int);

private slots:
    void on_configPushButton_clicked();

private:
    void initChildControls();
    void initDlgUI();

private:
    Ui::ConfigDialog *ui;
};

#endif // CONFIGDIALOG_H
