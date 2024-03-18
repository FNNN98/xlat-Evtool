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

#ifndef XLAT_EVTOOL_H
#define XLAT_EVTOOL_H

#include "ledwidget.h"
#include <QMainWindow>
#include <QSerialPort>
#include <QTableView>
#include <QStandardItemModel>
#include <QLineEdit>
#include <QHeaderView>
#include <QTimer>
#include <QDialog>
#include <vector>
#include <QBarSet>

QT_BEGIN_NAMESPACE
namespace Ui { class xlat_evtool;}
QT_END_NAMESPACE

class xlat_evtool : public QMainWindow
{
    Q_OBJECT

public:
    xlat_evtool(QWidget *parent = nullptr);
    ~xlat_evtool();
    struct xlatData {
        int reportNumber;
        int latency;
        int avgLatency;
        int stdev;
    };

signals:
    void serialDataReceived(const QByteArray &data);
    void processData(int numInputs, int latency, int average, int stdev);

private slots:
    void readSerialData();
    void checkAndOpenSerialPort();
    void checkConnectionStatus();
    void handleError(QSerialPort::SerialPortError error);
    //void printTotalArray();
    void updateTableView();
    void updateTableViewDynamic(const xlatData& newData);
    void initializeUI();
    void saveCSV();
    void handleCsvImport();
    void importCsv(const QString& filePath);
    void dataInterpolation();
    void updatePercentileData(int p90Value, int p95Value, int p5Value, int p10Value, int iqrValue,
                              int maxLatency, int minLatency, double avgLatency, int medianLatency,
                              int madValue);
    void clearData();
    void openGitHubLink();
    void disclaimer();

    void showScatterChartWindow();
    void showHistogramWindow();

private:
    Ui::xlat_evtool *ui;
    QTimer *connectionCheckTimer = new QTimer(this);

    QLineEdit *p90LineEdit;
    QLineEdit *p95LineEdit;
    QLineEdit *p5LineEdit;
    QLineEdit *p10LineEdit;
    QLineEdit *iqrLineEdit;
    QLineEdit *maxLatLineEdit;
    QLineEdit *minLatLineEdit;
    QLineEdit *avgMadLineEdit;
    QLineEdit *medLatLineEdit;
    QLineEdit *avgLatLineEdit;
    QLineEdit *stdevLineEdit;

    QLineEdit *portNameLineEdit;
    QLineEdit *descriptionLineEdit;
    QLineEdit *serialNumberLineEdit;
    QLineEdit *manufacturerLineEdit;
    QLineEdit *vidLineEdit;
    QLineEdit *pidLineEdit;

    QSerialPort *serialPort;

    LedWidget *comStatus;

    QStandardItemModel *model = new QStandardItemModel(this);
    QTableView *tableView;

    xlatData myData;
    std::vector<xlatData> allData;

    bool resize = true;

    int minLatency;
    int maxLatency;
    int p90Value;
    int p95Value;
    int p5Value;
    int p10Value;
    int iqrValue;
    int madValue;
    int avgLatency;
    int medianLatency;
    int stdev;
    int _counterCall = 0;

};

#endif // XLAT_EVTOOL_H
