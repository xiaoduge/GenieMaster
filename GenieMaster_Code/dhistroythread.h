#ifndef DHISTROYTHREAD_H
#define DHISTROYTHREAD_H

#include <QThread>
#include <QFile>

class DHistroyThread : public QThread
{
    Q_OBJECT
public:
    DHistroyThread(QObject* parent = Q_NULLPTR);
    ~DHistroyThread();

public:
    void run();
    void stop();

    void initFile();
    void initFeedFile();
    void initRoFile();
    void initEDIFile();
    void initOperationPresFile();
    void initRORateFile();
    void initRODrainRateFile();
    void initTemperatureFile();

    bool isPathExist(const QString&);

signals:
    void openFileError(const QString&);
    void sendHistoricalData(QStack<QString>&);

protected slots:
    void writeHistoricalData(const QString&, int& index);
    void readHistoricalData(int& index);

private:
    void writeFeedData(const QString&);
    void writeRoData(const QString&);
    void writeEDIData(const QString&);
    void writeOperationPresData(const QString&);
    void writeRORateData(const QString&);
    void writeRODrainRateData(const QString&);
    void writeTemperatureData(const QString&);

    bool compareFeedData(const QString&, int&, int&);
    bool compareRoData(const QString&, int&, int&);
    bool compareEDIData(const QString&, int&, int&);
    bool compareOperationPresData(const QString&, int&, int&);
    bool compareRORateData(const QString&, int&, int&);
    bool compareRODrainRateData(const QString&, int&, int&);
    bool compareTemperatureData(const QString&, int&, int&);

private:
    QFile m_feedFile;
    QFile m_RoFile;
    QFile m_EDIFile;
    QFile m_PresFile;
    QFile m_RoRateFile;
    QFile m_DrainRateFile;
    QFile m_TemperFile;
    bool isFeedUpdate;
    bool isRoUpdate;
    bool isEDIUpdate;
    bool isPresUpdate;
    bool isRoRateUpdate;
    bool isDrainRateUpdate;
    bool isTempUpdate;
};

#endif // DHISTROYTHREAD_H
