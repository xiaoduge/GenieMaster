#ifndef ALARMMSGDLG_H
#define ALARMMSGDLG_H

#include <QDialog>

namespace Ui {
class AlarmMsgDlg;
}

class AlarmMsgDlg : public QDialog
{
    Q_OBJECT

public:
    explicit AlarmMsgDlg(QWidget *parent = 0);
    ~AlarmMsgDlg();

protected slots:
    void showAlarmMsg(QStack<QString>&);

private slots:
    void on_outputButton_clicked();

private:
    void initTextBrower();
    void initFinishMessageBox();
    void initButton();

private:
    Ui::AlarmMsgDlg *ui;
};

#endif // ALARMMSGDLG_H
