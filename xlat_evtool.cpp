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

#include "xlat_evtool.h"
#include "ui_xlat_evtool.h"
#include "qserialport.h"
#include <QDebug>
#include <QtCharts>
#include <QCoreApplication>
#include <QtCharts/QChartView>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>
#include <QAbstractAxis>
#include <QString>
#include <QStandardItemModel>
#include <QtSerialPort/QSerialPortInfo>
#include <QFile>
#include <QFileDialog>
#include <QPushButton>
#include <algorithm> // for std::sort
#include <cmath>     // for std::round
#include <QLineEdit>
#include <QScrollBar>
#include <QLabel>
#include <QTimer>
#include <numeric> // for std::accumulate
#include <QMessageBox>
#include <QDesktopServices> // for github logo (https://github.com/logos)
#include <QUrl> // for github logo
#include <QDialog> // brand ownership disclaimer
#include <QVBoxLayout>
#include <QColor>

xlat_evtool::xlat_evtool(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::xlat_evtool)
{
    ui->setupUi(this);

    initializeUI(); // Setting up dark mode elements

    p90LineEdit = findChild<QLineEdit*>("p90");
    p95LineEdit = findChild<QLineEdit*>("p95");
    p5LineEdit = findChild<QLineEdit*>("p5");
    p10LineEdit = findChild<QLineEdit*>("p10");
    iqrLineEdit = findChild<QLineEdit*>("iqr");
    maxLatLineEdit = findChild<QLineEdit*>("maxLat");
    minLatLineEdit = findChild<QLineEdit*>("minLat");
    avgMadLineEdit = findChild<QLineEdit*>("avgMad");
    medLatLineEdit = findChild<QLineEdit*>("medLat");
    avgLatLineEdit = findChild<QLineEdit*>("avgLats");
    stdevLineEdit = findChild<QLineEdit*>("stdevi");
    portNameLineEdit = findChild<QLineEdit*>("portName");
    descriptionLineEdit = findChild<QLineEdit*>("description");
    serialNumberLineEdit = findChild<QLineEdit*>("serialNumber");
    manufacturerLineEdit = findChild<QLineEdit*>("manufacturer");
    vidLineEdit = findChild<QLineEdit*>("vid");
    pidLineEdit = findChild<QLineEdit*>("pid");

    LedWidget *vcomStatus = findChild<LedWidget*>("vcomStatus");
    tableView = findChild<QTableView*>("tableView");

    // Initialize the serial port
    serialPort = new QSerialPort(this);

    foreach(const QSerialPortInfo &port, QSerialPortInfo::availablePorts()) {
        if (port.isValid() && !port.isBusy()) {
            serialPort->setPortName(port.portName());
            break;
        }
    }

    serialPort->setBaudRate(QSerialPort::Baud1m);

    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setStopBits(QSerialPort::TwoStop);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setFlowControl(QSerialPort::HardwareControl);

    connect(serialPort, &QSerialPort::readyRead, this, &xlat_evtool::readSerialData);

    connect(serialPort, QOverload<QSerialPort::SerialPortError>::of(&QSerialPort::errorOccurred),
            this, &xlat_evtool::handleError);

    connect(connectionCheckTimer, &QTimer::timeout, this, &xlat_evtool::checkConnectionStatus);
    connectionCheckTimer->start(1000);
    checkConnectionStatus();

    connect(ui->csvSaving, &QPushButton::clicked, this, &xlat_evtool::saveCSV);

    connect(ui->csvImport, &QPushButton::clicked, this, &xlat_evtool::handleCsvImport);

    connect(ui->clearAll, &QPushButton::clicked, this, &xlat_evtool::clearData);

    QString executablePath = QCoreApplication::applicationDirPath();
    QString pngPath = executablePath + "/logo.png";

    QPushButton *github = findChild<QPushButton*>("github");
    QIcon icon(pngPath); // Replace ":/github_logo.png" with the actual path to your GitHub logo resource
        github->setIcon(icon);
        github->setIconSize(QSize(64, 64));
        github->setFlat(true);
        connect(github, &QPushButton::clicked, this, &xlat_evtool::openGitHubLink);

    QPushButton *ownership = findChild<QPushButton*>("xlatDisclaimer");
    ownership->setFlat(true);
    connect(ownership, &QPushButton::clicked, this, &xlat_evtool::disclaimer);

    // Scatter Plot
    connect(ui->visualizeChart, &QPushButton::clicked, this, &xlat_evtool::showScatterChartWindow);

    // Distribution Bar Plot
    connect(ui->visualizeChart_2, &QPushButton::clicked, this, &xlat_evtool::showHistogramWindow);


}

