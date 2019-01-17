#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include "dalarmthread.h"
#include "dhistroythread.h"
#include "dstringlist.h"
#include "dscrollerlabel.h"

namespace Ui {
class MainWindow;
}

class DAboutDlg;
class ConfigDialog;
class HexTestDialog;
class AlarmMsgDlg;
class HistoricalDataDlg;
class QSerialPort;
class QTableView;
class DTableModel;
class QLabel;
class DHelpDialog;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public:
    enum FunctionCode
    {
        Code1 = 1,
        Code2 = 2,
        Code3 = 3,
        Code4 = 4,
        Code5 = 5,
        Code6 = 6,
        Code8 = 8,
        Code15 = 15,
        Code16 = 16
    };

    enum RelayStateCode
    {
        RelayOff = 0,
        RelayOn = 1,
        RelayStationError = 2,
        RelayFunctionCodeError = 3,
        RelayDataCountError = 4,
        AddressError = 5
    };

    struct DHexCmd    //定义所有命令
    {
        QString hexReadM68;    //为0说明在初始页面，不能操作机器
        QString hexReadM47;    //为0说明设备处于维护或维修状态
        QString hexReadM37;    //判断程序是否在待机状态
        QString hexOnX23;      //置位X23，PLC跳转到运行状态
        QString hexWriteD50;   //写入71，界面跳转到运行界面
        QString hexOnX7;       //X7置1，返回待机界面
        QString hexReadYout;   //读取Y口输出
        QString hexReadD340;   //原水箱液位D340
        QString hexReadD160;   //进水压力D160位）
        QString hexReadD96;    //温度D96
        QString hexReadD73;    //进水电导D73
        QString hexReadD75;    //RO电导D75
        QString hexReadD71;    //EDI电阻率D71
        QString hexReadD168;   //RO弃水压力D168
        QString hexReadD122;   //RO产水流量D122
        QString hexReadD116;   //RO弃水流量D116
        QString hexReadD166;   //纯水箱液位D166
        QString hexReadD94;    //截留率D94
        //报警信息：
        QString hexReadC120;   //更换P Pack  C120：      01 01 A3 B8 00 01 5F AB
        QString hexReadC121;   //更换管道紫外灯 C121：   01 01 A3 B9 00 01 0E 6B
        QString hexReadC122;   //更换水箱空气过滤器C122：01 01 A3 BA 00 01 FE 6B
        QString hexReadC123;   //更换纯水紫外灯C123：    01 01 A3 BB 00 01 AF AB
        QString hexReadC125;   //更换水箱紫外灯C125：    01 01 A3 BD 00 01 4F AA
        QString hexReadC127;   //更换AT Pack C127：     01 01 A3 BF 00 01 EE 6A
        QString hexReadC128;   //更换管道过滤器C128：   01 01 A3 C0 00 01 DF B2

        QString hexReadM240;   //EDI产水低于下限M240：   01 01 00 F0 00 01 FD F9
        QString hexReadM122;   //RO产水流速低于设定值M122： 01 01 00 7A 00 01 DC 13
        QString hexReadX13;    //水箱溢流或漏水保护X13：    01 02 00 0C 00 01 79 C9
        QString hexReadM242;   //截留率低于下限M242：       01 01 00 F2 00 01 5C 39
        QString hexReadM119;   //RO压力大于设定值M119：    01 01 00 77 00 01 4D D0
        QString hexReadM43;    //进水压力低于下限M43：     01 01 00 2B 00 01 8D C2
        QString hexReadM166;   //RO弃水流量低于下限M166：  01 01 00 A6 00 01 1D E9
        QString hexReadM281;   //RO产水电导率大于上限M281：01 01 01 19 00 01 2D F1
        QString hexReadM280;   //进水电导率大于上限M280：  01 01 01 18 00 01 7C 31
        QString hexReadM155;   //RO工作压力低于下限M155    01 01 00 9B 00 01 8C 25
        //耗材寿命
        QString hexReadD285;   //P Pack D285,16位    01 04 01 1D 00 01 A0 30
        QString hexReadD301;   //AT Pack D301,16位   01 04 01 2D 00 01 A0 3F
        QString hexReadD287;   //水箱空气过滤器 D287 ,16位   01 04 01 1F 00 01 01 F0
        QString hexReadD288;   //纯水紫外灯 D288,16位       01 04 01 20 00 01 31 FC
        QString hexReadD290;   // 水箱紫外灯 D290 ,16位     01 04 01 22 00 01 90 3C
        QString hexReadD286;   //管道紫外灯：D286        01 04 01 1E 00 01 50 30
        QString hexReadD277;    //管道过滤器：D277        01 04 01 15 00 01 21 F2
    };

    struct CurRelayState    //所检测继电器的工作状态
    {
        RelayStateCode stateM68;    //为0说明在初始页面，不能操作机器
        RelayStateCode stateM47;    //为0说明设备处于维护或维修状态
        RelayStateCode stateM37;    //判断程序是否在待机状态
        RelayStateCode stateX23;    //
        RelayStateCode stateE1;
        RelayStateCode stateE2;
        RelayStateCode stateE3;
        RelayStateCode stateE8;
        RelayStateCode stateE9;
        RelayStateCode stateE10;

        RelayStateCode stateC1;
        RelayStateCode stateC2;
        RelayStateCode stateC3;
        RelayStateCode stateC4;

        RelayStateCode stateEDI;

        RelayStateCode stateN1;
        RelayStateCode stateN3;


    };
    struct CurRegisterData   //所检测参数值
    {
        int nD50;           //D50的值
        qreal doubleD340;   //原水箱液位D340
        qreal doubleD160;   //进水压力D160
        qreal doubleD96;    //温度D96
        qreal doubleD73;    //进水电导D73
        qreal doubleD75;    //RO电导D75
        qreal doubleD71;    //EDI电阻率D71
        qreal doubleD168;   //RO弃水压力D168
        qreal doubleD122;   //RO产水流量D122
        qreal doubleD116;   //RO弃水流量D116
        qreal doubleD166;   //纯水箱液位D166
        qreal doubleD94;    //截留率D94
    };
    struct ConsumablesStatus  //耗材信息
    {
        quint16 uintD285;   //P Pack D285,16位    01 04 01 1D 00 01 A0 30
        quint16 uintD301;   //AT Pack D301,16位   01 04 01 2D 00 01 A0 3F
        quint16 uintD287;   //水箱空气过滤器 D287 ,16位   01 04 01 1F 00 01 01 F0
        quint16 uintD288;   //纯水紫外灯 D288,16位       01 04 01 20 00 01 31 FC
        quint16 uintD290;   // 水箱紫外灯 D290 ,16位     01 04 01 22 00 01 90 3C
        quint16 uintD286;   //管道紫外灯：D286        01 04 01 1E 00 01 50 30
        quint16 uintD277;    //管道过滤器：D277        01 04 01 15 00 01 21 F2
    };
    struct ErrorMessage
    {
        RelayStateCode stateC120;   //更换P Pack  C120：      01 01 A3 B8 00 01 5F AB
        RelayStateCode stateC121;   //更换管道紫外灯 C121：   01 01 A3 B9 00 01 0E 6B
        RelayStateCode stateC122;   //更换水箱空气过滤器C122：01 01 A3 BA 00 01 FE 6B
        RelayStateCode stateC123;   //更换纯水紫外灯C123：    01 01 A3 BB 00 01 AF AB
        RelayStateCode stateC125;   //更换水箱紫外灯C125：    01 01 A3 BD 00 01 4F AA
        RelayStateCode stateC127;   //更换AT Pack C127：     01 01 A3 BF 00 01 EE 6A
        RelayStateCode stateC128;   //更换管道过滤器C128：   01 01 A3 C0 00 01 DF B2

        RelayStateCode stateM240;   //EDI产水低于下限M240：   01 01 00 F0 00 01 FD F9
        RelayStateCode stateM122;   //RO产水流速低于设定值M122： 01 01 00 7A 00 01 DC 13
        RelayStateCode stateX13;    //水箱溢流或漏水保护X13：    01 02 00 0C 00 01 79 C9
        RelayStateCode stateM242;   //截留率低于下限M242：       01 01 00 F2 00 01 5C 39
        RelayStateCode stateM119;   //RO压力大于设定值M119：    01 01 00 77 00 01 4D D0
        RelayStateCode stateM43;    //进水压力低于下限M43：     01 01 00 2B 00 01 8D C2
        RelayStateCode stateM166;   //RO弃水流量低于下限M166：  01 01 00 A6 00 01 1D E9
        RelayStateCode stateM281;   //RO产水电导率大于上限M281：01 01 01 19 00 01 2D F1
        RelayStateCode stateM280;   //进水电导率大于上限M280：  01 01 01 18 00 01 7C 31
        RelayStateCode stateM155;   //RO工作压力低于下限M155    01 01 00 9B 00 01 8C 25
    };

