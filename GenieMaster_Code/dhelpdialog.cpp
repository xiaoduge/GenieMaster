#include "dhelpdialog.h"
#include "ui_dhelpdialog.h"
#include <QGridLayout>
#include <QDir>
#include <QDebug>
#include <QMessageBox>

DHelpDialog::DHelpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DHelpDialog)
{
    ui->setupUi(this);

    initMap();
    createHelpFilePath();
    initDlg();
}

DHelpDialog::~DHelpDialog()
{
    delete ui;
}

void DHelpDialog::setOperatingInstructions()
{
    QTextCursor textCursor = ui->textEdit->textCursor();
    ui->textEdit->clear();

    QTextCharFormat charFormat;
    charFormat.setFont(QFont(QString("宋体"), 16));
    textCursor.setCharFormat(charFormat);

    QTextStream in(&m_OperFile);
    in.seek(0);
    QString strReadAll;
    strReadAll = in.readAll();
    textCursor.insertText(strReadAll);
}

void DHelpDialog::setUiExplain()
{
    QTextCursor textCursor = ui->textEdit->textCursor();
    ui->textEdit->clear();

    setAlignHLeft(textCursor);
    textCursor.insertText(m_strh1);
    textCursor.insertText(QString("\n\n"));

    QString imageName = QString(":/images/pic/main.png");
    setAlignHCenter(textCursor);
    insertImage(textCursor, imageName);
    setAlignHLeft(textCursor);
    textCursor.insertText(m_strb1);
    textCursor.insertText(QString("\n\n"));

    textCursor.insertText(m_strh2);
    textCursor.insertText(QString("\n\n"));

    imageName.clear();
    imageName = QString(":/images/pic/com.png");
    setAlignHCenter(textCursor);
    insertImage(textCursor, imageName);

    setAlignHLeft(textCursor);
    textCursor.insertText(m_strb2);
    textCursor.insertText(QString("\n\n"));

    textCursor.insertText(m_strh3);
    textCursor.insertText(QString("\n\n"));

    imageName.clear();
    imageName = QString(":/images/pic/par.png");
    setAlignHCenter(textCursor);
    insertImage(textCursor, imageName);

    setAlignHLeft(textCursor);
    textCursor.insertText(m_strb3);
    textCursor.insertText(QString("\n\n"));

    textCursor.insertText(m_strh4);
    textCursor.insertText(QString("\n\n"));

    imageName.clear();
    imageName = QString(":/images/pic/cons.png");
    setAlignHCenter(textCursor);
    insertImage(textCursor, imageName);

    setAlignHLeft(textCursor);
    textCursor.insertText(m_strb4);
    textCursor.insertText(QString("\n\n"));

    textCursor.insertText(m_strh5);
    textCursor.insertText(QString("\n\n"));

    imageName.clear();
    imageName = QString(":/images/pic/alar.png");
    setAlignHCenter(textCursor);
    insertImage(textCursor, imageName);

    setAlignHLeft(textCursor);
    textCursor.insertText(m_strb5);
    textCursor.insertText(QString("\n\n"));

    textCursor.insertText(m_strh6);
    textCursor.insertText(QString("\n\n"));

    imageName.clear();
    imageName = QString(":/images/pic/his.png");
    setAlignHCenter(textCursor);
    insertImage(textCursor, imageName);
    setAlignHLeft(textCursor);
    textCursor.insertText(m_strb6);
    textCursor.insertText(QString("\n\n"));
}

void DHelpDialog::setAlignHCenter(QTextCursor& textCursor)
{
    QTextBlockFormat blockFormat;
    blockFormat.setAlignment(Qt::AlignCenter);
    textCursor.setBlockFormat(blockFormat);
}

void DHelpDialog::setAlignHLeft(QTextCursor& textCursor)
{
    QTextBlockFormat blockFormat;
    blockFormat.setAlignment(Qt::AlignLeft);
    textCursor.setBlockFormat(blockFormat);

    QTextCharFormat charFormat;
    charFormat.setFont(QFont(QString("宋体"), 16));
    textCursor.setCharFormat(charFormat);
}

void DHelpDialog::insertImage(QTextCursor& textCursor, QString& strImageName)
{
    QTextImageFormat imageFormat;
    imageFormat.setName(strImageName);
    textCursor.insertImage(imageFormat);
    textCursor.insertText(QString("\n\n"));
}

void DHelpDialog::onTreeWidgetItemChanged()
{
    int nRow = ui->treeWidget->currentIndex().row();
    switch(nRow)
    {
    case 0:
        setOperatingInstructions();
        break;
    case 1:
        setUiExplain();
        break;
    default:
        break;
    }
}

