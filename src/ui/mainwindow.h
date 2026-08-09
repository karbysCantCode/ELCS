#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_createNewCircuitButton_clicked();

    void on_selectTool_clicked();

    void on_pokeTool_clicked();

    void on_AddCurveButton_clicked();

    void on_AddLineButton_clicked();

    void on_AddLabelButton_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
