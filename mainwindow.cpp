#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QToolTip>
#include <QPainter>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->pages->setCurrentIndex(0);
    // Sidebar styling
    ui->sideBox->setStyleSheet("background-color: #2D2D3A;");
    ui->dateEdit->setDate(QDate::currentDate());
    ui->dateEdit_2->setDate(QDate::currentDate());
    ui->head_amount->setStyleSheet("background-color: #2D2D3A;");
    ui->head_time->setStyleSheet("background-color: #2D2D3A;");
    ui->head_type->setStyleSheet("background-color: #2D2D3A;");
    loadMonthlySummary(QDate::currentDate());

    // Input validation
    QDoubleValidator *validator = new QDoubleValidator(0.00, 10000000.00, 2, this);
    validator->setNotation(QDoubleValidator::StandardNotation);
    ui->line_amount->setValidator(validator);


    QDate selectedDate = ui->dateEdit->date();
    QString yearStr  = selectedDate.toString("yyyy");
    QString monthStr = selectedDate.toString("MM");
    QString dayStr   = selectedDate.toString("dd");

    QString filePath = QDir::homePath() + "/Documents/Expenses/" + yearStr + "/" + monthStr + "/" + dayStr + ".txt";

    if (QFile::exists(filePath)) {
        updateRecentExpenses(filePath);
    }


};

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    qDebug() << "--------------------------------------------------";
    QString value = ui->line_amount->text();
    QString type_name = ui->comboBox->currentText();
    QString info = ui->info_text->toPlainText();
    QDate selectedDate = ui->dateEdit->date();
    QString Date = selectedDate.toString("yyyy-MM-dd");
    QString yearStr  = selectedDate.toString("yyyy");
    QString monthStr = selectedDate.toString("MM");
    QString dayStr   = selectedDate.toString("dd");
    QString basePath = QDir::homePath() + "/Documents/Expenses";
    QString amountStr;
    if(value != "")
    {
        if (!value.startsWith("₹ ") && !value.isEmpty())
        {
            value = "₹ " + value;
            ui->line_amount->setText(value);
            qDebug() << value;
        }

        // Daily file
        QString path = basePath + "/" + yearStr + "/" + monthStr;
        QDir dir;
        if (!dir.mkpath(path))
        {
            qDebug() << "Failed to create directory:" << path;
            return;
        }

        QString filePath = path + "/" + dayStr + ".txt";
        QFile file(filePath);

        if (file.open(QIODevice::Append | QIODevice::Text))
        {
            QTextStream out(&file);
            out << QTime::currentTime().toString("hh:mm:ss")
                << " | " << value
                << " | " << type_name
                << " | " << Date
                << " | " << info
                << " | " << "\n";
            file.close();
            qDebug() << "Expense saved to:" << filePath;

            updateRecentExpenses(filePath);

            amountStr = value;

        }
        else
            {
                qDebug() << "Could not open file for writing:" << filePath;
            }

        // Monthly summary file
        QString path2 = basePath + "/" + yearStr + "/" + monthStr;
        QDir dir2;
        if (!dir2.mkpath(path2))
            {
                qDebug() << "Failed to create directory:" << path2;
                return;
            }

        QString filePath2 = path2 + "/monthlydata.txt"; // removed extra slash
        QFile file2(filePath2);
        if (file2.open(QIODevice::Append | QIODevice::Text))
            {
                QTextStream out(&file2);
                out << QTime::currentTime().toString("hh:mm:ss")
                    << " | " << value
                    << " | " << type_name
                    << " | " << Date
                    << " | " << info
                    << " | " << "\n";
                file2.close();
                qDebug() << "Expense saved to:" << filePath2;

                updateRecentExpenses(filePath2);

                amountStr.remove("₹");
                amountStr.remove(" ");
                double Monthly_amount = amountStr.toDouble();
                updateMonthlySummary(type_name, Monthly_amount, selectedDate);
        }
        else
            {
                qDebug() << "Could not open file for writing:" << filePath2;
            }
     }
}