void DHelpDialog::initDlg()
{
    QPalette palette = this->palette();
    palette.setBrush(QPalette::Background, QColor(240, 255, 240));
    this->setPalette(palette);
    Qt::WindowFlags flags = this->windowFlags() & ~Qt::WindowContextHelpButtonHint
                                                |  Qt::WindowMinMaxButtonsHint;
    this->setWindowFlags(flags);
    this->setWindowState(Qt::WindowMaximized);

    this->setWindowTitle(tr("帮助"));
    this->setWindowIcon(QIcon(":/images/pic/helpblue.png"));

    initLayout();
    initTextEdit();

    connect(ui->treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
            this, SLOT(onTreeWidgetItemChanged()));

    setOperatingInstructions();
}

void DHelpDialog::initLayout()
{
    QString qmlTextEdit = "QTextEdit{background:#f0fff0;\
                                    }";
    QString qmlTreeWidget = "QTreeWidget{background:#f0fff0;\
                                        }";
    ui->textEdit->setStyleSheet(qmlTextEdit);
    ui->treeWidget->setStyleSheet(qmlTreeWidget);

    QGridLayout *gridLayout = new QGridLayout;
    ui->treeWidget->setMaximumWidth(this->width()/6);
    gridLayout->addWidget(ui->treeWidget, 0, 0, 1, 1);
    gridLayout->addWidget(ui->textEdit, 0, 1, 1, 1);
    this->setLayout(gridLayout);
}

void DHelpDialog::initTextEdit()
{
    QString qmlTextEdit = "QTextEdit{background-color:#f0fff0;\
                                    }";
    ui->textEdit->setStyleSheet(qmlTextEdit);
    ui->textEdit->setReadOnly(true);
}

void DHelpDialog::createHelpFilePath()
{
    QString strPath = QDir::currentPath() + QString("/") + QString("Help");
    if(!isExists(strPath))
    {
        QMessageBox::warning(this, tr("Error"), tr("Create Help File Path Error!"), QMessageBox::Ok);
        return;
    }
    QString strOperFileName = strPath + QString("/") + QString("helpOperate.ini");
    QString strUiFileName = strPath + QString("/") + QString("helpUI.ini");
    m_OperFile.setFileName(strOperFileName);
    m_UiFile.setFileName(strUiFileName);

    if(!m_OperFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Error"), tr("Open help file false:helpOperate.ini"), QMessageBox::Ok);
        return;
    }
    if(!m_UiFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Error"), tr("Open help file false:helpUI.ini"), QMessageBox::Ok);
        return;
    }
    else
    {
        analyseHelpUIFile();
    }
}

void  DHelpDialog::analyseHelpUIFile()
{
    QTextStream in(&m_UiFile);
    QString strIndex;
    QString strLine;
    while(!in.atEnd())
    {
       strLine = in.readLine();
       strIndex = strLine.mid(0, 3);
       lookup(strLine, strIndex);
       strLine.clear();
       strIndex.clear();
    }
}

void DHelpDialog::lookup(QString& strLine, QString& strIndex)
{
    switch (m_strMap.value(strIndex))
    {
    case 1:
        m_strh1 = strLine.remove(strIndex);
        break;
    case 2:
        m_strh2 = strLine.remove(strIndex);
        break;
    case 3:
        m_strh3 = strLine.remove(strIndex);
        break;
    case 4:
        m_strh4 = strLine.remove(strIndex);
        break;
    case 5:
        m_strh5 = strLine.remove(strIndex);
        break;
    case 6:
        m_strh6 = strLine.remove(strIndex);
        break;
    case 7:
        m_strb1 += strLine.remove(strIndex);
        break;
    case 8:
        m_strb2 += strLine.remove(strIndex);
        break;
    case 9:
        m_strb3 += strLine.remove(strIndex);
        break;
    case 10:
        m_strb4 += strLine.remove(strIndex);
        break;
    case 11:
        m_strb5 += strLine.remove(strIndex);
        break;
    case 12:
        m_strb6 += strLine.remove(strIndex);
        break;
    default:
        break;
    }
}

void DHelpDialog::initMap()
{
    m_strMap["h1:"] = 1;
    m_strMap["h2:"] = 2;
    m_strMap["h3:"] = 3;
    m_strMap["h4:"] = 4;
    m_strMap["h5:"] = 5;
    m_strMap["h6:"] = 6;
    m_strMap["b1:"] = 7;
    m_strMap["b2:"] = 8;
    m_strMap["b3:"] = 9;
    m_strMap["b4:"] = 10;
    m_strMap["b5:"] = 11;
    m_strMap["b6:"] = 12;
}

bool DHelpDialog::isExists(QString& strPath)
{
    QDir dir(strPath);
    if(dir.exists())
    {
        return true;
    }
    else
    {
        return dir.mkpath(strPath);
    }
}