protected:
    void timerEvent(QTimerEvent *event);
    void paintEvent(QPaintEvent *event);

    void writeHex(QString& hex);
    void readHex();

    void getScreenInfo();
    bool isPathExist(const QString&);

private slots:
    void setPortName(QString);
    void setBaudRate(QString);
    void setDataBits(int);
    void setParityBits(int);
    void setStopBits(int);
    void setFlowControl(int);

    void onConfigActiontriggered();//triggered
    void onCmdTesttriggered();
    void onCloseConnectActiontriggered();
    void onTestDlg(QString cmd);
    void onConsumablesActiontriggered();
    void onParameterActiontriggered();

    void onAlarmMessageActiontriggered();
    void onChildThreadError(const QString&);
    void onReceiveAlarmMessage(QStack<QString>&);

    void onHistoricalDataActiontriggered();
    void onReceiveHistoricalData(QStack<QString>&);
    void onReceiveHistorucalDataIndex(int&);

    void onFeedTankConfigtoggle(bool);
    void onVersionActiontriggered();
    void onHelpActiontriggered();

    void on_runPushButton_clicked();
    void on_standbyPushButton_clicked();
    void on_configPushButton_clicked();

private:
    void initMenuBar();
    void initStatusBar();
    void initLogFile();
    void initRelayState();
    void initGroupBox();
    void initButton();

    void initScrollerLabel();
    void updateAlarmMessage();

    void loadHexCmd();
    void writeHexList();
    void writeHexListRunClick();
    void writeHexListStandbyClick();
    void loadPixmap();

    void connectConfigSignals();
    void StringToHex(QString str, QByteArray &senddata);
    char ConvertHexChar(char ch);
    void sleep(unsigned int msec);

    void analyzeHex(QString &hex);    //将QString响应数据按2位一组，装入QStringList
    RelayStateCode analyzeSingleRelay(QString &);        //用于解析功能码01读取单个线圈的状态
    quint32 analyzeMultiRelay(QString &);         //用于解析功能码01读取多个线圈的状态
    qreal analyzeRegister16(QString &);         //用于解析16位的寄存器
    qreal analyzeRegister32(QString &);         //用于解析32位寄存器（2个16位寄存器）
    void analyzeYout(QString &);

    int analyzeWriteRegister06Back(QString &, QString &);  //解析功能码06写寄存器返回值
    RelayStateCode analyzeWriteRelay05Back(QString &, QString &);    //解析功能码05写单个线圈返回值

    void drawDestRect(QPainter &painter, int runX, int runY, int runWidth, int runHeight
                      , double xTimes, double yTimes);
    void drawDevicePixmap(QPainter &painter, int posX, int posY, double xTimes, double yTimes);
    void drawPipeLine(QPainter &painter, int posX, int posY, double xTimes, double yTimes);
    void drawDeviceText(QPainter &painter, int posX, int posY, double xTimes, double yTimes);
    void drawSignsLight(QPainter &painter, int posX, int posY, double xTimes, double yTimes); //运行指示灯
    void drawParameter(QPainter &painter, int posX, int posY, double xTimes, double yTimes);