xlat_evtool::~xlat_evtool()
{
    delete ui;
    if (serialPort->isOpen()) {
        serialPort->close();
    }
}


void xlat_evtool::initializeUI() {

    QString headerStyleSheet = "QMainWindow::title { background-color: rgb(31, 32, 41); color: rgb(101, 162, 133); }";
    this->setStyleSheet(headerStyleSheet);

    this->setStyleSheet("background-color: rgb(31, 32, 41); color: rgb(101, 162, 133);");

    QString buttonStyle = "background-color: rgb(61, 62, 71); color: rgb(101, 162, 133);";
        ui->csvSaving->setStyleSheet(buttonStyle);
        ui->csvImport->setStyleSheet(buttonStyle);
        ui->clearAll->setStyleSheet(buttonStyle);
        ui->visualizeChart_2->setStyleSheet(buttonStyle);
        ui->visualizeChart->setStyleSheet(buttonStyle);

    QString groupBoxStyle = "QGroupBox { border: 2px solid rgb(101, 162, 133); }"; // Border color for group boxes
        ui->groupBox->setStyleSheet(groupBoxStyle);
        ui->groupBox_2->setStyleSheet(groupBoxStyle);
        ui->groupBox_3->setStyleSheet(groupBoxStyle);
        ui->groupBox_6->setStyleSheet(groupBoxStyle);

     QString headerStyle = "QHeaderView::section { background-color: rgb(41, 42, 51); color: rgb(101, 162, 133); }";
     ui->tableView->horizontalHeader()->setStyleSheet(headerStyle);
     ui->tableView->verticalHeader()->setStyleSheet(headerStyle);

     QString scrollBarStyle =
         "QScrollBar:vertical {"
         "    border: none;"
         "    background-color: #000000;"
         "    width: 20px;"
         "    margin: 22px 0 22px 0;"
         "}"
         "QScrollBar::handle:vertical {"
         "    background-color: #32CC99;"
         "    min-height: 25px;"
         "}"
         "QScrollBar::add-line:vertical {"
         "    background-color: #32CC99;"
         "    height: 20px;"
         "    subcontrol-position: bottom;"
         "    subcontrol-origin: margin;"
         "}"
         "QScrollBar::sub-line:vertical {"
         "    background-color: #32CC99;"
         "    height: 20px;"
         "    subcontrol-position: top;"
         "    subcontrol-origin: margin;"
         "}"
         "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
         "    background: none;"
         "}";

    ui->tableView->verticalScrollBar()->setStyleSheet(scrollBarStyle);

    // Tooltips for IQR and MAD
    ui->label_8->setToolTip("Median Absolute Deviation (MAD) is a robust measure of variability that is less influenced by outliers compared to Standard Deviation. MAD is computed as the median of the absolute deviations from the dataset's median, contrasting with STDEV, which calculates the average distance of data points from the mean and is typically applied to symmetric distributions.");
    ui->label_8->setMouseTracking(true);

    ui->label_10->setToolTip("The Interquartile Range (IQR) is a measure of statistical dispersion or spread in a dataset. "
                             "It is calculated as the difference between the third quartile (Q3) and the first quartile (Q1) in the dataset. "
                             "In other words, it represents the range that contains the middle 50% of the data. "
                             "A larger IQR indicates greater variability within the dataset, while a smaller IQR suggests less variability. "
                             "IQR is particularly useful in identifying outliers and understanding the central tendency of the data.");
    ui->label_10->setMouseTracking(true);

}


