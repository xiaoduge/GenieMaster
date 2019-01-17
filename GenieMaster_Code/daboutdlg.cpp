#include "daboutdlg.h"
#include "ui_daboutdlg.h"
#include <QPainter>

DAboutDlg::DAboutDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DAboutDlg)
{
    ui->setupUi(this);

    Qt::WindowFlags flags = this->windowFlags() & ~Qt::WindowMaximizeButtonHint
                                                & ~Qt::WindowContextHelpButtonHint;
    this->setWindowFlags(flags);

    this->setFixedSize(300, 130);
    this->setWindowTitle(tr("关于.."));
}

DAboutDlg::~DAboutDlg()
{
    delete ui;
}

void DAboutDlg::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.setFont(QFont("华文行楷", 24, QFont::Bold));
    painter.setPen(Qt::blue);
    painter.drawText(QPointF(105, 40), tr("Rephile"));
    painter.setFont(QFont("宋体", 12));
    painter.setPen(Qt::black);
    painter.drawText(QPointF(45, 80), tr("Master Version 0.0.1 32-bit"));  //
    painter.drawText(QPointF(75, 100), tr("Built on 2017-10-15"));
}