void MainWindow::updateRecentExpenses(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    QStringList lines;
    while (!in.atEnd())
        {
            lines << in.readLine();
        }
    file.close();

    QStringList lastEntries = lines.mid(qMax(0, lines.size() - 3));

    QLabel* amountLabels[] = {ui->labelAmount1, ui->labelAmount2, ui->labelAmount3};
    QLabel* typeLabels[]   = {ui->labelType1, ui->labelType2, ui->labelType3};
    QLabel* dateLabels[]   = {ui->labeldate1, ui->labeldate2, ui->labeldate3};

    for (int i = 0; i < 3; i++) {
        int index = lastEntries.size() - 1 - i; // Start from last entry
        if (index >= 0) {
            QStringList parts = lastEntries[index].split("|");
            if (parts.size() >= 5) {
                amountLabels[i]->setText(parts[1].trimmed()); // Amount
                typeLabels[i]->setText(parts[2].trimmed());   // Type
                dateLabels[i]->setText(parts[3].trimmed());   // Date
            }
        } else {
            amountLabels[i]->clear();
            typeLabels[i]->clear();
            dateLabels[i]->clear();
        }
    }
}

void MainWindow::showDailyExpenses(const QString &filePath)
{
    if (ui->scrollAreaDaily->widget()) {
        delete ui->scrollAreaDaily->widget();
    }

    QWidget *container = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(container);

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QLabel *noData = new QLabel("No expenses recorded for this date.");
        noData->setStyleSheet("color: gray; padding: 10px;");
        layout->addWidget(noData);
    }
        else
        {

            QTextStream in(&file);

            // --- ADD HEADING ROW ---
            QWidget *headingRow = new QWidget;
            QHBoxLayout *headingLayout = new QHBoxLayout(headingRow);
            headingLayout->setContentsMargins(5, 2, 5, 2);

            QLabel *amountHeader = new QLabel("Amount");
            QLabel *typeHeader   = new QLabel("Type");
            QLabel *infoHeader   = new QLabel("Info");
            headingRow->setStyleSheet("background-color: #2D2D3A; padding: 6px; border-radius: 5px;");
            QString headerStyle = "font-weight: bold; color: white; font-size: 14px;";
            amountHeader->setStyleSheet(headerStyle);
            typeHeader->setStyleSheet(headerStyle);
            infoHeader->setStyleSheet(headerStyle);

            headingLayout->addWidget(amountHeader, 0, Qt::AlignCenter);
            headingLayout->addWidget(typeHeader, 0, Qt::AlignCenter);
            headingLayout->addWidget(infoHeader, 0, Qt::AlignCenter);

            layout->addWidget(headingRow); // Add heading before list items


            QStringList lines;
            while (!in.atEnd())
            {
                lines << in.readLine().trimmed();
            }
            file.close();

            std::reverse(lines.begin(), lines.end());  // <-- Reverse the list

            for (const QString &line : lines) {
                if (line.isEmpty()) continue;
                QStringList parts = line.split("|");
                if (parts.size() >= 5) {
                    QString amount = parts[1].trimmed();
                    QString type   = parts[2].trimmed();
                    QString info   = parts[4].trimmed();

                    QWidget *row = new QWidget;
                    QHBoxLayout *hLayout = new QHBoxLayout(row);
                    hLayout->setContentsMargins(5, 2, 5, 2);

                    QLabel *amountLabel = new QLabel(amount);
                    QLabel *typeLabel   = new QLabel(type);
                    QLabel *infoLabel   = new QLabel(info);

                    amountLabel->setStyleSheet("color: lightgreen;");
                    typeLabel->setStyleSheet("color: lightblue;");
                    infoLabel->setStyleSheet("color: white;");

                    hLayout->addWidget(amountLabel, 0, Qt::AlignCenter);
                    hLayout->addWidget(typeLabel, 0, Qt::AlignCenter);
                    hLayout->addWidget(infoLabel, 0, Qt::AlignCenter);

                    layout->addWidget(row);
                }
            }
        layout->addStretch();

        file.close();
    }

    container->setLayout(layout);
    ui->scrollAreaDaily->setWidget(container);
}

