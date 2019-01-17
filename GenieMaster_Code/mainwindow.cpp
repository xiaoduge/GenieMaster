#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "configdialog.h"
#include "hextestdialog.h"
#include "alarmmsgdlg.h"
#include "historicaldatadlg.h"
#include "hexfloat.h"
#include "dtablemodel.h"
#include "daboutdlg.h"
#include "dhelpdialog.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTime>
#include <QMessageBox>
#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QDate>
#include <QTime>
#include <QDesktopWidget>
#include <QTableView>
#include <QHeaderView>
#include <QTextBrowser>
#include <QDir>
#include <QLabel>


int indexFeed = 0;
int indexRo = 1;
int indexEDI = 2;
int indexPres = 3;
int indexRoRate = 4;
int indexDrainRate = 5;
int indexTemperature = 6;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowState(Qt::WindowMaximized); //初始化窗口最大化
    this->setWindowTitle(tr("监控软件"));     //设置窗口标题

    m_serialPort = new QSerialPort(this); //
    m_configDlg = new ConfigDialog(this); //
    m_hexTestDlg = new HexTestDialog(this);    
    connect(m_hexTestDlg, SIGNAL(sendHex(QString)), this, SLOT(onTestDlg(QString)));
    m_AlarmMsgDlg = new AlarmMsgDlg(this);
    connect(this, SIGNAL(sendAlarmMsgToDlg(QStack<QString>&)), m_AlarmMsgDlg, SLOT(showAlarmMsg(QStack<QString>&)));
    m_HistoricalDataDlg = new HistoricalDataDlg(this);
    connect(this, SIGNAL(sendHistoricalDataToDlg(QStack<QString>&)), m_HistoricalDataDlg, SLOT(showHistoricalData(QStack<QString>&)));
    connect(m_HistoricalDataDlg, SIGNAL(sendCurrentIndex(int&)), this, SLOT(onReceiveHistorucalDataIndex(int&)));

    m_aboutDlg = new DAboutDlg(this);
    m_helpDlg = new DHelpDialog(this);

    m_consumablesView = new QTableView;
    m_consumablesView->horizontalHeader()->setVisible(false); //不显示列表头
    m_consumablesView->verticalHeader()->setVisible(false);   //不显示行表头
    m_consumablesView->setWindowTitle(tr("耗材信息"));
    m_consumablesTableModel = new DTableModel;

    m_parameterView = new QTableView;
    m_parameterView->horizontalHeader()->setVisible(false);
    m_parameterView->verticalHeader()->setVisible(false);
    m_parameterView->setWindowTitle(tr("运行参数"));
    m_parameterTableModel = new DTableModel;

    m_alarmThread.start(); //启动报警信息写入线程
    m_histroyThread.start();
    connectConfigSignals();
    initMenuBar(); //初始化菜单栏
    initStatusBar(); //初始化状态栏
    initButton();
    initGroupBox();
    initRelayState();
    initScrollerLabel(); //初始化滚动字幕;

    loadPixmap();
    loadHexCmd();
    initLogFile();

    isRunButtonClicked = false;
    isStandbyButtonClicked = false;
    m_nPos = 0;

    getScreenInfo();    //获取显示器分辨率

    QPalette palette = this->palette();
    palette.setBrush(QPalette::Background, QBrush(QColor(0, 55, 120))); //
    this->setPalette(palette);

    isConnect = false; //串口是否连接
    isAbnormal = false; //判断返回数据是否正常，返回数据长度0时为true

    isFeedTank = true;
}

MainWindow::~MainWindow()
{
    if(logFile.isOpen())
    {
        logFile.close();
    }
    if(m_consumablesView != NULL)
    {
        delete m_consumablesView;
        m_consumablesView = NULL;
    }
    if(m_consumablesTableModel != NULL)
    {
        delete m_consumablesTableModel;
        m_consumablesTableModel = NULL;
    }
    if(m_parameterView != NULL)
    {
        delete m_parameterView;
        m_parameterView = NULL;
    }
    if(m_parameterTableModel != NULL)
    {
        delete m_parameterTableModel;
        m_parameterTableModel = NULL;
    }
    if(m_alarmThread.isRunning())
    {
        m_alarmThread.stop();
        m_alarmThread.wait();
    }
    if(m_histroyThread.isRunning())
    {
        m_histroyThread.stop();
        m_histroyThread.wait();
    }
    delete ui;
}

void MainWindow::timerEvent(QTimerEvent *event)
{

    if(id1 == event->timerId())
    {
        analyzeHex(m_receiveData);
        m_receiveData.clear();
        killTimer(id1);
    }
    else if(idTest == event->timerId())
    {
        emit sendAnswerToChild(m_receiveData);
        m_receiveData.clear();
        killTimer(idTest);
    }
    else if(idRefresh == event->timerId())
    {
        update();
    }
    else if(idStartCheck == event->timerId())
    {
        //若有运行按钮点击命令，则先发送运行命令
        killTimer(idStartCheck);
        if(isRunButtonClicked
           && m_curRelayState.stateM68 == RelayStateCode::RelayOn
           && m_curRelayState.stateM47 == RelayStateCode::RelayOn)
        {
            isRunButtonClicked = false;
            writeHexListRunClick();
            id1 = startTimer(50); //100
        }
        //若有待机按钮点击命令，则先发送待机命令
        else if(isStandbyButtonClicked
                && m_curRelayState.stateM68 == RelayStateCode::RelayOn
                && m_curRelayState.stateM47 == RelayStateCode::RelayOn)
        {
            isStandbyButtonClicked = false;
            writeHex(m_HexCmd.hexOnX7);
            sleep(50);
        }
        else
        {
            isRunButtonClicked = false;
            isStandbyButtonClicked = false;
            writeHexList();
            id1 = startTimer(50); //100
        }
        idAutoCheck = startTimer(3000);
    }

    else if(idAutoCheck == event->timerId())
    {
        //若有运行按钮点击命令，则先发送运行命令
        if(isRunButtonClicked
           && m_curRelayState.stateM68 == RelayStateCode::RelayOn
           && m_curRelayState.stateM47 == RelayStateCode::RelayOn)
        {
            isRunButtonClicked = false;
            writeHexListRunClick();
            id1 = startTimer(50); //100
        }
        //若有待机按钮点击命令，则先发送待机命令
        else if(isStandbyButtonClicked
                && m_curRelayState.stateM68 == RelayStateCode::RelayOn
                && m_curRelayState.stateM47 == RelayStateCode::RelayOn)
        {
            isStandbyButtonClicked = false;
            writeHexListStandbyClick();
            id1 = startTimer(50); //
        }
        else
        {
            writeHexList();
            id1 = startTimer(50); //100
        }
    }
}

void MainWindow::writeHex(QString &hex)
{
    QString str = hex.toLatin1();
    QString strHex = str.remove(QRegExp("\\s"));
    if(1 == strHex.length()%2)
    {
        strHex = strHex.insert(strHex.length() - 1, '0');
    }
    QByteArray sendData;
    StringToHex(strHex, sendData);
    m_serialPort->write(sendData);
}

void MainWindow::readHex()
{
    QByteArray buf = m_serialPort->readAll();
    QDataStream out(&buf, QIODevice::ReadWrite);
    while(!out.atEnd())
    {
        qint8 outChar = 0;
        out >> outChar;

        QString strReceive = QString("%1").arg(outChar&0xFF, 2, 16, QLatin1Char('0'));
        m_receiveData += strReceive.toUpper();
    }
}

void MainWindow::getScreenInfo()
{
    QDesktopWidget *desktopWidget = QApplication::desktop();
    QRect screenRect = desktopWidget->screenGeometry();
    m_screenWidth = screenRect.width();
    m_screenHeight = screenRect.height();
}

bool MainWindow::isPathExist(const QString &dirName) //判断目录是否存在，不存在则创建
{
    QDir dir(dirName);
    if(dir.exists())
    {
        return true;
    }
    else
    {
        return dir.mkpath(dirName);
    }
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    this->setFixedSize(this->width(), this->height()); //设置窗口固定大小

    QPoint groupBoxPos = ui->operateGroupBox->pos();
    int groupBoxWidth = ui->operateGroupBox->width();
    int groupBoxHeight = ui->operateGroupBox->height();
    int runX = groupBoxPos.x() + groupBoxWidth + 10;
    int runY = groupBoxPos.y() + ui->menuBar->height() + ui->mainToolBar->height() + 10;
    int runWidth = this->width() - (groupBoxPos.x() + groupBoxWidth + 20);
    int runHeight = groupBoxHeight;
    int posX = runX - 20;
    int posY = runY;

    double xTimes = m_screenWidth/1366.000;
    double yTimes = m_screenHeight/768.000;

    QPainter painter(this);

    drawDestRect(painter, runX, runY, runWidth, runHeight, xTimes, yTimes); //绘制目标区域轮廓
    drawPipeLine(painter, posX, posY, xTimes, yTimes);                      //绘制管道
    drawDeviceText(painter, posX, posY, xTimes, yTimes);                    //绘制设备标号
    drawDevicePixmap(painter, posX, posY, xTimes, yTimes);                  //绘制设备图标
    drawSignsLight(painter, posX, posY, xTimes, yTimes);
    drawParameter(painter, posX, posY, xTimes, yTimes);

    //绘制滚动字幕大小和位置
    m_scrollerLabel->move(5, this->height()-ui->statusBar->height()-35);
    m_scrollerLabel->resize(this->width() - 10, m_scrollerLabel->height());

}

void MainWindow::setPortName(QString portName)
{
    if(m_serialPort->isOpen())
    {
        m_serialPort->close();
    }

    bool ret;
    m_serialPort->setPort(QSerialPortInfo(portName));
    ret = m_serialPort->open(QIODevice::ReadWrite);
    QTextStream out(&logFile);
    QTime currentTime = QTime::currentTime();
    QString strCurTime = currentTime.toString("HH:mm:ss");
    QString strLogData;
    if(false == ret)
    {
        strLogData = strCurTime + tr("  Open port false  ") +  tr("\n");
        out << strLogData;
        isConnect = false;
        QMessageBox::warning(this, tr("Warning"), tr("Open port false"), QMessageBox::Ok);
    }
    else
    {
        connect(m_serialPort, &QSerialPort::readyRead, this, &MainWindow::readHex);
        idStartCheck = startTimer(100);
        idRefresh = startTimer(400);    //界面刷新
        strLogData = strCurTime + tr("  Open port success  ") +  tr("\n");
        out << strLogData;
        isConnect = true;
    }
}

void MainWindow::setBaudRate(QString baudRate)
{
     m_serialPort->setBaudRate(baudRate.toInt());
}

void MainWindow::setDataBits(int dataBitsIndex)
{
    switch(dataBitsIndex)
    {
    case 0:
        m_serialPort->setDataBits(QSerialPort::Data5);
        break;
    case 1:
        m_serialPort->setDataBits(QSerialPort::Data6);
        break;
    case 2:
        m_serialPort->setDataBits(QSerialPort::Data7);
        break;
    case 3:
        m_serialPort->setDataBits(QSerialPort::Data8);
        break;
    default:
        break;
    }
}

void MainWindow::setParityBits(int parityBitsIndex)
{
    switch (parityBitsIndex)
    {
    case 0:
        m_serialPort->setParity(QSerialPort::NoParity);
        break;
    case 1:
        m_serialPort->setParity(QSerialPort::EvenParity);
        break;
    case 2:
        m_serialPort->setParity(QSerialPort::OddParity);
        break;
    case 3:
        m_serialPort->setParity(QSerialPort::SpaceParity);
        break;
    case 4:
        m_serialPort->setParity(QSerialPort::MarkParity);
        break;
    default:
        break;
    }
}

void MainWindow::setStopBits(int stopBitsIndex)
{
    switch(stopBitsIndex)
    {
    case 0:
        m_serialPort->setStopBits(QSerialPort::OneStop);
        break;
    case 1:
        m_serialPort->setStopBits(QSerialPort::OneAndHalfStop);
        break;
    case 2:
        m_serialPort->setStopBits(QSerialPort::TwoStop);
        break;
    default:
        break;
    }
}

void MainWindow::setFlowControl(int flowControlIndex)
{
    switch(flowControlIndex)
    {
    case 0:
        m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
        break;
    case 1:
        m_serialPort->setFlowControl(QSerialPort::HardwareControl);
        break;
    case 2:
        m_serialPort->setFlowControl(QSerialPort::SoftwareControl);
        break;
    default:
        break;
    }
}

void MainWindow::onConfigActiontriggered()
{
    m_configDlg->show();

}

void MainWindow::onCmdTesttriggered()
{    
    m_hexTestDlg->show();
}

void MainWindow::onCloseConnectActiontriggered()
{
    if(m_serialPort->isOpen())
    {
        m_serialPort->close();
    }
    initRelayState();
    isConnect = false;
}

void MainWindow::onTestDlg(QString cmd)
{
    writeHex(cmd);
    idTest = startTimer(50);
}

