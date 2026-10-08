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

    //把 double 结果格式化成显示文本（15 位有效数字）
    static QString fromNumber(double value);
    //把按钮上显示的 × ÷ 1/X X^2 翻译成代码内部使用的运算符符号
    static QString operatorToken(const QString &buttonText);

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
    bool m_startNewOperand = false;     //按完 "=" 后下一次输入数字要重新开始算式
};
#endif // MAINWINDOW_H
