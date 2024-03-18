/******************************************************************************
* Qt COM listener for XLAT, captures data, charts, interpolates metrics for deeper insights.
* Copyright (c) 2024 axaro1
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include "ledwidget.h"
#include <QPainter>
#include <QLinearGradient>
#include <QFontMetrics>

LedWidget::LedWidget(QWidget *parent) : QWidget(parent), m_color(Qt::red), m_text("VCOM")
{
    setFixedSize(81, 51); // size of the LED widget
}

void LedWidget::setColor(QColor color)
{
    m_color = color;
    update(); // Trigger a repaint of the widget
}

void LedWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QLinearGradient gradient(0, 0, width(), height());
    gradient.setColorAt(0, QColor(70, 70, 70));
    gradient.setColorAt(1, QColor(20, 20, 20));

    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawRect(0, 0, width(), height());

    painter.setPen(Qt::NoPen);
    painter.setBrush(m_color);
    painter.drawRect(2, 2, width() - 4, height() - 2);

    QFont font("Droid Sans Mono", 8);
    painter.setFont(font);
    painter.setPen(Qt::black);

    QFontMetrics fm(font);
    int textWidth = fm.horizontalAdvance(m_text);
    int textHeight = fm.height();
    int x = (width() - textWidth) / 2;
    int y = (height() + textHeight) / 2;

    painter.drawText(x, y, m_text);

    // Add glossy effect
    QLinearGradient glossGradient(0, 0, width(), height());
    glossGradient.setColorAt(0, QColor(255, 255, 255, 150));
    glossGradient.setColorAt(1, QColor(255, 255, 255, 0));

    painter.setBrush(glossGradient);
    painter.drawRect(0, 0, width(), height());
}