void MainWindow::onConsumablesActiontriggered()
{
    QVector<QStringList> consumablesVector;
    //P Pack D285;AT Pack D301;水箱空气过滤器D287;纯水紫外灯D288;水箱紫外灯D290;管道紫外灯D286;管道过滤器：D277
    //管道紫外灯 uintD286;  管道过滤器 uintD277;
    QStringList list0, list1, list2, list3, list4, list5;
    list0 << tr("Consumables") << tr("Time Remaining");
    list1 << tr("P Pack") << tr("%1").arg(m_consumablesStatus.uintD285);
    list2 << tr("AT Pack") << tr("%1").arg(m_consumablesStatus.uintD301);
    list3 << tr("Tank Vent Filter") << tr("%1").arg(m_consumablesStatus.uintD287);
    list4 << tr("UV Lamp for EDI water") << tr("%1").arg(m_consumablesStatus.uintD288);
    list5 << tr("Tank UV Lamp") << tr("%1").arg(m_consumablesStatus.uintD290);
    consumablesVector << list0 << list1 << list2 << list3 << list4 << list5;

    m_consumablesTableModel->setTableModelVector(consumablesVector);
    m_consumablesTableModel->setTableModelColumnCount(2);

    m_consumablesView->setModel(m_consumablesTableModel);
    m_consumablesView->setAlternatingRowColors(true);
    m_consumablesView->setColumnWidth(0, 300);
    m_consumablesView->setColumnWidth(1, 300);
    for(int i = 0; i < m_consumablesTableModel->rowCount(QModelIndex()); i++)
    {
        m_consumablesView->setRowHeight(i, 50);
    }
    m_consumablesView->setFixedSize(604, 304);

    m_consumablesView->show();
}

void MainWindow::onParameterActiontriggered()
{
    QVector<QStringList> parameterVector;
    QStringList list;

    list << tr("Parameter") << tr("Value");
    parameterVector << list;
    list.clear();
    if(isFeedTank)
    {
        list << tr("Feed Tank Level") << tr("%1").arg(m_curRegisterData.doubleD340);
        parameterVector << list;
        list.clear();
    }

    list << tr("Feed Pressure") << tr("%1").arg(m_curRegisterData.doubleD160);
    parameterVector << list;
    list.clear();

    list << tr("Feed Water Temp.") << tr("%1").arg(m_curRegisterData.doubleD96);
    parameterVector << list;
    list.clear();

    list << tr("Feed Water Cond.") << tr("%1").arg(m_curRegisterData.doubleD73);
    parameterVector << list;
    list.clear();

    list << tr("RO Cond.") << tr("%1").arg(m_curRegisterData.doubleD75);
    parameterVector << list;
    list.clear();

    list << tr("EDI Resis.") << tr("%1").arg(m_curRegisterData.doubleD71);
    parameterVector << list;
    list.clear();

    list << tr("Operation Pres.") << tr("%1").arg(m_curRegisterData.doubleD168);
    parameterVector << list;
    list.clear();

    list << tr("RO Permeate Rate") << tr("%1").arg(m_curRegisterData.doubleD122);
    parameterVector << list;
    list.clear();

    list << tr("RO Drain Rate") << tr("%1").arg(m_curRegisterData.doubleD116);
    parameterVector << list;
    list.clear();

    list << tr("Tank Level") << tr("%1").arg(m_curRegisterData.doubleD166);
    parameterVector << list;
    list.clear();

    list << tr("RO Rejection Rate") << tr("%1").arg(m_curRegisterData.doubleD94);
    parameterVector << list;

    m_parameterTableModel->setTableModelVector(parameterVector);
    m_parameterTableModel->setTableModelColumnCount(2);

    m_parameterView->setModel(m_parameterTableModel);
    m_parameterView->setAlternatingRowColors(true);
    m_parameterView->setColumnWidth(0, 300);
    m_parameterView->setColumnWidth(1, 300);
    for(int i = 0; i < m_parameterTableModel->rowCount(QModelIndex()); i++)
    {
        m_parameterView->setRowHeight(i, 50);
    }
    if(isFeedTank)
    {
        m_parameterView->setFixedSize(604, 604);
    }
    else
    {
        m_parameterView->setFixedSize(604, 554);
    }
    m_parameterView->show();
}

void MainWindow::onAlarmMessageActiontriggered()
{
    emit requestAlarmMsg();
}

void MainWindow::onChildThreadError(const QString &msg)
{
    QTextStream out(&logFile);
    QTime curTime = QTime::currentTime();
    QString str_curTime = curTime.toString("HH:mm:ss");
    QString str_Temp = str_curTime + tr("  ") + msg + tr("\n");
    out << str_Temp;
}

void MainWindow::onReceiveAlarmMessage(QStack<QString>& strStack)
{
    emit sendAlarmMsgToDlg(strStack);
    m_AlarmMsgDlg->show();
}

void MainWindow::onHistoricalDataActiontriggered()
{
    m_HistoricalDataDlg->show();
}

void MainWindow::onReceiveHistorucalDataIndex(int& index)
{
    emit requestHistoricalData(index);
}

void MainWindow::onFeedTankConfigtoggle(bool isChecked)
{
    isFeedTank = isChecked;
    update();
    qDebug() << "isFeedTank:" << isFeedTank;
}

void MainWindow::onVersionActiontriggered()
{
    m_aboutDlg->show();
}

void MainWindow::onHelpActiontriggered()
{
    m_helpDlg->show();
}

void MainWindow::onReceiveHistoricalData(QStack<QString>& strStack)
{
    emit sendHistoricalDataToDlg(strStack);
}

void MainWindow::connectConfigSignals()
{
    connect(m_configDlg, SIGNAL(sendPortName(QString)), this, SLOT(setPortName(QString)));
    connect(m_configDlg, SIGNAL(sendBaud(QString)), this, SLOT(setBaudRate(QString)));
    connect(m_configDlg, SIGNAL(sendDataBits(int)), this, SLOT(setDataBits(int)));
    connect(m_configDlg, SIGNAL(sendParityBits(int)), this, SLOT(setParityBits(int)));
    connect(m_configDlg, SIGNAL(sendStopBits(int)), this, SLOT(setStopBits(int)));
    connect(m_configDlg, SIGNAL(sendFlowControl(int)), this, SLOT(setFlowControl(int)));
   //报警信息线程
    connect(this, SIGNAL(sendAlarmMsg(const QString&)),
            &m_alarmThread, SLOT(writeAlarm(const QString&)));
    connect(&m_alarmThread, SIGNAL(openFileError(const QString&)),
            this, SLOT(onChildThreadError(const QString&)));
    connect(this, SIGNAL(requestAlarmMsg()), &m_alarmThread, SLOT(readAlarm()));
    connect(&m_alarmThread, SIGNAL(sendAlarmMessage(QStack<QString>&)),
            this, SLOT(onReceiveAlarmMessage(QStack<QString>&)));

  //历史数据线程
    connect(this, SIGNAL(sendHistoricalData(const QString&, int&)),
            &m_histroyThread, SLOT(writeHistoricalData(const QString&, int&)));
    connect(&m_histroyThread, SIGNAL(openFileError(const QString&)),
            this, SLOT(onChildThreadError(const QString&)));
    connect(this, SIGNAL(requestHistoricalData(int&)), &m_histroyThread, SLOT(readHistoricalData(int&)));
    connect(&m_histroyThread, SIGNAL(sendHistoricalData(QStack<QString>&)),
            this, SLOT(onReceiveHistoricalData(QStack<QString>&)));
}

void MainWindow::StringToHex(QString str, QByteArray &senddata)
{
    char lstr, hstr;
    int hHexData, lHexData, hexData;
    int hexDataLen = 0;
    int len = str.length();
    senddata.resize(len/2);
    for(int i = 0; i < len;)
    {
        hstr = str[i].toLatin1();
        if(hstr == ' ')
        {
            i++;
            continue;
        }
        i++;
        if(i >= len)
            break;
        lstr = str[i].toLatin1();

        hHexData = ConvertHexChar(hstr);
        lHexData = ConvertHexChar(lstr);
        if((hHexData == 16) || (lHexData == 16))
        {
            break;
        }
        else
        {
            hexData = hHexData*16 + lHexData;
        }
        i++;
        senddata[hexDataLen] = (char)hexData;
        hexDataLen++;
    }
    senddata.resize(hexDataLen);
}

char MainWindow::ConvertHexChar(char ch)
{
    if((ch >= '0') && (ch <= '9'))
    {
        return ch - 0x30;
    }
    else if((ch >= 'A') && (ch <= 'F'))
    {
        return ch - 'A' + 10;
    }
    else if((ch >= 'a') && (ch <= 'f'))
    {
        return ch - 'a' + 10;
    }
    else
    {
        return ch - ch;
    }
}

void MainWindow::sleep(unsigned int msec)
{
    QTime delayTime = QTime::currentTime().addMSecs(msec);
    while(QTime::currentTime() < delayTime)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}

