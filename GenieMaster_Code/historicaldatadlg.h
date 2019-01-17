#ifndef HISTORICALDATADLG_H
#define HISTORICALDATADLG_H

#include <QDialog>

namespace Ui {
class HistoricalDataDlg;
}

class HistoricalDataDlg : public QDialog
{
    Q_OBJECT

public:
    explicit HistoricalDataDlg(QWidget *parent = 0);
    ~HistoricalDataDlg();

protected slots:
    void showHistoricalData(QStack<QString>&);
    void on_requestBtn_clicked();

private:
    void initDlgUI();
    void initTextBrower();
    void initButton();
    void initComboBox();
    void initFinishMessageBox();
    void initLabel();

signals:
    void sendCurrentIndex(int&);

private slots:
    void on_outputButton_clicked();

private:
    Ui::HistoricalDataDlg *ui;
};

#endif // HISTORICALDATADLG_H
