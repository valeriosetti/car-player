#include "spectrumgraph.h"
#include <QBrush>

SpectrumGraph::SpectrumGraph(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &SpectrumGraph::timer_expired);
    mags = QVector<float>(col_num);
}

void SpectrumGraph::timer_expired(void)
{
    update();
}

void SpectrumGraph::paint(QPainter *painter)
{
    QBrush brush(QColor(0, 0, 200));

    painter->setBrush(brush);
    painter->setPen(Qt::NoPen);
    painter->setRenderHint(QPainter::Antialiasing);

    QSizeF itemSize = size();
    int max_x = itemSize.width();
    int max_y = itemSize.height();
    int col_width = max_x / col_num;
    int col_height;

    for (int i=0; i<col_num; i++) {
        col_height = (int)(max_y * mags.at(i));
        painter->drawRoundedRect(i*col_width + col_spacing,
                                 max_y - col_height,
                                 col_width - 2*col_spacing, col_height, 10, 10);
    }
}

void SpectrumGraph::setData(QVector<float>* new_mags)
{
    mags.clear();
    for (float& mag_item : *new_mags) {
        mags.append(1.0 + mag_item/80.0);
    }

    update();
}