void MainWindow::analyzeHex(QString &hex)
{
    QTextStream out(&logFile);
    QTime currentTime = QTime::currentTime();
    QString strCurTime = currentTime.toString("HH:mm:ss");
    QString strLogData;

    int len = hex.length();
    if(0 == len)
    {
        isAbnormal = true;
    }
    else
    {
        isAbnormal = false;
    }
    if(len != 544 && len != 84 && len != 44)
    {
        strLogData = strCurTime + tr("  Len:%1  ").arg(len) + hex + tr("\n");
    }
    else
    {
        strLogData = strCurTime + tr("  Len:%1  ").arg(len) + tr("\n");
    }
    out << strLogData;
    strLogData.clear();

    qDebug() << tr("len:%1").arg(len);
    qDebug() << tr("Data:%1").arg(hex);

    QStringList cmdList;
    if(544 == len) //返回指令总长度552
    {
        m_strBackM68 = hex.mid(0, LEN12);    //M68是否处于初始界面 返回长度12
        m_curRelayState.stateM68 = analyzeSingleRelay(m_strBackM68);

        m_strBackM47 = hex.mid(LEN12, LEN12);
        m_curRelayState.stateM47 = analyzeSingleRelay(m_strBackM47);     //M47是否处于维护维修状态 返回长度12

        m_strBackM37 = hex.mid(LEN12*2, LEN12);    //M37是否处于运行状态  返回长度12
        m_curRelayState.stateM37 = analyzeSingleRelay(m_strBackM37);

        m_strBackYout = hex.mid(LEN12*3, LEN16);   //Yout输出  返回长度16
        analyzeYout(m_strBackYout);

        m_strBackD340 = hex.mid(LEN12*3+LEN16, LEN18);   //原水箱液位D340  返回长度18
        m_curRegisterData.doubleD340 = analyzeRegister32(m_strBackD340);
        m_curRegisterData.doubleD340 = (int)(m_curRegisterData.doubleD340 + 0.5); //int

        m_strBackD160 = hex.mid(LEN12*3+LEN16+LEN18, LEN18);   //进水压力D160   返回长度18
        m_curRegisterData.doubleD160 = analyzeRegister32(m_strBackD160);
        m_curRegisterData.doubleD160 = (float)(int)(m_curRegisterData.doubleD160*10+0.5)/10;

        m_strBackD96 = hex.mid(LEN12*3+LEN16+LEN18*2, LEN18);    // 温度D96       返回长度18
        m_curRegisterData.doubleD96 = analyzeRegister32(m_strBackD96);
        m_curRegisterData.doubleD96 = (float)(int)(m_curRegisterData.doubleD96*10+0.5)/10;
        emit sendHistoricalData(QString("%1").arg(m_curRegisterData.doubleD96), indexTemperature);

        m_strBackD73 = hex.mid(LEN12*3+LEN16+LEN18*3, LEN18);    //进水电导D73     返回长度18
        m_curRegisterData.doubleD73 = analyzeRegister32(m_strBackD73);
        m_curRegisterData.doubleD73 = (int)(m_curRegisterData.doubleD73 + 0.5);
        emit sendHistoricalData(QString("%1").arg(m_curRegisterData.doubleD73), indexFeed);

        m_strBackD75 = hex.mid(LEN12*3+LEN16+LEN18*4, LEN18);    //RO电导D75     返回长度18
        m_curRegisterData.doubleD75 = analyzeRegister32(m_strBackD75);
        m_curRegisterData.doubleD75 = (float)(int)(m_curRegisterData.doubleD75*10+0.5)/10;
        emit sendHistoricalData(QString("%1").arg(m_curRegisterData.doubleD75), indexRo);

        m_strBackD71 = hex.mid(LEN12*3+LEN16+LEN18*5, LEN18);    //EDI电阻率D71    返回长度18
        m_curRegisterData.doubleD71 = analyzeRegister32(m_strBackD71);
        m_curRegisterData.doubleD71 = (float)(int)(m_curRegisterData.doubleD71*10+0.5)/10;
        emit sendHistoricalData(QString("%1").arg(m_curRegisterData.doubleD71), indexEDI);

        m_strBackD168 = hex.mid(LEN12*3+LEN16+LEN18*6, LEN18);   //RO弃水压力D168   返回长度18
        m_curRegisterData.doubleD168 = analyzeRegister32(m_strBackD168);
        m_curRegisterData.doubleD168 = (float)(int)(m_curRegisterData.doubleD168*10+0.5)/10;
        emit sendHistoricalData(QString("%1").arg(m_curRegisterData.doubleD168), indexPres);

        m_strBackD122 = hex.mid(LEN12*3+LEN16+LEN18*7, LEN14);   //RO产水流量D122   返回长度18
        m_curRegisterData.doubleD122 = analyzeRegister16(m_strBackD122);
        emit sendHistoricalData(QString("%1").arg(m_curRegisterData.doubleD122), indexRoRate);

        m_strBackD116 = hex.mid(LEN12*3+LEN16+LEN18*7+LEN14, LEN14);   //RO弃水流量D116   返回长度18
        m_curRegisterData.doubleD116 = analyzeRegister16(m_strBackD116);
        emit sendHistoricalData(QString("%1").arg(m_curRegisterData.doubleD116), indexDrainRate);

        m_strBackD166 = hex.mid(LEN12*3+LEN16+LEN18*7+LEN14*2, LEN18);   //纯水箱液位D166   返回长度18
        m_curRegisterData.doubleD166 = analyzeRegister32(m_strBackD166);
        m_curRegisterData.doubleD166 = (int)(m_curRegisterData.doubleD166 + 0.5);

        m_strBackD94 = hex.mid(LEN12*3+LEN16+LEN18*8+LEN14*2, LEN18);    //截留率D94        返回长度18
        m_curRegisterData.doubleD94 = analyzeRegister32(m_strBackD94);
        m_curRegisterData.doubleD94 = (float)(int)(m_curRegisterData.doubleD94*10+0.5)/10;

        m_strBackC120 = hex.mid(LEN12*3+LEN16+LEN18*9+LEN14*2, LEN12);   //更换P Pack  C120：
        m_errorMessage.stateC120 = analyzeSingleRelay(m_strBackC120);

        m_strBackC121 = hex.mid(LEN12*4+LEN16+LEN18*9+LEN14*2, LEN12);   //更换管道紫外灯 C121：
        m_errorMessage.stateC121 = analyzeSingleRelay(m_strBackC121);

        m_strBackC122 = hex.mid(LEN12*5+LEN16+LEN18*9+LEN14*2, LEN12);   //更换水箱空气过滤器C122
        m_errorMessage.stateC122 = analyzeSingleRelay(m_strBackC122);

        m_strBackC123 = hex.mid(LEN12*6+LEN16+LEN18*9+LEN14*2, LEN12);   //更换纯水紫外灯C123：
        m_errorMessage.stateC123 = analyzeSingleRelay(m_strBackC123);

        m_strBackC125 = hex.mid(LEN12*7+LEN16+LEN18*9+LEN14*2, LEN12);   //更换水箱紫外灯C125：
        m_errorMessage.stateC125 = analyzeSingleRelay(m_strBackC125);

        m_strBackC127 = hex.mid(LEN12*8+LEN16+LEN18*9+LEN14*2, LEN12);   //更换AT Pack C127：
        m_errorMessage.stateC127 = analyzeSingleRelay(m_strBackC127);

        m_strBackC128 = hex.mid(LEN12*9+LEN16+LEN18*9+LEN14*2, LEN12);   //更换管道过滤器C128：
        m_errorMessage.stateC128 = analyzeSingleRelay(m_strBackC128);

        m_strBackM240 = hex.mid(LEN12*10+LEN16+LEN18*9+LEN14*2, LEN12);   //EDI产水低于下限M240：
        m_errorMessage.stateM240 = analyzeSingleRelay(m_strBackM240);

        m_strBackM122 = hex.mid(LEN12*11+LEN16+LEN18*9+LEN14*2, LEN12);   //RO产水流速低于设定值M122：
        m_errorMessage.stateM122 = analyzeSingleRelay(m_strBackM122);

        m_strBackX13 = hex.mid(LEN12*12+LEN16+LEN18*9+LEN14*2, LEN12);    //水箱溢流或漏水保护X13：
        qDebug() << m_strBackX13;
        m_errorMessage.stateX13 = analyzeSingleRelay(m_strBackX13);
        qDebug() << m_errorMessage.stateX13;

        m_strBackM242 = hex.mid(LEN12*13+LEN16+LEN18*9+LEN14*2, LEN12);   //截留率低于下限M242
        m_errorMessage.stateM242 = analyzeSingleRelay(m_strBackM242);

        m_strBackM119 = hex.mid(LEN12*14+LEN16+LEN18*9+LEN14*2, LEN12);   //RO压力大于设定值M119：
        m_errorMessage.stateM119 = analyzeSingleRelay(m_strBackM119);

        m_strBackM43 = hex.mid(LEN12*15+LEN16+LEN18*9+LEN14*2, LEN12);    //进水压力低于下限M43：
        m_errorMessage.stateM43 = analyzeSingleRelay(m_strBackM43);

        m_strBackM166 = hex.mid(LEN12*16+LEN16+LEN18*9+LEN14*2, LEN12);   //RO弃水流量低于下限M166：
        m_errorMessage.stateM166 = analyzeSingleRelay(m_strBackM166);

        m_strBackM281 = hex.mid(LEN12*17+LEN16+LEN18*9+LEN14*2, LEN12);   //RO产水电导率大于上限M281：
        m_errorMessage.stateM281 = analyzeSingleRelay(m_strBackM281);

        m_strBackM280 = hex.mid(LEN12*18+LEN16+LEN18*9+LEN14*2, LEN12);   //进水电导率大于上限M280：
        m_errorMessage.stateM280 = analyzeSingleRelay(m_strBackM280);

        m_strBackM155 = hex.mid(LEN12*19+LEN16+LEN18*9+LEN14*2, LEN12);   //RO工作压力低于下限M155
        m_errorMessage.stateM155 = analyzeSingleRelay(m_strBackM155);

        m_strBackD285 = hex.mid(LEN12*20+LEN16+LEN18*9+LEN14*2, LEN14);   //P Pack D285,16位
        m_consumablesStatus.uintD285 = analyzeRegister16(m_strBackD285);

        m_strBackD301 = hex.mid(LEN12*20+LEN16+LEN18*9+LEN14*3, LEN14);   //AT Pack D301,16位
        m_consumablesStatus.uintD301 = analyzeRegister16(m_strBackD301);

        m_strBackD287 = hex.mid(LEN12*20+LEN16+LEN18*9+LEN14*4, LEN14);   //水箱空气过滤器 D287 ,16位
        m_consumablesStatus.uintD287 = analyzeRegister16(m_strBackD287);

        m_strBackD288 = hex.mid(LEN12*20+LEN16+LEN18*9+LEN14*5, LEN14);   //纯水紫外灯 D288,16位
        m_consumablesStatus.uintD288 = analyzeRegister16(m_strBackD288);

        m_strBackD290 = hex.mid(LEN12*20+LEN16+LEN18*9+LEN14*6, LEN14);   // 水箱紫外灯 D290 ,16位
        m_consumablesStatus.uintD290 = analyzeRegister16(m_strBackD290);

        m_strBackD286 = hex.mid(LEN12*20+LEN16+LEN18*9+LEN14*7, LEN14);   //管道紫外灯：D286
        m_consumablesStatus.uintD286 = analyzeRegister16(m_strBackD286);

        m_strBackD277 = hex.mid(LEN12*20+LEN16+LEN18*9+LEN14*8, LEN14);    //管道过滤器：D277
        m_consumablesStatus.uintD277 = analyzeRegister16(m_strBackD277);

        updateAlarmMessage();
    }
    else if(84 == len) //返回指令总长度84
    {
        m_strBackM68 = hex.mid(0, LEN12);    //M68是否处于初始界面 返回长度12
        m_curRelayState.stateM68 = analyzeSingleRelay(m_strBackM68);

        m_strBackM47 = hex.mid(LEN12, LEN12);
        m_curRelayState.stateM47 = analyzeSingleRelay(m_strBackM47);     //M47是否处于维护维修状态 返回长度12

        m_strBackX23 = hex.mid(LEN12*2, LEN16);
        QString strAddressX23 = "3413";
        m_curRelayState.stateX23 = analyzeWriteRelay05Back(m_strBackX23, strAddressX23);

        m_strBackD50 = hex.mid(LEN12*2+LEN16, LEN16);
        QString strAddressD50 = "0032";
        m_curRegisterData.nD50 = analyzeWriteRegister06Back(m_strBackD50, strAddressD50);

        m_strBackYout = hex.mid(LEN12*2+LEN16*2, LEN16);   //Yout输出  返回长度16
        analyzeYout(m_strBackYout);

        m_strBackM37 = hex.mid(LEN12*2+LEN16*3, LEN12);
        m_curRelayState.stateM37 = analyzeSingleRelay(m_strBackM37);

        //update();
    }
    else if(44 == len) //返回指令总长度44
    {
        m_strBackYout = hex.mid(LEN16, LEN16);   //Yout输出  返回长度16
        analyzeYout(m_strBackYout);

        m_strBackM37 = hex.mid(LEN16*2, LEN12);
        m_curRelayState.stateM37 = analyzeSingleRelay(m_strBackM37);
        //update();
    }
    else
    {
        strLogData = strCurTime + tr("  The length of the return data error, length:%1").arg(len) + tr("\n");
        return;
    }

}

//用于解析功能码01读取单个线圈的状态
MainWindow::RelayStateCode MainWindow::analyzeSingleRelay(QString &backHex)
{
    QTextStream out(&logFile);
    QTime currentTime = QTime::currentTime();
    QString strCurTime = currentTime.toString("HH:mm:ss");
    QString strLogData;
    QString buf;
    buf = backHex.mid(0, 2);

    if("01" != buf)    //解析站号
    {
        strLogData = strCurTime + tr(" analyzeSingleRelay(): RelayStateCode::RelayStationError ") + backHex + tr("\n");
        out << strLogData;
        return RelayStateCode::RelayStationError;
    }

    buf = backHex.mid(2, 2);
    if("01" != buf && "02" != buf)   //解析功能码
    {
        strLogData = strCurTime + tr(" analyzeSingleRelay(): RelayStateCode::RelayFunctionCodeError; backHex=") + backHex + tr("\n");
        out << strLogData;
        return RelayStateCode::RelayFunctionCodeError;
    }

    buf = backHex.mid(4, 2);
    if("01" != buf)    //解析返回数据个数
    {
        strLogData = strCurTime + tr(" analyzeSingleRelay(): RelayStateCode::RelayDataCountError ") + tr("\n");
        out << strLogData;
        return RelayStateCode::RelayDataCountError;
    }

    //解析数据
    buf = backHex.mid(6, 2);
    bool ok;
    int intData = buf.toInt(&ok, 16)&0xFF;
    if(!ok)
    {
        strLogData = strCurTime + tr("  analyzeSingleRelay():  A conversion error occurs; buf = ") + buf + tr("\n");
        out << strLogData;
    }

    if(intData & 0x01)
    {
        return RelayStateCode::RelayOn;
    }
    else
    {
        return RelayStateCode::RelayOff;
    }
}

//解析所有Yout输出的情况
void MainWindow::analyzeYout(QString &backHex)
{
    quint32 stateYout;
    stateYout = analyzeMultiRelay(backHex);

    if(stateYout & 0x200000) //E10 Y5
    {
        m_curRelayState.stateE10 = RelayStateCode::RelayOn;
    }
    else
    {
        m_curRelayState.stateE10 = RelayStateCode::RelayOff;
    }

    if(stateYout & 0x000200) //C3 Y11
    {
        m_curRelayState.stateC3 = RelayStateCode::RelayOn;
    }
    else
    {
        m_curRelayState.stateC3 = RelayStateCode::RelayOff;
    }

    if(stateYout & 0x020000) //E1 Y1
    {
        m_curRelayState.stateE1 = RelayStateCode::RelayOn;
    }
    else
    {
        m_curRelayState.stateE1 = RelayStateCode::RelayOff;
    }

    if(stateYout & 0x000100) //C1 Y10
    {
        m_curRelayState.stateC1 = RelayStateCode::RelayOn;
    }
    else
    {
        m_curRelayState.stateC1 = RelayStateCode::RelayOff;
    }

    if(stateYout & 0x040000) //E2 Y2
    {
        m_curRelayState.stateE2 = RelayStateCode::RelayOn;
    }
    else
    {
        m_curRelayState.stateE2 = RelayStateCode::RelayOff;
    }

    if(stateYout & 0x080000) //E9 Y3
    {
        m_curRelayState.stateE9 = RelayStateCode::RelayOn;
    }
    else
    {
        m_curRelayState.stateE9 = RelayStateCode::RelayOff;
    }

    if(stateYout & 0x100000) //E3 Y4
    {
        m_curRelayState.stateE3 = RelayStateCode::RelayOn;
    }
    else
    {
        m_curRelayState.stateE3 = RelayStateCode::RelayOff;
    }

    if(stateYout & 0x000400) //EDI Y12
    {
        m_curRelayState.stateEDI = RelayStateCode::RelayOn;
    }
    else
    {
        m_curRelayState.stateEDI = RelayStateCode::RelayOff;
    }

    if(stateYout & 0x000800) //N1 Y13
    {
        m_curRelayState.stateN1 = RelayStateCode::RelayOn;
    }
    else
    {
        m_curRelayState.stateN1 = RelayStateCode::RelayOff;
    }

    if(stateYout & 0x002000) //N3 Y15
    {
        m_curRelayState.stateN3 = RelayStateCode::RelayOn;
    }
    else
    {
        m_curRelayState.stateN3 = RelayStateCode::RelayOff;
    }

    if(stateYout & 0x000004) //C2 Y22
    {
        m_curRelayState.stateC2 = RelayStateCode::RelayOn;
    }
    else
    {
        m_curRelayState.stateC2 = RelayStateCode::RelayOff;
    }

    if(stateYout & 0x004000) //C4 Y16
    {
        m_curRelayState.stateC4 = RelayStateCode::RelayOn;
    }
    else
    {
        m_curRelayState.stateC4 = RelayStateCode::RelayOff;
    }

    if(stateYout & 0x000001) // E8 Y20
    {
        m_curRelayState.stateE8 = RelayStateCode::RelayOn;
    }
    else
    {
        m_curRelayState.stateE8 = RelayStateCode::RelayOff;
    }

}