void xlat_evtool::checkAndOpenSerialPort() {
    if (!serialPort->isOpen()) {
        // Iterate through available serial ports
        foreach(const QSerialPortInfo &port, QSerialPortInfo::availablePorts()) {
            // Check if the port is open
            if (port.isValid() && !port.isBusy()) {
                // Set the port name to the first open port found
                serialPort->setPortName(port.portName());

                // Retrieve port information
                QSerialPortInfo portInfo(port.portName());
                // Update line edits with port information
                portNameLineEdit->setText(portInfo.portName());
                descriptionLineEdit->setText(portInfo.description());
                serialNumberLineEdit->setText(portInfo.serialNumber());
                manufacturerLineEdit->setText(portInfo.manufacturer());

                QString vidString = QString::number(portInfo.vendorIdentifier(), 16);
                QString pidString = QString::number(portInfo.productIdentifier(), 16);
                vidLineEdit->setText(vidString);
                pidLineEdit->setText(pidString);

                // Break the loop after finding and setting port information
                break;
            }
        }

        // If no open port is found, use a default port (COM5)
        if (serialPort->portName().isEmpty()) {
            serialPort->setPortName("COM5");

            // Set line edits to indicate default port
            portNameLineEdit->setText("Connection Unavailable");
            descriptionLineEdit->setText("Connection Unavailable");
            serialNumberLineEdit->setText("Connection Unavailable");
            manufacturerLineEdit->setText("Connection Unavailable");
            vidLineEdit->setText("N/A");
            pidLineEdit->setText("N/A");
        }

        // Try to open the serial port
        if (serialPort->open(QIODevice::ReadOnly)) {
            // Serial port opened successfully
            ui->vcomStatus->setColor(Qt::green);

            // Enable line edits
            portNameLineEdit->setEnabled(true);
            descriptionLineEdit->setEnabled(true);
            serialNumberLineEdit->setEnabled(true);
            manufacturerLineEdit->setEnabled(true);
        } else {
            // Failed to open serial port
            qWarning() << "Failed to open serial port:" << serialPort->errorString();
            ui->vcomStatus->setColor(Qt::red);

            // Set line edits to indicate connection unavailable
            portNameLineEdit->setEnabled(false);
            descriptionLineEdit->setEnabled(false);
            serialNumberLineEdit->setEnabled(false);
            manufacturerLineEdit->setEnabled(false);
        }
    }
}

void xlat_evtool::checkConnectionStatus() {
    checkAndOpenSerialPort();
}

void xlat_evtool::handleError(QSerialPort::SerialPortError error) {
    if (error == QSerialPort::ResourceError) {

        //qDebug() << "Serial port disconnected.";

        ui->vcomStatus->setColor(Qt::red);

        portNameLineEdit->setText("Connection Unavailable");
        descriptionLineEdit->setText("Connection Unavailable");
        serialNumberLineEdit->setText("Connection Unavailable");
        manufacturerLineEdit->setText("Connection Unavailable");
        vidLineEdit->setText("N/A");
        pidLineEdit->setText("N/A");

        // Attempt to reconnect the serial port
        serialPort->close();
        checkAndOpenSerialPort();

        // Restart the timer
        connectionCheckTimer->start(1000);
    }
}


