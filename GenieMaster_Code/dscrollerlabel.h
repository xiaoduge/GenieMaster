#ifndef DSCROLLERLABEL_H
#define DSCROLLERLABEL_H

#include <QLabel>

class DScrollerLabel : public QLabel
{
    Q_OBJECT
public:
    DScrollerLabel(QWidget* parent = Q_NULLPTR);

public:
    void setShowText(QString&);

protected:
    void paintEvent(QPaintEvent *event);

protected slots:
    void updateIndex();

private:
    int m_charWidth;
    int m_curIndex;
    int m_backIndex;
    QString m_showText;
};

#endif // DSCROLLERLABEL_H