//解析功能码06写寄存器返回值
int MainWindow::analyzeWriteRegister06Back(QString &backHex, QString &address)
{
    QTextStream out(&logFile);
    QTime currentTime = QTime::currentTime();
    QString strCurTime = currentTime.toString("HH:mm:ss");
    QString strLogData;

    QString buf;
    buf = backHex.mid(0, 2);

    if("01" != buf)    //解析站号
    {
        strLogData = strCurTime + tr("  analyzeWriteRegister06Back(): RelayStateCode::RelayStationError  ") + tr("\n");
        out << strLogData;
        return RelayStateCode::RelayStationError;
    }

    buf = backHex.mid(2, 2);
    if("06" != buf)   //解析功能码
    {
        strLogData = strCurTime + tr("  analyzeWriteRegister06Back(): RelayStateCode::RelayFunctionCodeError  ") + tr("\n");
        out << strLogData;
        return RelayStateCode::RelayFunctionCodeError;
    }

    buf = backHex.mid(4, 4);
    if(address != buf)    //解析返回数据个数
    {
        strLogData = strCurTime + tr("  analyzeWriteRegister06Back(): RelayStateCode::AddressError  ") + tr("\n");
        out << strLogData;
        return RelayStateCode::AddressError;
    }

    //解析数据
    buf = backHex.mid(8, 4);
    bool ok;
    int intData = buf.toInt(&ok, 16)&0xFFFF;
    if(!ok)
    {
        strLogData = strCurTime + tr("  analyzeWriteRegister06Back():  A conversion error occurs; buf = ") + buf + tr("\n");
        out << strLogData;
    }
    return intData;
}

MainWindow::RelayStateCode MainWindow::analyzeWriteRelay05Back(QString &backHex, QString &address)
{
    QTextStream out(&logFile);
    QTime curTime = QTime::currentTime();
    QString strCurTime = curTime.toString("HH:mm:ss");
    QString strLogData;

    QString buf;
    buf = backHex.mid(0, 2);

    if("01" != buf)    //解析站号
    {
        strLogData = strCurTime + tr("  analyzeWriteRelay05Back(): RelayStateCode::RelayStationError  ") + tr("\n");
        out << strLogData;
        return RelayStateCode::RelayStationError;
    }

    buf = backHex.mid(2, 2);
    if("05" != buf)   //解析功能码
    {
        strLogData = strCurTime + tr("  analyzeWriteRelay05Back: RelayStateCode::RelayFunctionCodeError  ") + tr("\n");
        out << strLogData;
        return RelayStateCode::RelayFunctionCodeError;
    }

    buf = backHex.mid(4, 4);
    if(address != buf)    //解析返回数据个数
    {
        strLogData = strCurTime + tr("  analyzeWriteRelay05Back: RelayStateCode::AddressError  ") + tr("\n");
        out << strLogData;
        return RelayStateCode::AddressError;
    }

    //解析数据
    buf = backHex.mid(8, 4);
    bool ok;
    int intData = buf.toInt(&ok, 16)&0xFFFF;
    if(!ok)
    {
        strLogData = strCurTime + tr("  analyzeWriteRelay05Back():  A conversion error occurs; buf = ") + buf + tr("\n");
        out << strLogData;
    }
    if(intData & 0xFF00)
    {
        return RelayStateCode::RelayOn;
    }
    else
    {
        return RelayStateCode::RelayOff;
    }

}

//用于解析功能码01读取多个线圈的状态
quint32 MainWindow::analyzeMultiRelay(QString &backHex)
{
    QTextStream out(&logFile);
    QTime currentTime = QTime::currentTime();
    QString strCurTime = currentTime.toString("HH:mm:ss");
    QString strLogData;

    QString buf;
    buf = backHex.mid(0, 2);

    if("01" != buf)    //解析站号
    {
        strLogData = strCurTime + tr("  analyzeMultiRelay(): RelayStateCode::RelayStationError\n");
        out << strLogData;
        return RelayStateCode::RelayStationError;
    }

    buf = backHex.mid(2, 2);
    if("01" != buf)   //解析功能码
    {
        strLogData = strCurTime + tr("  analyzeMultiRelay(): RelayStateCode::RelayFunctionCodeError\n");
        out << strLogData;
        return RelayStateCode::RelayFunctionCodeError;
    }

    buf = backHex.mid(4, 2);
    if("03" != buf)    //解析返回数据个数
    {
        strLogData = strCurTime + tr("  analyzeMultiRelay(): RelayStateCode::RelayDataCountError\n");
        out << strLogData;
        return RelayStateCode::RelayDataCountError;
    }

    //解析数据
    buf = backHex.mid(6, 6);
    bool ok;
    quint32 unData =(quint32)buf.toLong(&ok, 16)&0xFFFFFF;
    if(!ok)
    {
        strLogData = strCurTime + tr("  analyzeMultiRelay():  A conversion error occurs; buf = ") + buf + tr("\n");
        out << strLogData;
    }
    return unData;
}

//用于解析16位的寄存器
qreal MainWindow::analyzeRegister16(QString &backHex)
{
    QTextStream out(&logFile);
    QTime curTime = QTime::currentTime();
    QString strCurTime = curTime.toString("HH:mm:ss");
    QString strLogData;

    QString buf;
    buf = backHex.mid(0, 2);

    if("01" != buf)    //解析站号
    {
        strLogData = strCurTime + tr("  analyzeRegister16(): RelayStateCode::RelayDataCountError\n");
        out << strLogData;
        return RelayStateCode::RelayStationError;
    }

    buf = backHex.mid(2, 2);
    if("04" != buf)   //解析功能码
    {
        strLogData = strCurTime + tr("  analyzeRegister16(): RelayStateCode::RelayFunctionCodeError\n");
        out << strLogData;
        return RelayStateCode::RelayFunctionCodeError;
    }

    buf = backHex.mid(4, 2);
    if("02" != buf)    //解析返回数据个数
    {
        strLogData = strCurTime + tr("  analyzeRegister16(): RelayStateCode::RelayDataCountError\n");
        out << strLogData;
        return RelayStateCode::RelayDataCountError;
    }

    //解析数据
    buf = backHex.mid(6, 4);
    bool ok;
    quint16 uintData = buf.toInt(&ok, 16)&0xFFFF;
    if(!ok)
    {
        strLogData = strCurTime + tr("  analyzeRegister16():  A conversion error occurs; buf = ") + buf + tr("\n");
        out << strLogData;
    }
    return uintData;
}
//用于解析32位寄存器（2个16位寄存器）
qreal MainWindow::analyzeRegister32(QString &backHex)
{
    QTextStream out(&logFile);
    QTime currentTime = QTime::currentTime();
    QString strCurTime = currentTime.toString("HH:mm:ss");
    QString strLogData;

    QString buf;
    buf = backHex.mid(0, 2);

    if("01" != buf)    //解析站号
    {
        strLogData = strCurTime + tr("  analyzeRegister32(): RelayStateCode::RelayStationError\n");
        out << strLogData;
        return RelayStateCode::RelayStationError;
    }

    buf = backHex.mid(2, 2);
    if("04" != buf)   //解析功能码
    {
        strLogData = strCurTime + tr("  analyzeRegister32(): RelayStateCode::RelayFunctionCodeError\n");
        out << strLogData;
        return RelayStateCode::RelayFunctionCodeError;
    }

    buf = backHex.mid(4, 2);
    if("04" != buf)    //解析返回数据个数
    {
        strLogData = strCurTime + tr("  analyzeRegister32(): RelayStateCode::RelayDataCountError\n");
        out << strLogData;
        return RelayStateCode::RelayDataCountError;
    }

    //解析数据
    QString lstr = backHex.mid(6, 4);
    QString hstr = backHex.mid(10, 4);
    buf = hstr + lstr;
    bool ok;

    int temp = hstr.toInt(&ok, 16)&0xFFFF;
    if(!ok)
    {
        strLogData = strCurTime + tr("  analyzeRegister32():  A conversion error occurs; temp = ") + buf + tr("\n");
        out << strLogData;
    }
    if(temp & 0x8000) //判断是否为负数，负数则返回0
    {
        return 0;
    }
    else
    {
        DHexFloat hexFloat;
        hexFloat.src = buf.toLong(&ok, 16)&0xFFFFFFFF;
        if(!ok)
        {
            strLogData = strCurTime + tr("  analyzeRegister32():  A conversion error occurs; buf = ") + buf + tr("\n");
            out << strLogData;
        }
        return hexFloat.dest;
    }
}

void MainWindow::drawDestRect(QPainter &painter, int runX, int runY, int runWidth, int runHeight
                              , double xTimes, double yTimes)
{
    //绘制区尺寸：1236, 620
    Q_UNUSED(xTimes);
    painter.setPen(QPen(Qt::green, 1, Qt::DotLine));
    painter.drawRect(runX, runY, runWidth, (runHeight-10)*yTimes); //绘制显示区域外边框
}