void xlat_evtool::readSerialData() {

    checkAndOpenSerialPort();

    QByteArray data = serialPort->readAll();

    // Emit a signal containing the raw serial port input
    emit serialDataReceived(data);

    // Converting QByteArray to QString
    QString dataString(data);

    // Parsing logic
    QStringList dataList = dataString.split(';');

    if (dataList.size() >= 4) {

        myData.reportNumber = dataList[0].toInt();
        myData.latency = dataList[1].toInt();
        myData.avgLatency = dataList[2].toInt();
        myData.stdev = dataList[3].toInt();

        /*
        // Print the parsed xlatData using qDebug
        qDebug() << "Parsed xlatData:";
        qDebug() << "Report Number:" << myData.reportNumber;
        qDebug() << "Latency:" << myData.latency;
        qDebug() << "Average Latency:" << myData.avgLatency;
        qDebug() << "Standard Deviation:" << myData.stdev;
        */

        // Add myData to the allData vector
        allData.push_back(myData);
        dataInterpolation();
        updateTableViewDynamic(myData);
    }
}

void xlat_evtool::updateTableViewDynamic(const xlatData& newData) {

    int row = allData.size() - 1; // Get the index of the last row

    // Insert a new row at the end of the model
    model->insertRow(row);

    // Populate the new row with data from newData
    model->setItem(row, 0, new QStandardItem(QString::number(newData.reportNumber)));
    model->setItem(row, 1, new QStandardItem(QString::number(newData.latency)));
    model->setItem(row, 2, new QStandardItem(QString::number(newData.avgLatency)));
    model->setItem(row, 3, new QStandardItem(QString::number(newData.stdev)));

    // Set the model for the tableView
    tableView->setModel(model);
    //tableView->resizeColumnsToContents();  //Morte a chi ha creato questa funzione.

    // Scroll to the bottom to show the latest entry
    tableView->scrollToBottom();
    tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed); // Column 0 has a fixed size
    tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch); // Column 1 will stretch to fill available space
    tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed); // Column 2 has a fixed size
    tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch); // Column 3 will stretch to fill available space
}


void xlat_evtool::updateTableView() {

    // Loading allData vector in model
    for (int row = 0; row < allData.size(); ++row) {
        model->setItem(row, 0, new QStandardItem(QString::number(allData[row].reportNumber)));
        model->setItem(row, 1, new QStandardItem(QString::number(allData[row].latency)));
        model->setItem(row, 2, new QStandardItem(QString::number(allData[row].avgLatency)));
        model->setItem(row, 3, new QStandardItem(QString::number(allData[row].stdev)));
    }
    tableView->setModel(model);

    tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed); // Column 0 has a fixed size
    tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch); // Column 1 will stretch to fill available space
    tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed); // Column 2 has a fixed size
    tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch); // Column 3 will stretch to fill available space

    tableView->scrollToBottom();
}

void xlat_evtool::saveCSV() {

    QString filePath = QFileDialog::getSaveFileName(this, tr("Save CSV File"), "", tr("CSV Files (*.csv)"));

    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);

            // Adding data to CSVs, no need to use this program each time you want to see data
            out << "Minimum Latency: " << QString::number(minLatency) << "\n";
            out << "Maximum Latency: " << QString::number(maxLatency) << "\n";
            out << "p5: " << QString::number(p5Value) << "\n";
            out << "p10: " << QString::number(p10Value) << "\n";
            out << "p90: " << QString::number(p90Value) << "\n";
            out << "p95: " << QString::number(p95Value) << "\n";
            out << "IQR: " << QString::number(iqrValue) << "\n";
            out << "MAD: " << QString::number(madValue) << "\n";
            out << "Average Latency: " << QString::number(avgLatency) << "\n";
            out << "Median Latency: " << QString::number(medianLatency) << "\n";
            out << "STDEV : " << QString::number(stdev) << "\n";
            out << "\n";

            for (const auto& data : allData) {
                out << data.reportNumber << "," << data.latency << "," << data.avgLatency << "," << data.stdev << "\n";
            }

            file.close();

            //qDebug() << "CSV file saved successfully at:" << filePath;
        } else {
            qWarning() << "Failed to open file for writing:" << file.errorString();
        }
    }
}