void MainWindow::updateMonthlySummary(const QString &type_name, double amount, const QDate &date)
{
    QString monthFilePath = QDir::homePath() + "/Documents/Expenses/" +date.toString("yyyy") + "/" +date.toString("MM") + "/monthly_summary.txt";

    // Initialize all category totals
    double food = 0, transport = 0, rental = 0, education = 0;
    double lifestyle = 0, medical = 0, other = 0, grandTotal = 0;

    // Load existing data
    QFile file(monthFilePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith("Food & Beverages")) food = line.section(":", 1, 1).trimmed().toDouble();
            else if (line.startsWith("Transportation")) transport = line.section(":", 1, 1).trimmed().toDouble();
            else if (line.startsWith("Rental")) rental = line.section(":", 1, 1).trimmed().toDouble();
            else if (line.startsWith("Education")) education = line.section(":", 1, 1).trimmed().toDouble();
            else if (line.startsWith("Lifestyle & Personal")) lifestyle = line.section(":", 1, 1).trimmed().toDouble();
            else if (line.startsWith("Medical & Health")) medical = line.section(":", 1, 1).trimmed().toDouble();
            else if (line.startsWith("Other")) other = line.section(":", 1, 1).trimmed().toDouble();
            else if (line.startsWith("Grand Total")) grandTotal = line.section(":", 1, 1).trimmed().toDouble();
        }
        file.close();
    }
     qDebug()<<amount;
    // Update category based on type_name
    if (type_name == "Food & Beverages")
        food += amount;
    else if (type_name == "Transportation")
        transport += amount;
    else if (type_name == "Rental")
        rental += amount;
    else if (type_name == "Education")
        education += amount;
    else if (type_name == "Lifestyle & Personal")
        lifestyle += amount;
    else if (type_name == "Medical & Health")
        medical += amount;
    else
        other+= amount;

    grandTotal = food + transport + rental + education + lifestyle + medical + other;

    // Save updated data back to file
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "Food & Beverages: " << food << "\n";
        out << "Transportation: " << transport << "\n";
        out << "Rental: " << rental << "\n";
        out << "Education: " << education << "\n";
        out << "Lifestyle & Personal: " << lifestyle << "\n";
        out << "Medical & Health: " << medical << "\n";
        out << "Other: " << other << "\n";
        out << "Grand Total: " << grandTotal << "\n";
        file.close();
    }
    qDebug()<<medical;
    // Refresh pie chart
    createCharts(grandTotal, food, transport, rental, education, lifestyle, medical, other);
}

void MainWindow::createCharts(double grandTotal, double food, double transport,double rental, double education, double lifestyle,double medical, double other)
    {
    //qDebug()<<food;
    // Pastel Colors
    QStringList pastelColors = {"#FF5733", "#FFC300", "#DAF7A6", "#33FFBD", "#3380FF", "#9D33FF", "#FF33A8", "#FF914D"};

    // --- PIE CHART ---
    QPieSeries *pieSeries = new QPieSeries();
    QList<double> values = {food, transport, rental, education, lifestyle, medical, other};
    QStringList categories = {
        "Food & Beverages", "Transportation", "Rental",
        "Education", "Lifestyle & Personal", "Medical & Health", "Other"
    };

    if (grandTotal == 0) {
        pieSeries->append("No Data", 1);
    } else {
        for (int i = 0; i < values.size(); ++i) {
            if (values[i] > 0)
                pieSeries->append(categories[i], values[i]);
        }
    }

    for (int i = 0; i < pieSeries->slices().size(); ++i) {
        pieSeries->slices().at(i)->setLabelVisible(true);
        pieSeries->slices().at(i)->setBrush(QColor(pastelColors[i % pastelColors.size()]));
    }

    QChart *pieChart = new QChart();
    pieChart->addSeries(pieSeries);
    pieChart->setBackgroundBrush(Qt::transparent);
    pieChart->legend()->setAlignment(Qt::AlignRight);

    QChartView *pieView = new QChartView(pieChart);
    pieView->setRenderHint(QPainter::Antialiasing);

    if (!ui->pieChartWidget->layout()) {
        QVBoxLayout *layout = new QVBoxLayout(ui->pieChartWidget);
        layout->addWidget(pieView);
    } else {
        QLayoutItem *child;
        while ((child = ui->pieChartWidget->layout()->takeAt(0)) != nullptr)
            delete child->widget();
        ui->pieChartWidget->layout()->addWidget(pieView);
    }

    // --- BAR CHART ---
    QBarSeries *barSeries = new QBarSeries();

    for (int i = 0; i < values.size(); ++i) {
        QBarSet *set = new QBarSet(categories[i]);
        *set << values[i]; // only one value per set
        set->setColor(pastelColors[i % pastelColors.size()]);
        barSeries->append(set);
    }

    QChart *barChart = new QChart();
    barChart->addSeries(barSeries);
    barChart->setBackgroundBrush(Qt::transparent);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    QStringList emptyCategory;
    emptyCategory << "";
    axisX->append(emptyCategory);
    barChart->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    barChart->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisY);

    QChartView *barView = new QChartView(barChart);
    barView->setRenderHint(QPainter::Antialiasing);

    if (!ui->barChartWidget->layout()) {
        QVBoxLayout *layout = new QVBoxLayout(ui->barChartWidget);
        layout->addWidget(barView);
    } else {
        QLayoutItem *child;
        while ((child = ui->barChartWidget->layout()->takeAt(0)) != nullptr)
            delete child->widget();
        ui->barChartWidget->layout()->addWidget(barView);
    }


    // --- GROUP BOX COLORS (MATCH PIE COLORS) ---
    ui->food_label->setStyleSheet("background-color: " + pastelColors[0] + ";");
    ui->label_13->setStyleSheet("background-color: " + pastelColors[1] + ";");
    ui->label_12->setStyleSheet("background-color: " + pastelColors[2] + ";");
    ui->label_10->setStyleSheet("background-color: " + pastelColors[3] + ";");
    ui->label_8->setStyleSheet("background-color: " + pastelColors[4] + ";");
    ui->label_6->setStyleSheet("background-color: " + pastelColors[5] + ";");
    ui->label_18->setStyleSheet("background-color: " + pastelColors[6] + ";");
    qDebug()<<grandTotal;

    // --- Update Labels ---
    ui->label_5->setText("₹ "+QString::number(grandTotal, 'f', 2));
    ui->food_label->setText("₹ "+QString::number(food, 'f', 2));
    ui->label_13->setText("₹ "+QString::number(transport, 'f', 2));
    ui->label_12->setText("₹ "+QString::number(rental, 'f', 2));
    ui->label_10->setText("₹ "+QString::number(education, 'f', 2));
    ui->label_8->setText("₹ "+QString::number(lifestyle, 'f', 2));
    ui->label_6->setText("₹ "+QString::number(medical, 'f', 2));
    ui->label_18->setText("₹ "+QString::number(other, 'f', 2));
}


