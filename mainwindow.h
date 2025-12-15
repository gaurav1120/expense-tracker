#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QStackedWidget>
#include <QMap>
#include <QString>
#include <QDate>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void on_add_expense_clicked();
    void on_home_clicked();
    void on_daily_expense_clicked();
    void on_monthly_expense_clicked();
    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::MainWindow *ui;
    void updateRecentExpenses(const QString &filePath);
    void updateMonthlySummary(const QString &type_name, double amount, const QDate &date);
    void createCharts(double grandTotal, double food, double transport,
                        double rental, double education, double lifestyle,
                        double medical, double other);
    void showDailyExpenses(const QString &filePath);
    void loadMonthlySummary(const QDate &date);
    void showMonthlyExpenses( QString month,QString year);
};

#endif // MAINWINDOW_H