void xlat_evtool::importCsv(const QString& filePath) {
    clearData();

    QFile file(filePath);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);

        in.readLine();

        int row = 1;

        while (!in.atEnd()) {
            row++;
            QString line = in.readLine();
            QStringList fields = line.split(',');

            // Ensure that there are enough fields, fails first 13 information fields
            if (fields.size() >= 4) {
                xlatData newData;

                // Check if each field can be converted to an integer
                bool conversionError = false;
                for (int i = 0; i < 4; ++i) {
                    bool ok;
                    fields[i].toInt(&ok);
                    if (!ok) {
                        QMessageBox::critical(nullptr, "Import Error", "Non-numeric data in field " + QString::number(i) + " of line " + QString::number(row));
                        conversionError = true;
                        break;
                    }
                }

                if (conversionError) {
                    clearData();
                    return;
                }

                newData.reportNumber = fields[0].toInt();
                newData.latency = fields[1].toInt();
                newData.avgLatency = fields[2].toInt();
                newData.stdev = fields[3].toInt();
                stdev = newData.stdev; // native XLAT average and STDEV data is 100% accurate according to my tests
                allData.push_back(newData);
                dataInterpolation();
            }
        }

        file.close();
        updateTableView();
    } else {
        qWarning() << "Failed to open file for reading:" << file.errorString();
    }
}



void xlat_evtool::handleCsvImport() {

    QString filePath = QFileDialog::getOpenFileName(this, tr("Open CSV File"), "", tr("CSV Files (*.csv)"));

    if (!filePath.isEmpty()) {
        importCsv(filePath);
    } else {
        //qDebug() << "No file selected for import.";
    }
}

void xlat_evtool::dataInterpolation() {

    std::vector<int> latencies;
    for (const auto& d : allData) {
        latencies.push_back(d.latency);
    }
    std::sort(latencies.begin(), latencies.end());

    int size = latencies.size();

    double sumLatency = 0.0;
    double sumSqrt = 0.0;
    double medianLatency = 0.0;
    double madValue = 0.0;

    for (const auto& d : allData) {
        sumLatency += d.latency;
        double difference = d.latency - latencies[size / 2];
        madValue += std::abs(difference);
        sumSqrt += difference * difference;
    }

    double meanLatency = sumLatency / size;

    double p90_index = 0.90 * size;
    double p95_index = 0.95 * size;
    double p5_index = 0.05 * size;
    double p10_index = 0.10 * size;
    double q1_index = 0.25 * size;
    double q3_index = 0.75 * size;

    p90Value = latencies[std::round(p90_index)];
    p95Value = latencies[std::round(p95_index)];
    p5Value = latencies[std::round(p5_index)];
    p10Value = latencies[std::round(p10_index)];
    iqrValue = latencies[std::round(q3_index)] - latencies[std::round(q1_index)];
    maxLatency = latencies.back();
    minLatency = latencies.front();
    avgLatency = meanLatency;

    if (size % 2 == 0) {
        medianLatency = (latencies[size / 2 - 1] + latencies[size / 2]) / 2;
    } else {
        medianLatency = latencies[size / 2];
    }

    // Calculate MAD
    madValue /= size;

    // Calculate standard deviation
    stdev = static_cast<int>(std::sqrt(sumSqrt / size));

    updatePercentileData(p90Value, p95Value, p5Value, p10Value, iqrValue,
                         maxLatency, minLatency, avgLatency, medianLatency,
                         madValue);

    _counterCall++; // Increment counter to prevent malformed data visualization due to low data pool
}


