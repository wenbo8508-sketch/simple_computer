#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStack>
#include <QLabel>

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
    QString operand;
    QString opcode;
    QStack<QString> operands;
    QStack<QString> opcodes;

    QString calculation(bool *ok=NULL);

private slots:
    void btnNum_clicked();

    void on_point_clicked();

    void on_del_clicked();

    void on_C_clicked();

    void binaryOperatorClicked();

    void btnUnaryoperatorClicked();

    void on_sum_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