signals:
    void sendAnswerToChild(QString);

    //发送报警信息
    void sendAlarmMsg(const QString&);
    void requestAlarmMsg();
    void sendAlarmMsgToDlg(QStack<QString>&);

    //发送历史数据
    void sendHistoricalData(const QString&, int&);
    void requestHistoricalData(int&);
    void sendHistoricalDataToDlg(QStack<QString>&);

private:
    Ui::MainWindow *ui;

    ConfigDialog *m_configDlg;
    HexTestDialog *m_hexTestDlg;
    AlarmMsgDlg *m_AlarmMsgDlg;
    HistoricalDataDlg *m_HistoricalDataDlg;
    DAboutDlg *m_aboutDlg;
    DHelpDialog *m_helpDlg;

    QSerialPort *m_serialPort;
    QFile logFile;
    DAlarmThread m_alarmThread; //写报警信息线程
    DHistroyThread m_histroyThread; //写历史数据线程

    QTableView *m_consumablesView;
    DTableModel *m_consumablesTableModel;
    QTableView *m_parameterView;
    DTableModel *m_parameterTableModel;

//    QLabel *m_scrollerLabel;
    DScrollerLabel *m_scrollerLabel;
    int m_nPos;
    DStringList m_strLabelList;

    QString m_receiveData;
    int id1, idTest, idAutoCheck, idRefresh, idStartCheck;
    bool isRunButtonClicked;
    bool isStandbyButtonClicked;
    bool isConnect;
    bool isAbnormal;

    bool isFeedTank;

    FunctionCode m_functionCode; //响应功能码
    int answerDataCount; //响应数据字节数

    double m_screenWidth;    //默认1366
    double m_screenHeight;   //默认768