void MainWindow::drawDevicePixmap(QPainter &painter, int posX, int posY, double xTimes, double yTimes)
{
    //绘制图标
    painter.drawPixmap(QPointF((posX  + 1150)*xTimes, (posY + 100)*yTimes), pixmapValve); //E10
    painter.drawPixmap(QPointF((posX  + 1075)*xTimes, (posY + 25)*yTimes), pixmapPretreatment); //
    painter.drawPixmap(QPointF((posX  + 1050)*xTimes, (posY + 25)*yTimes), pixmapPretreatment); //
    painter.drawPixmap(QPointF((posX  + 1025)*xTimes, (posY + 25)*yTimes), pixmapPretreatment); //
    if(isFeedTank)
    {
        painter.drawPixmap(QPointF((posX  + 880)*xTimes,  (posY + 10)*yTimes), pixmapTank);    //FeedTank
    }
    painter.drawPixmap(QPointF((posX  + 800)*xTimes,  (posY + 85)*yTimes), pixmapPump);    //C3
    painter.drawPixmap(QPointF((posX  + 700)*xTimes,  (posY + 100)*yTimes), pixmapValve);  //E1
    painter.drawPixmap(QPointF((posX  + 600)*xTimes,  (posY + 103)*yTimes), pixmapPre);    //P1
    painter.drawPixmap(QPointF((posX  + 500)*xTimes,  (posY + 130)*yTimes), pixmapPack);   //P-Pack
    painter.drawPixmap(QPointF((posX  + 350)*xTimes,  (posY + 103)*yTimes), pixmapSensor); //I1
    painter.drawPixmap(QPointF((posX  + 250)*xTimes,  (posY + 85)*yTimes), pixmapPump);    //C1
    painter.drawPixmap(QPointF((posX  + 140)*xTimes,  (posY + 150)*yTimes), pixmapRO);     //RO1
    painter.drawPixmap(QPointF((posX  + 100)*xTimes,  (posY + 220)*yTimes), pixmapRO);     //RO2
    painter.drawPixmap(QPointF((posX  + 200)*xTimes,  (posY + 345)*yTimes), pixmapSensor); //I2
    painter.drawPixmap(QPointF((posX  + 350)*xTimes,  (posY + 342)*yTimes), pixmapValve);  //E9
    painter.drawPixmap(QPointF((posX  + 200)*xTimes,  (posY + 485)*yTimes), pixmapPre);    //P2
    painter.drawPixmap(QPointF((posX  + 350)*xTimes,  (posY + 482)*yTimes), pixmapValve);  //E2
    painter.drawPixmap(QPointF((posX  + 280)*xTimes,  (posY + 295)*yTimes), pixmapRoValve);//E3
    painter.drawPixmap(QPointF((posX  + 340)*xTimes,  (posY + 263)*yTimes), pixmapFlow);   //S2
    painter.drawPixmap(QPointF((posX  + 455)*xTimes,  (posY + 423)*yTimes), pixmapFlow);   //S4
    painter.drawPixmap(QPointF((posX  + 460)*xTimes,  (posY + 282)*yTimes), pixmapPack);  //AT-Pack
    painter.drawPixmap(QPointF((posX  + 570)*xTimes,  (posY + 253)*yTimes), pixmapEDI);   //EDI
    painter.drawPixmap(QPointF((posX  + 700)*xTimes,  (posY + 255)*yTimes), pixmapSensor); //I3
    painter.drawPixmap(QPointF((posX  + 800)*xTimes,  (posY + 262)*yTimes), pixmapUvOff);  //N1
    painter.drawPixmap(QPointF((posX  + 1000)*xTimes, (posY + 290)*yTimes), pixmapTank);  //Tank
    painter.drawPixmap(QPointF((posX  + 935)*xTimes,  (posY + 437)*yTimes), pixmapPump);    //C2
    painter.drawPixmap(QPointF((posX  + 1100)*xTimes, (posY + 437)*yTimes), pixmapPump);    //C4
    painter.drawPixmap(QPointF((posX  + 800)*xTimes,  (posY + 362)*yTimes), pixmapValve);  //E8


    QColor levelColor(40, 150, 200);
    painter.setPen(QPen(levelColor, 1, Qt::DotLine));
    painter.setBrush(QBrush(levelColor));
    painter.setRenderHint(QPainter::Antialiasing, true);
    if(isFeedTank)
    {
        double feedTankAdd = 90*(1 - m_curRegisterData.doubleD340/100);//原水箱液位D340
        QRectF feedTankLevelRect(QPointF((posX  + 884)*xTimes,  (posY + 20 + feedTankAdd)*yTimes),
                                 QPointF((posX  + 928)*xTimes,  (posY + 110)*yTimes)); //FeedTank Level
        painter.drawRect(feedTankLevelRect);
    }

    double tankAdd = 90*(1 - m_curRegisterData.doubleD166/100); //进水压力D160
    QRectF tankLevelRect(QPointF((posX  + 1004)*xTimes, (posY + 300 + tankAdd)*yTimes),
                         QPointF((posX  + 1048)*xTimes, (posY + 390)*yTimes)); //Tank Level
    painter.drawRect(tankLevelRect);
    if(m_curRelayState.stateN1 == RelayOn)
    {
        painter.drawPixmap(QPointF((posX  + 800)*xTimes,  (posY + 262)*yTimes), pixmapUvOn);  //N1
    }
    else
    {
        painter.drawPixmap(QPointF((posX  + 800)*xTimes,  (posY + 262)*yTimes), pixmapUvOff);  //N1
    }

    if(m_curRelayState.stateN3 == RelayStateCode::RelayOn)
    {
        painter.drawPixmap(QPointF((posX  + 1016)*xTimes, (posY + 290)*yTimes), pixmapTankUvOn); //N3
    }
    else
    {
        painter.drawPixmap(QPointF((posX  + 1016)*xTimes, (posY + 290)*yTimes), pixmapTankUvOff); //N3
    }

}

void MainWindow::drawPipeLine(QPainter &painter, int posX, int posY, double xTimes, double yTimes)
{
    //绘制管道
    painter.setPen(QPen(Qt::green, 4, Qt::SolidLine));
    painter.drawLine(QPointF((posX + 1120)*xTimes, (posY + 118)*yTimes), QPointF((posX + 1225)*xTimes, (posY + 118)*yTimes)); //E10进水*
    painter.drawLine(QPointF((posX + 1120)*xTimes, (posY + 30)*yTimes),  QPointF((posX + 1120)*xTimes, (posY + 118)*yTimes));  // E10-预处理处置
    painter.drawLine(QPointF((posX + 1000)*xTimes, (posY + 30)*yTimes),  QPointF((posX + 1120)*xTimes, (posY + 30)*yTimes));   //预处理水平连接
    painter.drawLine(QPointF((posX + 1000)*xTimes, (posY + 30)*yTimes),  QPointF((posX + 1000)*xTimes, (posY + 118)*yTimes));  // 预处理-水箱垂直
    painter.drawLine(QPointF((posX + 536)*xTimes,  (posY + 118)*yTimes), QPointF((posX + 1000)*xTimes, (posY + 118)*yTimes));  //水箱-P柱水平
    painter.drawLine(QPointF((posX + 536)*xTimes,  (posY + 118)*yTimes), QPointF((posX + 536)*xTimes,  (posY + 130)*yTimes));   //P柱垂直右
    painter.drawLine(QPointF((posX + 514)*xTimes,  (posY + 118)*yTimes), QPointF((posX + 514)*xTimes,  (posY + 130)*yTimes));   //P柱垂直左
    painter.drawLine(QPointF((posX + 160)*xTimes,  (posY + 118)*yTimes), QPointF((posX + 514)*xTimes,  (posY + 118)*yTimes));   //P柱-RO柱水平
    painter.drawLine(QPointF((posX + 160)*xTimes,  (posY + 118)*yTimes), QPointF((posX + 160)*xTimes,  (posY + 150)*yTimes));   //C1-RO柱垂直
    painter.drawLine(QPointF((posX + 120)*xTimes,  (posY + 168)*yTimes), QPointF((posX + 140)*xTimes,  (posY + 168)*yTimes));   //RO1-RO2水平
    painter.drawLine(QPointF((posX + 120)*xTimes,  (posY + 168)*yTimes), QPointF((posX + 120)*xTimes,  (posY + 220)*yTimes));   //RO1-RO2垂直
    painter.drawLine(QPointF((posX + 50)*xTimes,   (posY + 238)*yTimes), QPointF((posX + 100)*xTimes,  (posY + 238)*yTimes));    //RO2弃水水平
    painter.drawLine(QPointF((posX + 50)*xTimes,   (posY + 30)*yTimes),  QPointF((posX + 50)*xTimes,   (posY + 500)*yTimes));      //回水弃水垂直
    painter.drawLine(QPointF((posX + 50)*xTimes,   (posY + 30)*yTimes),  QPointF((posX + 450)*xTimes,  (posY + 30)*yTimes));      //回水水平
    painter.drawLine(QPointF((posX + 450)*xTimes,  (posY + 30)*yTimes),  QPointF((posX + 450)*xTimes,  (posY + 118)*yTimes));    //进水电导前回水垂直
    painter.drawLine(QPointF((posX + 120)*xTimes,  (posY + 300)*yTimes), QPointF((posX + 120)*xTimes,  (posY + 360)*yTimes));   //RO2产水
    painter.drawLine(QPointF((posX + 160)*xTimes,  (posY + 220)*yTimes), QPointF((posX + 160)*xTimes,  (posY + 360)*yTimes));   //RO1产水
    painter.drawLine(QPointF((posX + 50)*xTimes,   (posY + 500)*yTimes), QPointF((posX + 420)*xTimes,  (posY + 500)*yTimes));    //弃水水平
    painter.drawLine(QPointF((posX + 120)*xTimes,  (posY + 360)*yTimes), QPointF((posX + 420)*xTimes,  (posY + 360)*yTimes));   //RO2-I2
    painter.drawLine(QPointF((posX + 420)*xTimes,  (posY + 360)*yTimes), QPointF((posX + 420)*xTimes,  (posY + 500)*yTimes));   //E9-E2垂直
    painter.drawLine(QPointF((posX + 420)*xTimes,  (posY + 430)*yTimes), QPointF((posX + 530)*xTimes,  (posY + 430)*yTimes));   //RO弃水排水水平
    painter.drawLine(QPointF((posX + 530)*xTimes,  (posY + 430)*yTimes), QPointF((posX + 530)*xTimes,  (posY + 500)*yTimes));   //RO弃水排水口垂直
    painter.drawLine(QPointF((posX + 299)*xTimes,  (posY + 270)*yTimes), QPointF((posX + 299)*xTimes,  (posY + 360)*yTimes));   //I2-E3垂直
    painter.drawLine(QPointF((posX + 299)*xTimes,  (posY + 270)*yTimes), QPointF((posX + 474)*xTimes,  (posY + 270)*yTimes));   //E3-AT_Pack*
    painter.drawLine(QPointF((posX + 496)*xTimes,  (posY + 270)*yTimes), QPointF((posX + 496)*xTimes,  (posY + 282)*yTimes));   //AT柱垂直右
    painter.drawLine(QPointF((posX + 474)*xTimes,  (posY + 270)*yTimes), QPointF((posX + 474)*xTimes,  (posY + 282)*yTimes));   //AT柱垂直左
    painter.drawLine(QPointF((posX + 496)*xTimes,  (posY + 270)*yTimes), QPointF((posX + 1012)*xTimes, (posY + 270)*yTimes));   //AT_Pack-TANK
    painter.drawLine(QPointF((posX + 1012)*xTimes, (posY + 270)*yTimes), QPointF((posX + 1012)*xTimes, (posY + 300)*yTimes));   //TANK进水垂直
    painter.drawLine(QPointF((posX + 550)*xTimes,  (posY + 270)*yTimes), QPointF((posX + 550)*xTimes,  (posY + 321)*yTimes));   //EDI弃水垂直1
    painter.drawLine(QPointF((posX + 550)*xTimes,  (posY + 321)*yTimes), QPointF((posX + 635)*xTimes,  (posY + 321)*yTimes));   //EDI弃水水平1
    painter.drawLine(QPointF((posX + 635)*xTimes,  (posY + 321)*yTimes), QPointF((posX + 635)*xTimes,  (posY + 380)*yTimes));   //EDI弃水垂直2
    painter.drawLine(QPointF((posX + 545)*xTimes,  (posY + 380)*yTimes), QPointF((posX + 635)*xTimes,  (posY + 380)*yTimes));   //EDI弃水水平2
    painter.drawLine(QPointF((posX + 545)*xTimes,  (posY + 380)*yTimes), QPointF((posX + 545)*xTimes,  (posY + 500)*yTimes));   //EDI弃水垂直3
    painter.drawLine(QPointF((posX + 1025)*xTimes, (posY + 400)*yTimes), QPointF((posX + 1025)*xTimes, (posY + 470)*yTimes));   //TANK出水口垂直
    painter.drawLine(QPointF((posX + 900)*xTimes,  (posY + 470)*yTimes), QPointF((posX + 1200)*xTimes, (posY + 470)*yTimes));   //C2-C4水平
    painter.drawLine(QPointF((posX + 900)*xTimes,  (posY + 380)*yTimes), QPointF((posX + 900)*xTimes,  (posY + 470)*yTimes));   //C2-E8垂直
    painter.drawLine(QPointF((posX + 750)*xTimes,  (posY + 380)*yTimes), QPointF((posX + 900)*xTimes,  (posY + 380)*yTimes));   //C2-E8水平
    painter.drawLine(QPointF((posX + 750)*xTimes,  (posY + 380)*yTimes), QPointF((posX + 750)*xTimes,  (posY + 430)*yTimes));   //EDI取水口
}