void MainWindow::loadMonthlySummary(const QDate &date)
{
    QString monthFilePath = QDir::homePath() + "/Documents/Expenses/" +
                            date.toString("yyyy") + "/" +
                            date.toString("MM") + "/monthly_summary.txt";

    QFile file(monthFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // No file yet → show empty chart
        createCharts(0, 0, 0, 0, 0, 0, 0, 0);
        return;
    }

    double food = 0, transport = 0, rental = 0, education = 0;
    double lifestyle = 0, medical = 0, other = 0, grandTotal = 0;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith("Food & Beverages")) food = line.section(":", 1, 1).trimmed().toDouble();
        else if (line.startsWith("Transportation")) transport = line.section(":", 1, 1).trimmed().toDouble();
        else if (line.startsWith("Rental")) rental = line.section(":", 1, 1).trimmed().toDouble();
        else if (line.startsWith("Education")) education = line.section(":", 1, 1).trimmed().toDouble();
        else if (line.startsWith("Lifestyle & Personal")) lifestyle = line.section(":", 1, 1).trimmed().toDouble();
        else if (line.startsWith("Medical & Health")) medical = line.section(":", 1, 1).trimmed().toDouble();
        else if (line.startsWith("Other")) other = line.section(":", 1, 1).trimmed().toDouble();
        else if (line.startsWith("Grand Total")) grandTotal = line.section(":", 1, 1).trimmed().toDouble();
    }
    file.close();
    createCharts(grandTotal, food, transport, rental, education, lifestyle, medical, other);


}



