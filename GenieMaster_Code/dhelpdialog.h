#ifndef DHELPDIALOG_H
#define DHELPDIALOG_H

#include <QDialog>
#include <qtextcursor.h>
#include <QFile>

namespace Ui {
class DHelpDialog;
}

class DHelpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DHelpDialog(QWidget *parent = 0);
    ~DHelpDialog();

protected:
    void setOperatingInstructions();
    void setUiExplain();

    bool isExists(QString&);
protected slots:
    void onTreeWidgetItemChanged();

private:
    void initDlg();
    void initLayout();
    void initTextEdit();

    void createHelpFilePath();
    void analyseHelpUIFile();
    void lookup(QString&, QString&);
    void initMap();

    void setAlignHCenter(QTextCursor&);
    void setAlignHLeft(QTextCursor&);
    void insertImage(QTextCursor&, QString&);

private:
    Ui::DHelpDialog *ui;

    QFile m_OperFile;
    QFile m_UiFile;

    QMap<QString, int> m_strMap;
    QString m_strh1;
    QString m_strb1;
    QString m_strh2;
    QString m_strb2;
    QString m_strh3;
    QString m_strb3;
    QString m_strh4;
    QString m_strb4;
    QString m_strh5;
    QString m_strb5;
    QString m_strh6;
    QString m_strb6;
};

#endif // DHELPDIALOG_H
