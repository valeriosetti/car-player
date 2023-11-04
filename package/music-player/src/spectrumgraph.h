#ifndef SPECTRUMGRAPH_H
#define SPECTRUMGRAPH_H

#include <QObject>
#include <QQuickPaintedItem>
#include <QPainter>
#include <QImage>
#include <QTimer>

class SpectrumGraph : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(int bars_count MEMBER col_num)

public:
    SpectrumGraph(QQuickItem *parent = 0);

    void paint(QPainter *painter);

public slots:
    void timer_expired(void);
    void setData(QVector<float>* new_mags);

private:
    QVector<float> mags;
    QTimer m_timer;
    int col_num = 20;
    int col_spacing = 2;
};

#endif // SPECTRUMGRAPH_H
