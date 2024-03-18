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

#ifndef LEDWIDGET_H
#define LEDWIDGET_H

#include <QWidget>
#include <QColor>

class LedWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LedWidget(QWidget *parent = nullptr);

    void setColor(QColor color);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QColor m_color;
    QString m_text;
};

#endif // LEDWIDGET_H

