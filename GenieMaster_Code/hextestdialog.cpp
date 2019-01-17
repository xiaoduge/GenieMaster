#include "hextestdialog.h"
#include "ui_hextestdialog.h"
#include "mainwindow.h"

HexTestDialog::HexTestDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HexTestDialog)
{
    ui->setupUi(this);

    this->setWindowTitle(tr("通讯测试"));
    Qt::WindowFlags flags = this->windowFlags() & ~Qt::WindowContextHelpButtonHint;
    this->setWindowFlags(flags);
    this->setFixedSize(this->width(), this->height());
    QObject *tempParent = this->parent();
    const MainWindow *mainParent = static_cast<const MainWindow*>(tempParent);
    connect(mainParent, &MainWindow::sendAnswerToChild, this, &HexTestDialog::onAnswerFromParent);
}

HexTestDialog::~HexTestDialog()
{
    delete ui;
}

void HexTestDialog::on_closepushButton_clicked()
{
    this->close();
}

void HexTestDialog::on_testpushButton_clicked()
{
    QString hex = ui->hexLineEdit->text().toLatin1();
    emit sendHex(hex);
    emit sendCmd();
}

void HexTestDialog::onAnswerFromParent(QString answer)
{
    ui->backlineEdit->setText(answer);
}