void MainWindow::showMonthlyExpenses( QString month,QString year)
{

    // Create full file path
    QString filePath =QDir::homePath() + "/Documents/Expenses/" + year + "/" + month +  + "/monthlydata.txt";

    if (ui->scrollAreaMonthly->widget()) {
        delete ui->scrollAreaMonthly->widget();
    }

    QWidget *container = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(container);

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QLabel *noData = new QLabel("No expenses recorded for this month.");
        noData->setStyleSheet("color: gray; padding: 10px;");
        layout->addWidget(noData);
    }
    else
    {
        QTextStream in(&file);

        // --- Heading Row ---
        QWidget *headingRow = new QWidget;
        QHBoxLayout *headingLayout = new QHBoxLayout(headingRow);
        headingLayout->setContentsMargins(5, 2, 5, 2);

        QLabel *amountHeader = new QLabel("Amount");
        QLabel *typeHeader   = new QLabel("Type");
        QLabel *infoHeader   = new QLabel("Info");
        QLabel *dateHeader   = new QLabel("Date");

        headingRow->setStyleSheet("background-color: #2D2D3A; padding: 6px; border-radius: 5px;");
        QString headerStyle = "font-weight: bold; color: white; font-size: 14px;";
        amountHeader->setStyleSheet(headerStyle);
        typeHeader->setStyleSheet(headerStyle);
        dateHeader->setStyleSheet(headerStyle);
        infoHeader->setStyleSheet(headerStyle);


        headingLayout->addWidget(amountHeader, 0, Qt::AlignCenter);
        headingLayout->addWidget(typeHeader, 0, Qt::AlignCenter);
        headingLayout->addWidget(dateHeader, 0, Qt::AlignCenter);
        headingLayout->addWidget(infoHeader, 0, Qt::AlignCenter);


        layout->addWidget(headingRow);

        QStringList lines;
        while (!in.atEnd()) {
            lines << in.readLine().trimmed();
        }
        file.close();

        std::reverse(lines.begin(), lines.end());

        for (const QString &line : lines)
        {
            if (line.isEmpty()) continue;
            QStringList parts = line.split("|");
            if (parts.size() >= 5)
            {

                QString amount = parts[1].trimmed();
                QString type   = parts[2].trimmed();
                QString date   = parts[3].trimmed();
                QString info   = parts[4].trimmed();

                QWidget *row = new QWidget;
                QHBoxLayout *hLayout = new QHBoxLayout(row);
                hLayout->setContentsMargins(5, 2, 5, 2);

                QLabel *amountLabel = new QLabel(amount);
                QLabel *typeLabel   = new QLabel(type);
                QLabel *dateLabel   = new QLabel(date);
                QLabel *infoLabel   = new QLabel(info);

                amountLabel->setStyleSheet("color: lightgreen;");
                typeLabel->setStyleSheet("color: lightblue;");
                dateLabel->setStyleSheet("color: yellow;");
                infoLabel->setStyleSheet("color: white;");


                hLayout->addWidget(amountLabel, 0, Qt::AlignCenter);
                hLayout->addWidget(typeLabel, 0, Qt::AlignCenter);
                hLayout->addWidget(dateLabel, 0, Qt::AlignCenter);
                hLayout->addWidget(infoLabel, 0, Qt::AlignCenter);


                layout->addWidget(row);
            }
        }
        layout->addStretch();
    }

    container->setLayout(layout);
    ui->scrollAreaMonthly->setWidget(container);
}


//------------------pages------------------------------------

void MainWindow::on_home_clicked()
{
    ui->pages->setCurrentIndex(0);
}

void MainWindow::on_add_expense_clicked()
{
    ui->pages->setCurrentIndex(1);
}

void MainWindow::on_daily_expense_clicked()
{

    QDate date = ui->dateEdit_2->date();
    {

        QString filePath = QDir::homePath() + "/Documents/Expenses/" +
                           date.toString("yyyy") + "/" +
                           date.toString("MM") + "/" +
                           date.toString("dd") + ".txt";
        showDailyExpenses(filePath);
    }
    ui->pages->setCurrentIndex(2);
}

void MainWindow::on_monthly_expense_clicked()
{

    QString month;
    int currentYear = QDate::currentDate().year();
    QString year=QString::number(currentYear);
    ui->date_lineEdit->setText(QString::number(currentYear));
    int currentMonth = QDate::currentDate().month();
    ui->comboBox_2->setCurrentIndex(currentMonth - 1);
    if(currentMonth<10)
    {
        month="0"+QString::number(currentMonth);
    }
    else
    {
        month=QString::number(currentMonth);
    }

    showMonthlyExpenses( month,  year);

    ui->pages->setCurrentIndex(3);
}

void MainWindow::on_pushButton_2_clicked()
{
    QDate date = ui->dateEdit_2->date();
    {
        QString filePath = QDir::homePath() + "/Documents/Expenses/" +
                           date.toString("yyyy") + "/" +
                           date.toString("MM") + "/" +
                           date.toString("dd") + ".txt";
        showDailyExpenses(filePath);
    }

}

void MainWindow::on_pushButton_3_clicked()
{
    QString month;
    int type_index =1+ ui->comboBox_2->currentIndex();
    if(type_index<10)
        {
            month="0"+QString::number(type_index);
        }
    else
        {
            month=QString::number(type_index);
        }
    QLineEdit *date_lineEdit = new QLineEdit(this);
    // Set validator for 4-digit year
    date_lineEdit->setValidator(new QIntValidator(2000, 2100, this));
    QString year=ui->date_lineEdit->text();


    showMonthlyExpenses(month,year);


}

