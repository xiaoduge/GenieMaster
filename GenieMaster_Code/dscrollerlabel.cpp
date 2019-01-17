#include "dscrollerlabel.h"
#include <QTimer>
#include <QPainter>
#include <QDebug>

DScrollerLabel::DScrollerLabel(QWidget* parent):QLabel(parent)
{
    m_curIndex = 0;
    m_backIndex = 0;
    m_showText = " ";

    m_charWidth = fontMetrics().width("a")*1.5;
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateIndex()));
    timer->start(100);
    m_curIndex = 0;
}

void DScrollerLabel::setShowText(QString& showText)
{
    m_showText.clear();
    m_showText = showText;
}

void DScrollerLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawText(0, 20, m_showText.mid(m_curIndex));
    int nMax = (m_showText.length() - m_curIndex)*m_charWidth;
    if(nMax < width())
    {
        painter.drawText(width() - m_charWidth*m_curIndex, 20, m_showText.left(m_curIndex));
    }
//    painter.drawText(width() - m_charWidth*m_curIndex, 20, m_showText.left(m_curIndex));

    QLabel::paintEvent(event);
}

void DScrollerLabel::updateIndex()
{
    update();
    m_curIndex++;

    if (m_curIndex*m_charWidth > width())
    {
        m_curIndex = 0;
    }

}