void MainWindow::drawDeviceText(QPainter &painter, int posX, int posY, double xTimes, double yTimes)
{
    //绘制文字标号
    if(isAbnormal)  //串口是否打开
    {
        painter.setFont(QFont(tr("隶书"), 16, QFont::Bold));
        painter.setPen(Qt::white);
        painter.drawText(QPointF((posX + 550)*xTimes, (posY - 10)*yTimes), tr("通讯断开"));
    }
    if(isConnect == false && !isAbnormal)  //串口是否打开
    {
        painter.setFont(QFont(tr("隶书"), 16, QFont::Bold));
        painter.setPen(Qt::white);
        painter.drawText(QPointF((posX + 500)*xTimes, (posY - 10)*yTimes), tr("Please initialize the serial port"));
    }
    else if(m_curRelayState.stateM68 == RelayStateCode::RelayOff && !isAbnormal)  //判断系统是否处于初始化界面
    {
        painter.setFont(QFont(tr("隶书"), 16, QFont::Bold));
        painter.setPen(Qt::white);
        painter.drawText(QPointF((posX + 520)*xTimes, (posY - 10)*yTimes), tr("In the initial state of system"));
    }
    else if(m_curRelayState.stateM47 == RelayStateCode::RelayOff && !isAbnormal) //M47为0，设备处于维护维修状态
    {
        painter.setFont(QFont(tr("隶书"), 16, QFont::Bold));
        painter.setPen(Qt::white);
        painter.drawText(QPointF((posX + 500)*xTimes, (posY - 10)*yTimes), tr("System is under maintenance"));
    }
    else if(m_curRelayState.stateM37 == RelayStateCode::RelayOff && !isAbnormal) //M37为0，设备处于运行状态
    {
        painter.setFont(QFont(tr("隶书"), 16, QFont::Bold));
        painter.setPen(Qt::white);
        painter.drawText(QPointF((posX + 580)*xTimes, (posY - 10)*yTimes), tr("Running"));
    }
    else if(m_curRelayState.stateM37 == RelayStateCode::RelayOn && !isAbnormal) //M37为1， 设备处于待机状态
    {
        painter.setFont(QFont(tr("隶书"), 16, QFont::Bold));
        painter.setPen(Qt::white);
        painter.drawText(QPointF((posX + 580)*xTimes, (posY - 10)*yTimes), tr("Standby"));
    }


    painter.setFont(QFont(tr("隶书"), 12, QFont::Bold));
    painter.setPen(Qt::white);
    painter.drawText(QPointF((posX + 1160)*xTimes, (posY + 150)*yTimes), tr("E10"));
//    painter.drawText(QPointF((posX + 870)*xTimes,  (posY + 150)*yTimes), tr("Feed Tank"));
    painter.drawText(QPointF((posX + 810)*xTimes,  (posY + 150)*yTimes), tr("C3"));
    painter.drawText(QPointF((posX + 710)*xTimes,  (posY + 150)*yTimes), tr("E1"));
    painter.drawText(QPointF((posX + 610)*xTimes,  (posY + 150)*yTimes), tr("P1"));
    painter.drawText(QPointF((posX + 356)*xTimes,  (posY + 150)*yTimes), tr("I1"));
    painter.drawText(QPointF((posX + 260)*xTimes,  (posY + 150)*yTimes), tr("C1"));
    painter.drawText(QPointF((posX + 206)*xTimes,  (posY + 392)*yTimes), tr("I2"));
    painter.drawText(QPointF((posX + 360)*xTimes,  (posY + 392)*yTimes), tr("E9"));
    painter.drawText(QPointF((posX + 207)*xTimes,  (posY + 532)*yTimes), tr("P2"));
    painter.drawText(QPointF((posX + 360)*xTimes,  (posY + 532)*yTimes), tr("E2"));
    painter.drawText(QPointF((posX + 465)*xTimes,  (posY + 462)*yTimes), tr("S4"));
    painter.drawText(QPointF((posX + 310)*xTimes,  (posY + 315)*yTimes), tr("E3"));
    painter.drawText(QPointF((posX + 350)*xTimes,  (posY + 302)*yTimes), tr("S2"));
    painter.drawText(QPointF((posX + 705)*xTimes,   (posY + 302)*yTimes), tr("I3"));
    painter.drawText(QPointF((posX + 825)*xTimes,  (posY + 302)*yTimes), tr("N1"));
    painter.drawText(QPointF((posX + 1020)*xTimes, (posY + 285)*yTimes), tr("N3"));
    painter.drawText(QPointF((posX + 945)*xTimes,  (posY + 502)*yTimes), tr("C2"));
    painter.drawText(QPointF((posX + 1110)*xTimes, (posY + 502)*yTimes), tr("C4"));
    painter.drawText(QPointF((posX + 810)*xTimes,  (posY + 412)*yTimes), tr("E8"));
    painter.drawText(QPointF((posX + 589)*xTimes,  (posY + 355)*yTimes), tr("T"));
}

void MainWindow::drawSignsLight(QPainter &painter, int posX, int posY, double xTimes, double yTimes)
{
    qreal r = 8.0;
    painter.setRenderHint(QPainter::Antialiasing, true);
    //背景色rgb(240, 240, 240)
    painter.setBrush(QBrush(Qt::green));
    painter.setPen(QPen(Qt::green));

    if(m_curRelayState.stateE10 == RelayStateCode::RelayOn && isRunE10 == true)
    {
        painter.drawEllipse(QPointF((posX + 1168)*xTimes,  (posY + 85)*yTimes), r, r);  //E10
        isRunE10 = false;
    }
    else if(m_curRelayState.stateE10 == RelayStateCode::RelayOn && isRunE10 == false)
    {
        isRunE10 = true;
    }
    else
    {
        isRunE10 = false;
    }

    if(m_curRelayState.stateC3 == RelayStateCode::RelayOn && isRunC3 == true)
    {
        painter.drawEllipse(QPointF((posX + 818)*xTimes,  (posY + 70)*yTimes), r, r); //C3
        isRunC3 = false;
    }
    else if(m_curRelayState.stateC3 == RelayStateCode::RelayOn && isRunC3 == false)
    {
        isRunC3 = true;
    }
    else
    {
        isRunC3 = false;
    }

    if(m_curRelayState.stateE1 == RelayStateCode::RelayOn && isRunE1 == true)
    {
        painter.drawEllipse(QPointF((posX + 718)*xTimes,  (posY + 85)*yTimes), r, r);  //E1
        isRunE1 = false;
    }
    else if(m_curRelayState.stateE1 == RelayStateCode::RelayOn && isRunE1 == false)
    {
        isRunE1 = true;
    }
    else
    {
        isRunE1 = false;
    }

    if(m_curRelayState.stateC1 == RelayStateCode::RelayOn && isRunC1 == true)
    {
        painter.drawEllipse(QPointF((posX + 268)*xTimes,  (posY + 70)*yTimes), r, r); //C1
        isRunC1 = false;
    }
    else if(m_curRelayState.stateC1 == RelayStateCode::RelayOn && isRunC1 == false)
    {
        isRunC1 = true;
    }
    else
    {
        isRunC1 = false;
    }

    if(m_curRelayState.stateE9 == RelayStateCode::RelayOn && isRunE9 == true)
    {
        painter.drawEllipse(QPointF((posX + 368)*xTimes,  (posY + 327)*yTimes), r, r); //E9
        isRunE9 = false;
    }
    else if(m_curRelayState.stateE9 == RelayStateCode::RelayOn && isRunE9 == false)
    {
        isRunE9 = true;
    }
    else
    {
        isRunE9 = false;
    }

    if(m_curRelayState.stateE2 == RelayStateCode::RelayOn && isRunE2 == true)
    {
        painter.drawEllipse(QPointF((posX + 368)*xTimes,  (posY + 467)*yTimes), r, r); //E2
        isRunE2 = false;
    }
    else if(m_curRelayState.stateE2 == RelayStateCode::RelayOn && isRunE2 == false)
    {
        isRunE2 = true;
    }
    else
    {
        isRunE2 = false;
    }

    if(m_curRelayState.stateE3 == RelayStateCode::RelayOn && isRunE3 == true)
    {
        painter.drawEllipse(QPointF((posX + 265)*xTimes,  (posY + 290)*yTimes), r, r); //E3
        isRunE3 = false;
    }
    else if(m_curRelayState.stateE3 == RelayStateCode::RelayOn && isRunE3 == false)
    {
        isRunE3 = true;
    }
    else
    {
        isRunE3 = false;
    }

    if(m_curRelayState.stateEDI == RelayStateCode::RelayOn && isRunEDI == true)
    {
        painter.drawEllipse(QPointF((posX + 590)*xTimes,  (posY + 238)*yTimes), r, r); //EDI
        isRunEDI = false;
    }
    else if(m_curRelayState.stateEDI == RelayStateCode::RelayOn && isRunEDI == false)
    {
        isRunEDI = true;
    }
    else
    {
        isRunEDI = false;
    }

    if(m_curRelayState.stateC2 == RelayStateCode::RelayOn && isRunC2 == true)
    {
        painter.drawEllipse(QPointF((posX + 953)*xTimes,  (posY + 422)*yTimes), r, r); //C2
        isRunC2 = false;
    }
    else if(m_curRelayState.stateC2 == RelayStateCode::RelayOn && isRunC2 == false)
    {
        isRunC2 = true;
    }
    else
    {
        isRunC2 = false;
    }

    if(m_curRelayState.stateC4 == RelayStateCode::RelayOn && isRunC4 == true)
    {
        painter.drawEllipse(QPointF((posX + 1118)*xTimes,  (posY + 422)*yTimes), r, r); //C4
        isRunC4 = false;
    }
    else if(m_curRelayState.stateC4 == RelayStateCode::RelayOn && isRunC4 == false)
    {
        isRunC4 = true;
    }
    else
    {
        isRunC4 = false;
    }

    if(m_curRelayState.stateE8 == RelayStateCode::RelayOn && isRunE8 == true)
    {
        painter.drawEllipse(QPointF((posX + 818)*xTimes,  (posY + 347)*yTimes), r, r); //E8
        isRunE8 = false;
    }
    else if(m_curRelayState.stateE8 == RelayStateCode::RelayOn && isRunE8 == false)
    {
        isRunE8 = true;
    }
    else
    {
        isRunE8 = false;
    }
}

void MainWindow::drawParameter(QPainter &painter, int posX, int posY, double xTimes, double yTimes)
{
    painter.setFont(QFont(tr("隶书"), 14, QFont::Bold));
    painter.setBrush(QBrush(QColor(255, 0, 0)));
//    painter.setPen(QPen(QColor(30, 30, 210)));
    painter.setPen(Qt::white);
    if(isFeedTank)
    {
        painter.drawText(QPointF((posX + 945)*xTimes, (posY + 80)*yTimes),
                         tr("%1%").arg(m_curRegisterData.doubleD340));  //原水箱液位D340
    }

    painter.drawText(QPointF((posX + 600)*xTimes,  (posY + 95)*yTimes),
                     tr("%1bar").arg(m_curRegisterData.doubleD160));  //进水压力D160

    painter.drawText(QPointF((posX + 345)*xTimes,  (posY + 70)*yTimes),
                     tr("%1").arg(m_curRegisterData.doubleD96));   //温度D96
    painter.drawText(QPointF((posX + 390)*xTimes,  (posY + 70)*yTimes),
                     tr("SS"));                          //翻译成温度摄氏度符号

    painter.drawText(QPointF((posX + 345)*xTimes,  (posY + 95)*yTimes),
                     tr("%1us/cm").arg(m_curRegisterData.doubleD73));  //进水电导D73

    painter.drawText(QPointF((posX + 175)*xTimes,  (posY + 335)*yTimes),
                     tr("%1us/cm").arg(m_curRegisterData.doubleD75));  //RO电导D75

    painter.drawText(QPointF((posX + 700)*xTimes,  (posY + 245)*yTimes),
                     tr("%1MΩ.cm").arg(m_curRegisterData.doubleD71));  //EDI电阻率D71

    painter.drawText(QPointF((posX + 200)*xTimes,  (posY + 475)*yTimes),
                     tr("%1bar").arg(m_curRegisterData.doubleD168));  //RO弃水压力D168

    painter.drawText(QPointF((posX + 340)*xTimes,  (posY + 250)*yTimes),
                     tr("%1L/hr").arg(m_curRegisterData.doubleD122));  //RO产水流量D122

    painter.drawText(QPointF((posX + 455)*xTimes,  (posY + 410)*yTimes),
                     tr("%1L/hr").arg(m_curRegisterData.doubleD116));  //RO弃水流量D116

    painter.drawText(QPointF((posX + 1060)*xTimes,  (posY + 350)*yTimes),
                     tr("%1%").arg(m_curRegisterData.doubleD166));  //纯水箱液位D166

}