private:
    DHexCmd m_HexCmd;
    struct CurRelayState m_curRelayState;
    CurRegisterData m_curRegisterData;
    ConsumablesStatus m_consumablesStatus;
    ErrorMessage m_errorMessage;

    QString m_strBackM68;    //判断机器是否处于初始界面
    QString m_strBackM47;    //判断机器是否处于维护维修状态
    QString m_strBackX23;    //检查X23是否成功启动
    QString m_strBackD50;    //检查D50是否成功写入
    QString m_strBackM37;    //M37是否处于运行状态
    QString m_strBackYout;   //Yout输出
    QString m_strBackD340;   //原水箱液位D340
    QString m_strBackD160;   //进水压力D160
    QString m_strBackD96;    // 温度D96
    QString m_strBackD73;    //进水电导D73
    QString m_strBackD75;    //RO电导D75
    QString m_strBackD71;    //EDI电阻率D71
    QString m_strBackD168;   //RO弃水压力D168
    QString m_strBackD122;   //RO产水流量D122
    QString m_strBackD116;   //RO弃水流量D116
    QString m_strBackD166;   //纯水箱液位D166
    QString m_strBackD94;    //截留率D94

    QString m_strBackC120;   //更换P Pack  C120：
    QString m_strBackC121;   //更换管道紫外灯 C121：
    QString m_strBackC122;   //更换水箱空气过滤器C122
    QString m_strBackC123;   //更换纯水紫外灯C123：
    QString m_strBackC125;   //更换水箱紫外灯C125：
    QString m_strBackC127;   //更换AT Pack C127：
    QString m_strBackC128;   //更换管道过滤器C128：
    QString m_strBackM240;   //EDI产水低于下限M240：
    QString m_strBackM122;   //RO产水流速低于设定值M122：
    QString m_strBackX13;    //水箱溢流或漏水保护X13：
    QString m_strBackM242;   //截留率低于下限M242：
    QString m_strBackM119;   //RO压力大于设定值M119：
    QString m_strBackM43;    //进水压力低于下限M43：
    QString m_strBackM166;   //RO弃水流量低于下限M166：
    QString m_strBackM281;   //RO产水电导率大于上限M281：
    QString m_strBackM280;   //进水电导率大于上限M280：
    QString m_strBackM155;   //RO工作压力低于下限M155

    QString m_strBackD285;   //P Pack D285,16位
    QString m_strBackD301;   //AT Pack D301,16位
    QString m_strBackD287;   //水箱空气过滤器 D287 ,16位
    QString m_strBackD288;   //纯水紫外灯 D288,16位
    QString m_strBackD290;   // 水箱紫外灯 D290 ,16位
    QString m_strBackD286;   //管道紫外灯：D286
    QString m_strBackD277;    //管道过滤器：D277

//素材图标
private:
    QPixmap pixmapValve;
    QPixmap pixmapRoValve;
    QPixmap pixmapPretreatment;
    QPixmap pixmapTank;
    QPixmap pixmapPump;
    QPixmap pixmapPre;
    QPixmap pixmapPack;
    QPixmap pixmapSensor;
    QPixmap pixmapRO;
    QPixmap pixmapFlow;
    QPixmap pixmapEDI;
    QPixmap pixmapUvOn;
    QPixmap pixmapUvOff;
    QPixmap pixmapTankUvOn;
    QPixmap pixmapTankUvOff;

private:
    bool isRunE1;
    bool isRunE2;
    bool isRunE3;
    bool isRunE8;
    bool isRunE9;
    bool isRunE10;
    bool isRunC1;
    bool isRunC2;
    bool isRunC3;
    bool isRunC4;
    bool isRunEDI;
};

#endif // MAINWINDOW_H
