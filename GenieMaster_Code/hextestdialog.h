#ifndef HEXTESTDIALOG_H
#define HEXTESTDIALOG_H

#include <QDialog>

namespace Ui {
class HexTestDialog;
}

class HexTestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HexTestDialog(QWidget *parent = 0);
    ~HexTestDialog();

signals:
    void sendHex(QString);
    void sendCmd();

private slots:
    void on_closepushButton_clicked();
    void on_testpushButton_clicked();
    void onAnswerFromParent(QString);

private:
    Ui::HexTestDialog *ui;
};

#endif // HEXTESTDIALOG_H