void MainWindow::initMenuBar()
{
    //0, 55, 120
    ui->menuBar->setMinimumHeight(25);
    QString qmlMenuBar = "QMenuBar{background-color:#003778;\
                                   color:white;\
                                   font-family:仿宋;\
                                   font-size: 18px}\
                          QMenuBar:item:selected{background-color:#436EEE;\
                                                 border-radius: 5px;}";
    ui->menuBar->setStyleSheet(qmlMenuBar);

    QString qmlMenu = "QMenu{background-color:#003778;\
                             color:white;}\
                       QMenu:item:selected{color:#969696;}";

    QMenu *configMenu = ui->menuBar->addMenu(tr("操作"));
    configMenu->setStyleSheet(qmlMenu);

    QAction *configAction = new QAction(QIcon(":/images/pic/config.png"), tr("配置串口"), this);
    connect(configAction, SIGNAL(triggered()), this, SLOT(onConfigActiontriggered()));

    QAction *runAction = new QAction(QIcon(":/images/pic/run.png"), tr("运行"), this);
    connect(runAction, SIGNAL(triggered()), this, SLOT(on_runPushButton_clicked()));

    QAction *standbyAction = new QAction(QIcon(":/images/pic/standby.png"), tr("待机"), this);
    connect(standbyAction, SIGNAL(triggered()), this, SLOT(on_standbyPushButton_clicked()));

    QAction *cmdTestAction = new QAction(QIcon(":/images/pic/test.png"), tr("通讯测试"), this);
    connect(cmdTestAction, SIGNAL(triggered()), this, SLOT(onCmdTesttriggered()));

    QAction *closeConnectAction = new QAction(QIcon(":/images/pic/closeConnect.png"), tr("断开连接"), this);
    connect(closeConnectAction, SIGNAL(triggered()), this, SLOT(onCloseConnectActiontriggered()));

    QList<QAction*> configMenuList;
    configMenuList << configAction << runAction << standbyAction << cmdTestAction << closeConnectAction;
    configMenu->addActions(configMenuList);
    configMenu->insertSeparator(cmdTestAction);

    QMenu *parameterMenu = ui->menuBar->addMenu(tr("参数"));
    parameterMenu->setStyleSheet(qmlMenu);
    QAction *parameterAction = new QAction(QIcon(":/images/pic/runPara.png"), tr("运行参数"), this);
    connect(parameterAction, SIGNAL(triggered()), this, SLOT(onParameterActiontriggered()));

    QAction *consumablesAction = new QAction(QIcon(":/images/pic/consumables.png"), tr("耗材信息"), this);
    connect(consumablesAction, SIGNAL(triggered()), this, SLOT(onConsumablesActiontriggered()));

    QAction *alarmMsgAction = new QAction(QIcon(":/images/pic/alarm.png"), tr("报警信息"), this);
    connect(alarmMsgAction, SIGNAL(triggered()), this, SLOT(onAlarmMessageActiontriggered()));

    QAction *historyAction = new QAction(QIcon(":/images/pic/histroy.png"), tr("历史数据"), this);
    connect(historyAction, SIGNAL(triggered()), this, SLOT(onHistoricalDataActiontriggered()));

    QList<QAction*> parameterMenuList;
    parameterMenuList << parameterAction << consumablesAction << alarmMsgAction << historyAction;
    parameterMenu->addActions(parameterMenuList);

    QMenu *appendixMenu = ui->menuBar->addMenu(tr("显示"));
    appendixMenu->setStyleSheet(qmlMenu);
    QAction *feedTankAction = new QAction(tr("Feed Tank"), this);
    feedTankAction->setCheckable(true);
    feedTankAction->setChecked(true);
    connect(feedTankAction, SIGNAL(toggled(bool)), this, SLOT(onFeedTankConfigtoggle(bool)));
    appendixMenu->addAction(feedTankAction);

    QMenu *helpMenu = ui->menuBar->addMenu("帮助");
    helpMenu->setStyleSheet(qmlMenu);
    QAction *versionAction = new QAction(QIcon(":/images/pic/about.png"), tr("关于.."), this);
    connect(versionAction, SIGNAL(triggered()), this, SLOT(onVersionActiontriggered()));
    QAction *helpAction = new QAction(QIcon(":/images/pic/help.png"), tr("帮助"), this);
    connect(helpAction, SIGNAL(triggered()), this, SLOT(onHelpActiontriggered()));
    helpMenu->addAction(versionAction);
    helpMenu->addAction(helpAction);

    QString qmlToolBar = "QMenu::separator{background: white;}";
    ui->mainToolBar->addAction(configAction);
    ui->mainToolBar->addAction(runAction);
    ui->mainToolBar->addAction(standbyAction);
    ui->mainToolBar->addAction(parameterAction);
    ui->mainToolBar->addAction(consumablesAction);
    ui->mainToolBar->addAction(alarmMsgAction);
    ui->mainToolBar->addAction(historyAction);
    ui->mainToolBar->insertSeparator(configAction);
    ui->mainToolBar->insertSeparator(parameterAction);
    ui->mainToolBar->setMovable(false);
    ui->mainToolBar->setStyleSheet(qmlToolBar);
}

void MainWindow::initStatusBar()
{
    QLabel *statusLabel = new QLabel;
    statusLabel->setFrameStyle(QFrame::Box | QFrame::Sunken);
    statusLabel->setText(tr("<a href=\"http://www.rephile.cn\" style=\"color:white;text-decoration:none;\">上海乐枫生物科技有限公司</a>"));
    statusLabel->setStyleSheet("link-color:#FF0000;text-decoration:underline;");
    statusLabel->setTextFormat(Qt::RichText);
    statusLabel->setOpenExternalLinks(true);
    QPalette palette = statusLabel->palette();
    palette.setBrush(QPalette::Text, QBrush(Qt::white));
    statusLabel->setPalette(palette);
    ui->statusBar->addPermanentWidget(statusLabel);

//    QString qmlStatus = "QStatusBar{background-color:green;}";
//    ui->statusBar->setStyleSheet(qmlStatus);
}

void MainWindow::initLogFile()
{
    QDate currentDate = QDate::currentDate();
    QString strDate = currentDate.toString("yyyyMM"); //log文件每月创建一个
    QString logFileName;
    QString pathName = QDir::currentPath() + QString("/") +QString("logging");
    if(!isPathExist(pathName))
    {
        QMessageBox::warning(this, tr("提示"), tr("创建目录失败"), QMessageBox::Ok);
        logFileName = tr("log") + strDate + tr(".log");
    }
    else
    {
        logFileName = pathName + QString("/") + QString("log") + strDate + QString(".log");
    }
    logFile.setFileName(logFileName);
    if(!logFile.open(QIODevice::Append|QIODevice::WriteOnly|QIODevice::Text))
    {
        QMessageBox::warning(this, tr("提示"), tr("写log文件失败"), QMessageBox::Ok);
    }
}

void MainWindow::initRelayState()
{
    m_curRelayState = {RelayStateCode::RelayOn,   //    RelayStateCode stateM68;
                       RelayStateCode::RelayOn,   //    RelayStateCode stateM47;
                       RelayStateCode::RelayOn,  //    RelayStateCode stateM37;
                       RelayStateCode::RelayOff,  //    RelayStateCode stateX23;
                       RelayStateCode::RelayOff,  //    RelayStateCode stateE1;
                       RelayStateCode::RelayOff,  //    RelayStateCode stateE2;
                       RelayStateCode::RelayOff,  //    RelayStateCode stateE3;
                       RelayStateCode::RelayOff,  //    RelayStateCode stateE8;
                       RelayStateCode::RelayOff,  //    RelayStateCode stateE9;
                       RelayStateCode::RelayOff,  //    RelayStateCode stateE10;
                       RelayStateCode::RelayOff,  //    RelayStateCode stateC1;
                       RelayStateCode::RelayOff,  //    RelayStateCode stateC2;
                       RelayStateCode::RelayOff,  //    RelayStateCode stateC3;
                       RelayStateCode::RelayOff,  //    RelayStateCode stateC4;
                       RelayStateCode::RelayOff,  //    RelayStateCode stateEDI;
                       RelayStateCode::RelayOff,  //    RelayStateCode stateN1;
                       RelayStateCode::RelayOff   //    RelayStateCode stateN3;
                      };
    m_curRegisterData = {0};
    m_consumablesStatus = {0};
}

void MainWindow::initGroupBox()
{
    //font-weight:bloder;
    QString qss1="QGroupBox {\
    border: 2px solid #FFFFFF;\
    border-radius: 5px;\
    margin-top: 1ex; \
    font-family:仿宋;\
    color:white;\
    font-size:16px;\
    }\
    QGroupBox::title {\
    subcontrol-origin: margin;\
    subcontrol-position: top center;\
    padding: 0 3px;\
    }";
    QString qss2="QGroupBox {\
    font-family:隶书;\
    color:white;\
    font-size:20px;\
    }";
    ui->operateGroupBox->setStyleSheet(qss2);
    }

void MainWindow::initButton()
{
    QString qmlBtn = "QPushButton{background-color:white;\
                                  color:#003778;\
                                  font-family:宋体;\
                                  width:80px;  \
                                  height:30px; \
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
    ui->configPushButton->setStyleSheet(qmlBtn);
    ui->runPushButton->setStyleSheet(qmlBtn);
    ui->standbyPushButton->setStyleSheet(qmlBtn);
}

void MainWindow::initScrollerLabel()
{
    m_scrollerLabel = new DScrollerLabel(this);
    QString qmlLabel = "QLabel{color:white;\
                               font-size:18px;\
                               font-family:隶书; \
                              }";
    m_scrollerLabel->setStyleSheet(qmlLabel);

    m_strLabelList.listStringClear();
}

void MainWindow::updateAlarmMessage()
{  
//    if(RelayStateCode::RelayOn == m_errorMessage.stateC120)  //更换P Pack  C120：
//    {
//        m_strLabelList.removeFixItem(tr("Exchange P Pack"));
//        m_strLabelList << tr("Exchange P Pack");
//    }
//    else
//    {
//        m_strLabelList.removeFixItem(tr("Exchange P Pack"));
//    }

//    if(RelayStateCode::RelayOn == m_errorMessage.stateC121)  //更换管道紫外灯 C121：
//    {
//        m_strLabelList.removeFixItem(tr("Exchange Pipe UV Lamp"));
//        m_strLabelList << tr("Exchange Pipe UV Lamp");
//    }
//    else
//    {
//        m_strLabelList.removeFixItem(tr("Exchange Pipe UV Lamp"));
//    }

//    if(RelayStateCode::RelayOn == m_errorMessage.stateC122)  //更换水箱空气过滤器C122
//    {
//        m_strLabelList.removeFixItem(tr("Exchange Tank Vent Filter"));
//        m_strLabelList << tr("Exchange Tank Vent Filter");
//    }
//    else
//    {
//        m_strLabelList.removeFixItem(tr("Exchange Tank Vent Filter"));
//    }

//    if(RelayStateCode::RelayOn == m_errorMessage.stateC123) //更换纯水紫外灯C123：
//    {
//        m_strLabelList.removeFixItem(tr("Exchange EDI UV Lamp"));
//        m_strLabelList << tr("Exchange EDI UV Lamp");
//    }
//    else
//    {
//        m_strLabelList.removeFixItem(tr("Exchange EDI UV Lamp"));
//    }

//    if(RelayStateCode::RelayOn == m_errorMessage.stateC125) //更换水箱紫外灯C125：
//    {
//        m_strLabelList.removeFixItem(tr("Exchange Tank UV Lamp"));
//        m_strLabelList << tr("Exchange Tank UV Lamp");
//    }
//    else
//    {
//        m_strLabelList.removeFixItem(tr("Exchange Tank UV Lamp"));
//    }

//    if(RelayStateCode::RelayOn == m_errorMessage.stateC127)   //更换AT Pack C127：
//    {
//        m_strLabelList.removeFixItem(tr("Exchange AT Pack"));
//        m_strLabelList << tr("Exchange AT Pack");
//    }
//    else
//    {
//        m_strLabelList.removeFixItem(tr("Exchange AT Pack"));
//    }

//    if(RelayStateCode::RelayOn == m_errorMessage.stateC128)  //更换管道过滤器C128：
//    {
//        m_strLabelList.removeFixItem(tr("Exchange Pipe Filter"));
//        m_strLabelList << tr("Exchange Pipe Filter");
//    }
//    else
//    {
//        m_strLabelList.removeFixItem(tr("Exchange Pipe Filter"));
//    }

//    if(RelayStateCode::RelayOn == m_errorMessage.stateM240)  //EDI产水低于下限M240：
//    {
//        m_strLabelList.removeFixItem(tr("EDI Product Resis. < Lower Limit"));
//        m_strLabelList << tr("EDI Product Resis. < Lower Limit");
//        emit sendAlarmMsg(tr("EDI Product Resis. < Lower Limit"));
//    }
//    else
//    {
//        m_strLabelList.removeFixItem(tr("EDI Product Resis. < Lower Limit"));
//    }

//    if(RelayStateCode::RelayOn == m_errorMessage.stateM122)  //RO产水流速低于设定值M122：
//    {
//        m_strLabelList.removeFixItem(tr("RO Production Rate < SP"));
//        m_strLabelList << tr("RO Production Rate < SP");
//        emit sendAlarmMsg(tr("RO Production Rate < SP"));
//    }
//    else
//    {
//        m_strLabelList.removeFixItem(tr("RO Production Rate < SP"));
//    }

//    if(RelayStateCode::RelayOn == m_errorMessage.stateX13)   //水箱溢流或漏水保护X13：
//    {
//        m_strLabelList.removeFixItem(tr("Leak or Tank Overflow"));
//        m_strLabelList << tr("Leak or Tank Overflow");
//        emit sendAlarmMsg(tr("Leak or Tank Overflow"));
//    }
//    else
//    {
//        m_strLabelList.removeFixItem(tr("Leak or Tank Overflow"));
//    }

//    if(RelayStateCode::RelayOn == m_errorMessage.stateM242)  //截留率低于下限M242
//    {
//        m_strLabelList.removeFixItem(tr("RO Rej. Rate < Lower Limit"));
//        m_strLabelList << tr("RO Rej. Rate < Lower Limit");
//        emit sendAlarmMsg(tr("RO Rej. Rate < Lower Limit"));
//    }
//    else
//    {
//        m_strLabelList.removeFixItem(tr("RO Rej. Rate < Lower Limit"));
//    }

//    if(RelayStateCode::RelayOn == m_errorMessage.stateM119) //RO压力大于设定值M119：
//    {
//        m_strLabelList.removeFixItem(tr("RO Pressure > Upper Limit"));
//        m_strLabelList << tr("RO Pressure > Upper Limit");
//        emit sendAlarmMsg(tr("RO Pressure > Upper Limit"));
//    }
//    else
//    {
//        m_strLabelList.removeFixItem(tr("RO Pressure > Upper Limit"));
//    }

//    if(RelayStateCode::RelayOn == m_errorMessage.stateM43)   //进水压力低于下限M43：
//    {
//        m_strLabelList.removeFixItem(tr("Feed Pressure < Lower Limit"));
//        m_strLabelList << tr("Feed Pressure < Lower Limit");
//        emit sendAlarmMsg(tr("Feed Pressure < Lower Limit"));
//    }
//    else
//    {
//        m_strLabelList.removeFixItem(tr("Feed Pressure < Lower Limit"));
//    }

//    if(RelayStateCode::RelayOn == m_errorMessage.stateM166)  //RO弃水流量低于下限M166：
//    {
//        m_strLabelList.removeFixItem(tr("RO Drain Rate < Lower Limit"));
//        m_strLabelList << tr("RO Drain Rate < Lower Limit");
//        emit sendAlarmMsg(tr("RO Drain Rate < Lower Limit"));
//    }
//    else
//    {
//        m_strLabelList.removeFixItem(tr("RO Drain Rate < Lower Limit"));
//    }

//    if(RelayStateCode::RelayOn == m_errorMessage.stateM281)   //RO产水电导率大于上限M281：
//    {
//        m_strLabelList.removeFixItem(tr("RO Conductivity > Upper Limit"));
//        m_strLabelList << tr("RO Conductivity > Upper Limit");
//        emit sendAlarmMsg(tr("RO Conductivity > Upper Limit"));
//    }
//    else
//    {
//        m_strLabelList.removeFixItem(tr("RO Conductivity > Upper Limit"));
//    }

//    if(RelayStateCode::RelayOn == m_errorMessage.stateM280)   //进水电导率大于上限M280：
//    {
//        m_strLabelList.removeFixItem(tr("Feed Conductivity > Upper Limit"));
//        m_strLabelList << tr("Feed Conductivity > Upper Limit");
//        emit sendAlarmMsg(tr("Feed Conductivity > Upper Limit"));
//    }
//    else
//    {
//        m_strLabelList.removeFixItem(tr("Feed Conductivity > Upper Limit"));
//    }

//    if(RelayStateCode::RelayOn == m_errorMessage.stateM155)  //RO工作压力低于下限M155
//    {
//        m_strLabelList.removeFixItem(tr("RO Pressure < Lower Limit"));
//        m_strLabelList << tr("RO Pressure < Lower Limit");;
//        emit sendAlarmMsg(tr("RO Pressure < Lower Limit"));
//    }
//    else
//    {
//        m_strLabelList.removeFixItem(tr("RO Pressure < Lower Limit"));
//    }

//    m_scrollerLabel->setShowText(m_strLabelList.listToString());
//    qDebug() << m_strLabelList.listToString();
}

