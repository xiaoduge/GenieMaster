#include "alarmmsgdlg.h"
#include "ui_alarmmsgdlg.h"
#include <QStack>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QTextStream>

AlarmMsgDlg::AlarmMsgDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AlarmMsgDlg)
{
    ui->setupUi(this);

    Qt::WindowFlags flags = this->windowFlags() & ~Qt::WindowContextHelpButtonHint;
    this->setWindowFlags(flags);
    this->setWindowTitle(tr("报警信息"));
    this->resize(600, 360);
    this->setStyleSheet("background-color:#003778;");
    initTextBrower();
    initButton();
}

AlarmMsgDlg::~AlarmMsgDlg()
{
    delete ui;
}

void AlarmMsgDlg::showAlarmMsg(QStack<QString>& strStack)
{
    QString strTemp;
    while(!strStack.isEmpty())
    {
        strTemp += strStack.pop();
        strTemp += QString("\n");
    }
    ui->textBrowser->setText(strTemp);
    strStack.clear();
}

void AlarmMsgDlg::initTextBrower()
{
    QString qmlTextBrowser = "QTextBrowser{background-color:#003778;\
                                           border-radius:5px;\
                                           border-color:white;\
                                           border-width:1px;\
                                           border-style: outset;\
                                           font-family:宋体;\
                                           font-size:18px;\
                                           font-width:bold;\
                                           color:white;\
                                           }";
ui->textBrowser->setStyleSheet(qmlTextBrowser);
}

void AlarmMsgDlg::initFinishMessageBox()
{
    QString qmlMsgBox = "QDialog{background-color:#003778; \
                                }\
                         QLabel{color:white;\
                                font-family:宋体;\
                                font-size:16px;\
                               }";
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("完成"));
    msgBox.setText(tr("数据导出完成"));
    msgBox.addButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setStyleSheet(qmlMsgBox);
    msgBox.exec();
}

void AlarmMsgDlg::initButton()
{
    QString qmlBtn = "QPushButton{background-color:white;\
                                  color:#003778;\
                                  font-family:宋体;\
                                  width:100px;   \
                                  height:25px;  \
                                  border-radius:5px;\
                                  border-style: outset;\
                                  } \
                      QPushButton:hover{background-color:#D7D7D7;\
                                        color:#003778;\
                                        border-radius:5px;\
                                        }\
                      QPushButton:pressed{background-color:#C7C7C7;\
                                          color:#003778;\
                                          border-radius:5px;\
                                          border-color:black;\
                                          border-width:1px;\
                                          border-style: outset;\
                                          }";
    ui->outputButton->setStyleSheet(qmlBtn);
}

void AlarmMsgDlg::on_outputButton_clicked()
{
    QString saveFileName = QFileDialog::getSaveFileName(this, tr("保存"), "D:", tr("文件(*.csv)"));
    if(saveFileName.isEmpty())
    {
        QMessageBox::warning(this, tr("错误"), tr("保存文件失败"), QMessageBox::Ok);
        return;
    }
    QFile saveFile(saveFileName);
    if(!saveFile.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this, tr("错误"), tr("保存文件失败"), QMessageBox::Ok);
        return;
    }
    QTextStream out(&saveFile);
    QString strTemp = ui->textBrowser->toPlainText();
    strTemp.replace(QString("  "), QString(","));
    out << QString("Date") << QString(",") << QString("Time") << QString(",") << QString("Values") << endl;
    out << strTemp;
    saveFile.close();
    initFinishMessageBox();
}
