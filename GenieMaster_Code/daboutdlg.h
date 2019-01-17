#ifndef DABOUTDLG_H
#define DABOUTDLG_H

#include <QDialog>

namespace Ui {
class DAboutDlg;
}

class DAboutDlg : public QDialog
{
    Q_OBJECT

public:
    explicit DAboutDlg(QWidget *parent = 0);
    ~DAboutDlg();

protected:
    void paintEvent(QPaintEvent*);

private:
    Ui::DAboutDlg *ui;
};

#endif // DABOUTDLG_H