void MainWindow::loadHexCmd()
{
    m_HexCmd.hexReadM68 = "01 01 00 44 00 01 BD DF";             //为0说明在初始页面，不能操作机器
    m_HexCmd.hexReadM47 = "01 01 00 2F 00 01 CC 03";             //为0说明设备处于维护维修状态
    m_HexCmd.hexReadM37 = "01 01 00 25 00 01 EC 01";             //判断程序是否在待机状态
    m_HexCmd.hexOnX23 = "01 05 34 13 FF 00 73 CF";               //置位X23，PLC跳转到运行状态
    m_HexCmd.hexWriteD50 = "01 06 00 32 00 47 68 37";            //写入71，界面跳转到运行界面
    m_HexCmd.hexOnX7 = "01 05 34 07 FF 00 33 CB";                //X7置1，返回待机界面
    m_HexCmd.hexReadYout = "01 01 33 00 00 18 33 44";            //读取Y口输出
    m_HexCmd.hexReadD340 = "01 04 01 54 00 02 31 E7";            //原水箱液位D340
    m_HexCmd.hexReadD160 = "01 04 00 A0 00 02 71 E9";            //进水压力D160
    m_HexCmd.hexReadD96 = "01 04 00 60 00 02 71 D5";             //温度D96
    m_HexCmd.hexReadD73 = "01 04 00 49 00 02 A0 1D ";            //进水电导D73
    m_HexCmd.hexReadD75 = "01 04 00 4B 00 02 01 DD";             //RO电导D75
    m_HexCmd.hexReadD71 = "01 04 00 47 00 02 C1 DE";             //EDI电阻率D71
    m_HexCmd.hexReadD168 = "01 04 00 A8 00 02 F0 2B";            //RO弃水压力D168
    m_HexCmd.hexReadD122 = "01 04 00 7A 00 01 10 13";            //RO产水流量D122
    m_HexCmd.hexReadD116 = "01 04 00 74 00 01 71 D0";            //RO弃水流量D116
    m_HexCmd.hexReadD166 = "01 04 00 A6 00 02 91 E8";            //纯水箱液位D166
    m_HexCmd.hexReadD94 = "01 04 00 5E 00 02 10 19";             //截留率D94
    //报警信息：
    m_HexCmd.hexReadC120 = "01 01 32 78 00 01 73 6B";   //更换P Pack  C120：
    m_HexCmd.hexReadC121 = "01 01 32 79 00 01 22 AB";   //更换管道紫外灯 C121：
    m_HexCmd.hexReadC122 = "01 01 32 7A 00 01 D2 AB";   //更换水箱空气过滤器C122：
    m_HexCmd.hexReadC123 = "01 01 32 7B 00 01 83 6B";   //更换纯水紫外灯C123：
    m_HexCmd.hexReadC125 = "01 01 32 7D 00 01 63 6A";   //更换水箱紫外灯C125：
    m_HexCmd.hexReadC127 = "01 01 32 7F 00 01 C2 AA";   //更换AT Pack C127：
    m_HexCmd.hexReadC128 = "01 01 32 80 00 01 F2 9A";   //更换管道过滤器C128：

    m_HexCmd.hexReadM240 = "01 01 00 F0 00 01 FD F9";   //EDI产水低于下限M240：
    m_HexCmd.hexReadM122 = "01 01 00 7A 00 01 DC 13";   //RO产水流速低于设定值M122：
//    m_HexCmd.hexReadX13 = "01 02 00 0C 00 01 79 C9";    //水箱溢流或漏水保护X13：
    m_HexCmd.hexReadX13 = "01 02 34 0B 00 01 C6 38";     //水箱溢流或漏水保护X13：
    m_HexCmd.hexReadM242 = "01 01 00 F2 00 01 5C 39";   //截留率低于下限M242：
    m_HexCmd.hexReadM119 = "01 01 00 77 00 01 4D D0";   //RO压力大于设定值M119：
    m_HexCmd.hexReadM43 = " 01 01 00 2B 00 01 8D C2";    //进水压力低于下限M43：
    m_HexCmd.hexReadM166 = "01 01 00 A6 00 01 1D E9";   //RO弃水流量低于下限M166：
    m_HexCmd.hexReadM281 = "01 01 01 19 00 01 2D F1";   //RO产水电导率大于上限M281：
    m_HexCmd.hexReadM280 = "01 01 01 18 00 01 7C 31";   //进水电导率大于上限M280：
    m_HexCmd.hexReadM155 = "01 01 00 9B 00 01 8C 25";   //RO工作压力低于下限M155

    m_HexCmd.hexReadD285 = "01 04 01 1D 00 01 A0 30";   //P Pack D285,16位
    m_HexCmd.hexReadD301 = "01 04 01 2D 00 01 A0 3F";   //AT Pack D301,16位
    m_HexCmd.hexReadD287 = "01 04 01 1F 00 01 01 F0";   //水箱空气过滤器 D287 ,16位
    m_HexCmd.hexReadD288 = "01 04 01 20 00 01 31 FC";   //纯水紫外灯 D288,16位
    m_HexCmd.hexReadD290 = "01 04 01 22 00 01 90 3C";   // 水箱紫外灯 D290 ,16位
    m_HexCmd.hexReadD286 = "01 04 01 1E 00 01 50 30";   //管道紫外灯：D286
    m_HexCmd.hexReadD277 = "01 04 01 15 00 01 21 F2";    //管道过滤器：D277
}

void MainWindow::writeHexList()
{
    //M68是否处于初始界面-M47是否处于维修维护状态--M37是否处于运行状态-Yout输出
    //原水箱液位D340-进水压力D160-温度D96-进水电导D73-RO电导D75
    //EDI电阻率D71-RO弃水压力D168-RO产水流量D122-RO弃水流量D116
    //纯水箱液位D166-截留率D94
    writeHex(m_HexCmd.hexReadM68);
    sleep(50);
    writeHex(m_HexCmd.hexReadM47);
    sleep(50);
    writeHex(m_HexCmd.hexReadM37);
    sleep(50);
    writeHex(m_HexCmd.hexReadYout);
    sleep(50);
    writeHex(m_HexCmd.hexReadD340);
    sleep(50);
    writeHex(m_HexCmd.hexReadD160);
    sleep(50);
    writeHex(m_HexCmd.hexReadD96);
    sleep(50);
    writeHex(m_HexCmd.hexReadD73);
    sleep(50);
    writeHex(m_HexCmd.hexReadD75);
    sleep(50);
    writeHex(m_HexCmd.hexReadD71);
    sleep(50);
    writeHex(m_HexCmd.hexReadD168);
    sleep(50);
    writeHex(m_HexCmd.hexReadD122);
    sleep(50);
    writeHex(m_HexCmd.hexReadD116);
    sleep(50);
    writeHex(m_HexCmd.hexReadD166);
    sleep(50);
    writeHex(m_HexCmd.hexReadD94);
    sleep(50);
    writeHex(m_HexCmd.hexReadC120);
    sleep(50);
    writeHex(m_HexCmd.hexReadC121);
    sleep(50);
    writeHex(m_HexCmd.hexReadC122);
    sleep(50);
    writeHex(m_HexCmd.hexReadC123);
    sleep(50);
    writeHex(m_HexCmd.hexReadC125);
    sleep(50);
    writeHex(m_HexCmd.hexReadC127);
    sleep(50);
    writeHex(m_HexCmd.hexReadC128);
    sleep(50);
    writeHex(m_HexCmd.hexReadM240);
    sleep(50);
    writeHex(m_HexCmd.hexReadM122);
    sleep(50);
    writeHex(m_HexCmd.hexReadX13);
    sleep(50);
    writeHex(m_HexCmd.hexReadM242);
    sleep(50);
    writeHex(m_HexCmd.hexReadM119);
    sleep(50);
    writeHex(m_HexCmd.hexReadM43);
    sleep(50);
    writeHex(m_HexCmd.hexReadM166);
    sleep(50);
    writeHex(m_HexCmd.hexReadM281);
    sleep(50);
    writeHex(m_HexCmd.hexReadM280);
    sleep(50);
    writeHex(m_HexCmd.hexReadM155);
    sleep(50);
    writeHex(m_HexCmd.hexReadD285);
    sleep(50);
    writeHex(m_HexCmd.hexReadD301);
    sleep(50);
    writeHex(m_HexCmd.hexReadD287);
    sleep(50);
    writeHex(m_HexCmd.hexReadD288);
    sleep(50);
    writeHex(m_HexCmd.hexReadD290);
    sleep(50);
    writeHex(m_HexCmd.hexReadD286);
    sleep(50);
    writeHex(m_HexCmd.hexReadD277);
}

void MainWindow::writeHexListRunClick()
{
    //M68是否处于初始界面-M47是否处于维修维护状态-X23运行-D50加载页面-Yout输出
    writeHex(m_HexCmd.hexReadM68);
    sleep(50);
    writeHex(m_HexCmd.hexReadM47);
    sleep(50);
    writeHex(m_HexCmd.hexOnX23);
    sleep(50);
    writeHex(m_HexCmd.hexWriteD50);
    sleep(50);
    writeHex(m_HexCmd.hexReadYout);
    sleep(50);
    writeHex(m_HexCmd.hexReadM37);
}

void MainWindow::writeHexListStandbyClick()
{
    writeHex(m_HexCmd.hexOnX7);
    sleep(50);
    writeHex(m_HexCmd.hexReadYout);
    sleep(50);
    writeHex(m_HexCmd.hexReadM37);
}

void MainWindow::loadPixmap()
{
    pixmapValve.load(":/images/pic/Valve.png");
    pixmapRoValve.load(":/images/pic/ROValve.png");
    pixmapPretreatment.load(":/images/pic/Pretreatment.png");
    pixmapTank.load(":/images/pic/Tank.png");
    pixmapPump.load(":/images/pic/Pump.png");
    pixmapPre.load(":/images/pic/Pre.png");
    pixmapPack.load(":/images/pic/Pack.png");
    pixmapSensor.load(":/images/pic/Sense.png");
    pixmapRO.load(":/images/pic/RoPack.png");
    pixmapFlow.load(":/images/pic/Flow.png");
    pixmapEDI.load(":/images/pic/EdiPack.png");
    pixmapUvOn.load(":/images/pic/UVON2.png");
    pixmapUvOff.load(":/images/pic/UVOFF2.png");
    pixmapTankUvOn.load(":/images/pic/UVON");
    pixmapTankUvOff.load(":/images/pic/UVOFF");

}

void MainWindow::on_runPushButton_clicked()
{
    isRunButtonClicked = true;
}

void MainWindow::on_standbyPushButton_clicked()
{
    isStandbyButtonClicked = true;
}

void MainWindow::on_configPushButton_clicked()
{
    m_configDlg->show();
}