void xlat_evtool::updatePercentileData(int p90Value, int p95Value, int p5Value, int p10Value, int iqrValue,
                                       int maxLatency, int minLatency, double avgLatency, int medianLatency,
                                       int madValue) {

    if(_counterCall>7){
    p90LineEdit->setText(QString::number(p90Value));
    }else{
    p90LineEdit->setText("need more data");
    }

    if(_counterCall>9){
    p95LineEdit->setText(QString::number(p95Value));
    }else{
    p95LineEdit->setText("need more data");
    }

    if(_counterCall>3){
    iqrLineEdit->setText(QString::number(iqrValue));
    }else{
    iqrLineEdit->setText("need more data");
    }


    p5LineEdit->setText(QString::number(p5Value));
    p10LineEdit->setText(QString::number(p10Value));
    maxLatLineEdit->setText(QString::number(maxLatency));
    minLatLineEdit->setText(QString::number(minLatency));
    medLatLineEdit->setText(QString::number(medianLatency));
    avgLatLineEdit->setText(QString::number(avgLatency));
    stdevLineEdit->setText(QString::number(stdev));
    avgMadLineEdit->setText(QString::number(madValue));
}


void xlat_evtool::clearData() {

    model->removeRows(0, model->rowCount());

    allData.clear();

    // Clearing QLineEdit fields
    p90LineEdit->clear();
    p95LineEdit->clear();
    p5LineEdit->clear();
    p10LineEdit->clear();
    iqrLineEdit->clear();
    maxLatLineEdit->clear();
    minLatLineEdit->clear();
    avgMadLineEdit->clear();
    medLatLineEdit->clear();
    stdevLineEdit->clear();
    avgLatLineEdit->clear();

    _counterCall = 0;

    minLatency = 0;
    maxLatency = 0;
    p90Value = 0;
    p95Value = 0;
    p5Value = 0;
    p10Value = 0;
    iqrValue = 0;
    madValue = 0;
    avgLatency = 0;
    medianLatency = 0;
    stdev = 0;

}


void xlat_evtool::openGitHubLink() {
    QUrl githubUrl("https://github.com/FNNN98/xlat-Evtool");
    QDesktopServices::openUrl(githubUrl);
}

