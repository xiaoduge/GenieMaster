#include "historicaldatadlg.h"
#include "ui_historicaldatadlg.h"
#include <QStack>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QDebug>

HistoricalDataDlg::HistoricalDataDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HistoricalDataDlg)
{
    ui->setupUi(this);

    Qt::WindowFlags flags = this->windowFlags() & ~Qt::WindowContextHelpButtonHint;
    this->setWindowFlags(flags);
    this->setWindowTitle(tr("历史数据"));
//    this->resize(600, 360);

    initDlgUI();
    initTextBrower();
    initComboBox();
    initButton();
    initLabel();
}

HistoricalDataDlg::~HistoricalDataDlg()
{
    delete ui;
}

void HistoricalDataDlg::showHistoricalData(QStack<QString>& strStack)
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

void HistoricalDataDlg::on_requestBtn_clicked()
{
    int index = ui->comboBox->currentIndex();
    emit sendCurrentIndex(index);
}

void HistoricalDataDlg::initDlgUI()
{
    QString qmlDlg = "QDialog{background-color:#003778;width:600;height:360;}";
    this->setStyleSheet(qmlDlg);
}

void HistoricalDataDlg::initTextBrower()
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

void HistoricalDataDlg::initButton()
{
    QString qmlBtn = "QPushButton{background-color:white;\
                                  color:#003778;\
                                  font-family:宋体;\
                                  width:80px;\
                                  height:20px;\
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
    ui->requestBtn->setStyleSheet(qmlBtn);
    ui->outputButton->setStyleSheet(qmlBtn);
}

void HistoricalDataDlg::initComboBox()
{
    QStringList optionList;
    optionList << tr("进水水质")  << tr("Ro水质") << tr("EDI水质") << tr("工作压力")
               << tr("Ro产水流量") << tr("Ro弃水流量") << tr("温度");
    ui->comboBox->addItems(optionList);
}

void HistoricalDataDlg::on_outputButton_clicked()
{
    QString saveFileName = QFileDialog::getSaveFileName(this, tr("保存"), "D:", tr("文本(*.csv)"));
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
    out << QString("Date") << QString(",") << QString("Values") << endl;
    out << strTemp;
    saveFile.close();
    initFinishMessageBox();
}

void HistoricalDataDlg::initFinishMessageBox()
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

void HistoricalDataDlg::initLabel()
{
    QString qmlLabel = "QLabel{color:white;}";
    ui->label->setStyleSheet(qmlLabel);
}

