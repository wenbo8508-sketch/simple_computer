#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStack>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
class QKeyEvent;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    //键盘事件：把按键翻译成按钮对象名，复用鼠标点击的同一套槽函数
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void btnNum_clicked();

    void on_point_clicked();

    void on_del_clicked();

    void on_C_clicked();

    void on_CE_clicked();

    void binaryOperatorClicked();

    void btnUnaryoperatorClicked();

    void on_sum_clicked();

private:
    Ui::MainWindow *ui;

    //计算器的内部状态，全部收进 private，外部只能靠按键驱动（原来这些是 public 的）
    QString operand;                //当前正在输入的操作数
    QString opcode;                 //当前按下的运算符
    QStack<QString> operands;       //已经入栈的操作数
    QStack<QString> opcodes;        //已经入栈的运算符
    bool m_startNewOperand = false; //按完 "=" 后下一次输入数字要重新开始算式
    bool m_hasError = false;        //除数为 0、开方负数、结果溢出时进入错误状态

    QString calculation();                      //取出栈顶两个操作数做一次二元运算，结果重新压回栈
    static QString fromNumber(double value);      //结果转显示文本（15 位有效数字）
    static QString operatorToken(const QString &buttonText);  //× ÷ 1/X X^2 → * / 1/x x^2
    void showError(const QString &message);       //显示"错误"并进入错误状态
};
#endif // MAINWINDOW_H