void xlat_evtool::disclaimer() {

    QDialog *disclaimerDialog = new QDialog();
    // title
    disclaimerDialog->setWindowTitle("Disclaimer");
    QVBoxLayout *layout = new QVBoxLayout(disclaimerDialog);

    QLabel *disclaimerLabel = new QLabel();
    disclaimerLabel->setText("                                   XLAT Branding Notice:\n\n"
                             "The XLAT branding used in this application is owned by Finalmouse.\n\n "
                             "I do not claim any ownership or rights to the XLAT branding, and its use here \n"
                             "is solely for testing purposes related to the interoperability of this application\n"
                             "with the XLAT tool.\n\n");

    layout->addWidget(disclaimerLabel);

    // Create a QPushButton to close the dialog
    QPushButton *closeButton = new QPushButton("Close");
    connect(closeButton, &QPushButton::clicked, disclaimerDialog, &QDialog::close);

    layout->addWidget(closeButton);

    disclaimerDialog->setLayout(layout);

    disclaimerDialog->setModal(true);

    disclaimerDialog->exec();

    delete disclaimerDialog;
}
void xlat_evtool::showScatterChartWindow() {

    QtCharts::QChart *scatterChart = new QtCharts::QChart();
    scatterChart->setTitle("Latency Scatter Chart");
    scatterChart->setBackgroundBrush(QBrush(Qt::white));

    QtCharts::QValueAxis *xAxis = new QtCharts::QValueAxis();
    QtCharts::QValueAxis *yAxis = new QtCharts::QValueAxis();

    yAxis->setRange(0, maxLatency * 1.1); // graph 10% higher than max data, prevents splitted data points

    scatterChart->addAxis(xAxis, Qt::AlignBottom);
    scatterChart->addAxis(yAxis, Qt::AlignLeft);

    QtCharts::QScatterSeries *scatterSeries = new QtCharts::QScatterSeries();
    scatterSeries->setMarkerSize(4);
    scatterSeries->setPen(Qt::NoPen);

    for (const auto& data : allData) {
        QPointF point(data.reportNumber, data.latency);
        scatterSeries->append(point);
    }

    // Set color to blue
    scatterSeries->setColor(Qt::blue);

    scatterChart->addSeries(scatterSeries);
    scatterSeries->attachAxis(xAxis);
    scatterSeries->attachAxis(yAxis);

    QtCharts::QChartView *chartView = new QtCharts::QChartView(scatterChart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QMainWindow *chartWindow = new QMainWindow();
    chartWindow->setWindowTitle("Scatter Chart");
    chartWindow->setCentralWidget(chartView);

    chartWindow->resize(1600, 900);
    chartWindow->show();
}


void xlat_evtool::showHistogramWindow() {

    QMainWindow *histogramWindow = new QMainWindow();
    histogramWindow->setWindowTitle("Histogram");

    QWidget *centralWidget = new QWidget();
    histogramWindow->setCentralWidget(centralWidget);

    QVBoxLayout *layout = new QVBoxLayout();
    centralWidget->setLayout(layout);

    histogramWindow->resize(1600, 900);
    histogramWindow->show();

    int range = maxLatency - minLatency;

    int step = range / 16;

    std::vector<int> variables(16);

    for (int i = 0; i < 16; ++i) {
        variables[i] = static_cast<int>(minLatency + i * step);
        QString valueString = QString::number(variables[i]);
        //qDebug() << valueString;
    }

    QBarSet *set0 = new QBarSet(QString::number(minLatency) + "-" + QString::number(variables[1]));
    QBarSet *set1 = new QBarSet(QString::number(variables[1]) + "-" + QString::number(variables[2]));
    QBarSet *set2 = new QBarSet(QString::number(variables[2]) + "-" + QString::number(variables[3]));
    QBarSet *set3 = new QBarSet(QString::number(variables[3]) + "-" + QString::number(variables[4]));
    QBarSet *set4 = new QBarSet(QString::number(variables[4]) + "-" + QString::number(variables[5]));
    QBarSet *set5 = new QBarSet(QString::number(variables[5]) + "-" + QString::number(variables[6]));
    QBarSet *set6 = new QBarSet(QString::number(variables[6]) + "-" + QString::number(variables[7]));
    QBarSet *set7 = new QBarSet(QString::number(variables[7]) + "-" + QString::number(variables[8]));
    QBarSet *set8 = new QBarSet(QString::number(variables[8]) + "-" + QString::number(variables[9]));
    QBarSet *set9 = new QBarSet(QString::number(variables[9]) + "-" + QString::number(variables[10]));
    QBarSet *set10 = new QBarSet(QString::number(variables[10]) + "-" + QString::number(variables[11]));
    QBarSet *set11 = new QBarSet(QString::number(variables[11]) + "-" + QString::number(variables[12]));
    QBarSet *set12 = new QBarSet(QString::number(variables[12]) + "-" + QString::number(variables[13]));
    QBarSet *set13 = new QBarSet(QString::number(variables[13]) + "-" + QString::number(variables[14]));
    QBarSet *set14 = new QBarSet(QString::number(variables[14]) + "-" + QString::number(variables[15]));
    QBarSet *set15 = new QBarSet(QString::number(variables[15]) + "-" + QString::number(maxLatency));

    int bar1 = 0;
    int bar2 = 0;
    int bar3 = 0;
    int bar4 = 0;
    int bar5 = 0;
    int bar6 = 0;
    int bar7 = 0;
    int bar8 = 0;
    int bar9 = 0;
    int bar10 = 0;
    int bar11 = 0;
    int bar12 = 0;
    int bar13 = 0;
    int bar14 = 0;
    int bar15 = 0;
    int bar16 = 0;

    for (const auto& data : allData) {
        if (data.latency >= variables[0] && data.latency < variables[1]) {
            bar1++;
        } else if (data.latency >= variables[1] && data.latency < variables[2]) {
            bar2++;
        } else if (data.latency >= variables[2] && data.latency < variables[3]) {
            bar3++;
        } else if (data.latency >= variables[3] && data.latency < variables[4]) {
            bar4++;
        } else if (data.latency >= variables[4] && data.latency < variables[5]) {
            bar5++;
        } else if (data.latency >= variables[5] && data.latency < variables[6]) {
            bar6++;
        } else if (data.latency >= variables[6] && data.latency < variables[7]) {
            bar7++;
        } else if (data.latency >= variables[7] && data.latency < variables[8]) {
            bar8++;
        } else if (data.latency >= variables[8] && data.latency < variables[9]) {
            bar9++;
        } else if (data.latency >= variables[9] && data.latency < variables[10]) {
            bar10++;
        } else if (data.latency >= variables[10] && data.latency < variables[11]) {
            bar11++;
        } else if (data.latency >= variables[11] && data.latency < variables[12]) {
            bar12++;
        } else if (data.latency >= variables[12] && data.latency < variables[13]) {
            bar13++;
        } else if (data.latency >= variables[13] && data.latency < variables[14]) {
            bar14++;
        } else if (data.latency >= variables[14] && data.latency < variables[15]) {
            bar15++;
        } else if (data.latency >= variables[15]) {
            bar16++;
        }
    }

    *set0 << bar1;
    *set1 << bar2;
    *set2 << bar3;
    *set3 << bar4;
    *set4 << bar5;
    *set5 << bar6;
    *set6 << bar7;
    *set7 << bar8;
    *set8 << bar9;
    *set9 << bar10;
    *set10 << bar11;
    *set11 << bar12;
    *set12 << bar13;
    *set13 << bar14;
    *set14 << bar15;
    *set15 << bar16;


    QBarSeries *series = new QBarSeries();
    series->append(set0);
    series->append(set1);
    series->append(set2);
    series->append(set3);
    series->append(set4);
    series->append(set5);
    series->append(set6);
    series->append(set7);
    series->append(set8);
    series->append(set9);
    series->append(set10);
    series->append(set11);
    series->append(set12);
    series->append(set13);
    series->append(set14);
    series->append(set15);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Latency data point distribution");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QStringList categories;
    categories << "TEST";

    QBarCategoryAxis *axisX = new QBarCategoryAxis();

    axisX->setGridLineVisible(true); // already visible, forcing it
    axisX->setMinorGridLineVisible(true); //NOT WORKING, contact me if you find a solution, I'm using QT 5.15 (!!!)
    axisX->setGridLineColor(Qt::gray);
    axisX->setGridLinePen(QPen(Qt::DotLine));

    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();

    axisY->setGridLineVisible(true);
    //axisY->setMinorGridLineVisible(true);
    axisY->setGridLineColor(Qt::gray);
    axisY->setGridLinePen(QPen(Qt::DotLine));
    //setting up plot height based on max bar value
    axisY->setRange(0, std::max({bar1, bar2, bar3, bar4, bar5, bar6, bar7, bar8, bar9, bar10, bar11, bar12, bar13, bar14, bar15, bar16}));
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->setAcceptHoverEvents(true);

    // Bar color gradient
    set0->setColor(QColor(0, 128, 0));
    set1->setColor(QColor(85, 170, 0));
    set2->setColor(QColor(128, 212, 0));
    set3->setColor(QColor(170, 255, 0));
    set4->setColor(QColor(213, 255, 0));
    set5->setColor(QColor(255, 255, 0));
    set6->setColor(QColor(255, 213, 0));
    set7->setColor(QColor(255, 170, 0));
    set8->setColor(QColor(255, 128, 0));
    set9->setColor(QColor(255, 85, 0));
    set10->setColor(QColor(255, 0, 0));
    set11->setColor(QColor(212, 0, 0));
    set12->setColor(QColor(170, 0, 0));
    set13->setColor(QColor(128, 0, 0));
    set14->setColor(QColor(85, 0, 0));
    set15->setColor(QColor(43, 0, 0));

    QChartView *chartView = new QChartView(chart);

    chartView->setRenderHint(QPainter::Antialiasing);

    layout->addWidget(chartView);
}
