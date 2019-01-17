#include "configdialog.h"
#include "ui_configdialog.h"
#include <QSerialPort>
#include <QSerialPortInfo>

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);

    this->setWindowTitle(tr("通讯配置"));  //(B?$BCVcY8}L>>N(B

    //
    Qt::WindowFlags flags = this->windowFlags() & ~Qt::WindowContextHelpButtonHint;
    this->setWindowFlags(flags);
    this->setFixedSize(this->width(), this->height()); //设置对话框固定大小
    initChildControls(); //
    initDlgUI();
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::initChildControls()
{
    //获取可用COM
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QSerialPort serial;
        serial.setPort(info);
        if(serial.open(QIODevice::ReadWrite))
        {
            ui->comComboBox->addItem(serial.portName());
            serial.close();
        }
    }

    QStringList baudList;
    baudList << tr("1200") << tr("2400") << tr("4800") << tr("9600")
             << tr("19200") << tr("38400") << tr("57600") << tr("115200");
    ui->baudComboBox->addItems(baudList);
    ui->baudComboBox->setCurrentIndex(3);

    QStringList dataList;
    dataList << tr("5") << tr("6") << tr("7") << tr("8");
    ui->DataComboBox->addItems(dataList);
    ui->DataComboBox->setCurrentIndex(3);

    QStringList parityList;
    parityList << tr("None") << tr("Even") << tr("Odd") << tr("Space") << tr("Mark");
    ui->parityComboBox->addItems(parityList);
    ui->parityComboBox->setCurrentIndex(1);

    QStringList stopList;
    stopList << tr("1") << tr("1.5") << tr("2");
    ui->stopComboBox->addItems(stopList);
    ui->stopComboBox->setCurrentIndex(0);

    QStringList flowList;
    flowList << tr("None") << tr("RTS/CTS") << tr("XON/XOFF");
    ui->flowComboBox->addItems(flowList);
    ui->flowComboBox->setCurrentIndex(0);
}

void ConfigDialog::initDlgUI()
{
    QString qmlDlg = "QDialog{background-color:#003778;}\
                      QLabel{color:white;}\
                      QPushButton{background-color:white;\
                                  color:#003778;\
                                  font-family:宋体;\
                                  width:60px;\
                                  height:40px;\
                                  border-radius:3px;\
                                  border-style: outset;\
                                  } \
                      QPushButton:hover{background-color:#D7D7D7;\
                                        }\
                      QPushButton:pressed{background-color:#C7C7C7;\
                                          border-color:black;\
                                          border-width:1px;\
                                          border-style: outset;\
                                          }";

    this->setStyleSheet(qmlDlg);
}

void ConfigDialog::on_configPushButton_clicked()
{
    QString strPortName = ui->comComboBox->currentText();
    emit sendPortName(strPortName);

    QString strBaudRate = ui->baudComboBox->currentText();
    emit sendBaud(strBaudRate);

    int databitsIndex = ui->DataComboBox->currentIndex();
    emit sendDataBits(databitsIndex);

    int paritybitsIndex = ui->parityComboBox->currentIndex();
    emit sendParityBits(paritybitsIndex);

    int stopbitsIndex = ui->stopComboBox->currentIndex();
    emit sendStopBits(stopbitsIndex);

    int flowcontrolIndex = ui->flowComboBox->currentIndex();
    emit sendFlowControl(flowcontrolIndex);

    this->close();
}










