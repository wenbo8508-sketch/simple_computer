#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <math.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->num0, SIGNAL(clicked()), this, SLOT(btnNum_clicked()));
    connect(ui->num1, SIGNAL(clicked()), this, SLOT(btnNum_clicked()));
    connect(ui->num2, SIGNAL(clicked()), this, SLOT(btnNum_clicked()));
    connect(ui->num3, SIGNAL(clicked()), this, SLOT(btnNum_clicked()));
    connect(ui->num4, SIGNAL(clicked()), this, SLOT(btnNum_clicked()));
    connect(ui->num5, SIGNAL(clicked()), this, SLOT(btnNum_clicked()));
    connect(ui->num6, SIGNAL(clicked()), this, SLOT(btnNum_clicked()));
    connect(ui->num7, SIGNAL(clicked()), this, SLOT(btnNum_clicked()));
    connect(ui->num8, SIGNAL(clicked()), this, SLOT(btnNum_clicked()));
    connect(ui->num9, SIGNAL(clicked()), this, SLOT(btnNum_clicked()));

    connect(ui->addition, SIGNAL(clicked()), this, SLOT(binaryOperatorClicked()));
    connect(ui->subtraction, SIGNAL(clicked()), this, SLOT(binaryOperatorClicked()));
    connect(ui->multiple, SIGNAL(clicked()), this, SLOT(binaryOperatorClicked()));
    connect(ui->divite, SIGNAL(clicked()), this, SLOT(binaryOperatorClicked()));
    connect(ui->fraction, SIGNAL(clicked()), this, SLOT(btnUnaryoperatorClicked()));
    connect(ui->square, SIGNAL(clicked()), this, SLOT(btnUnaryoperatorClicked()));
    connect(ui->root, SIGNAL(clicked()), this, SLOT(btnUnaryoperatorClicked()));
    connect(ui->zhengfu, SIGNAL(clicked()), this, SLOT(btnUnaryoperatorClicked()));
    connect(ui->present, SIGNAL(clicked()), this, SLOT(btnUnaryoperatorClicked()));
    //CE / C / ⌫ / . / = 几个按钮的对象名与 on_xxx_clicked() 槽名对应，
    //由 setupUi() 里的 QMetaObject::connectSlotsByName() 自动连接，其中 CE 原来没有槽函数，现已补上

}

MainWindow::~MainWindow()
{
    delete ui;
}

//二元运算：从操作数栈里取出栈顶的两个数，配合最近按下的运算符求值。
//关键改动：算完的结果必须重新压回 operands，否则像 "5+3-2=" 这样的连续运算，
//第二次求值时栈里已经没有操作数可用，只能返回 0（原程序最终显示 0 就是这个原因）。
QString MainWindow::calculation(bool *ok)
{
    if (ok)
        *ok = true;

    //操作数不足两个时不做二元运算，直接把当前值原样返回。
    //原实现在这里返回 QString::number(0)，于是 "5+" 之后显示被覆盖成 "0"，
    //"5=" 这种没有运算符的输入也会变成 0。
    if (operands.size() < 2 || opcodes.isEmpty()) {
        ui->statusbar->showMessage(QString("operands is %1, opcode is %2")
                                       .arg(operands.size()).arg(opcodes.size()));
        if (!operand.isEmpty())
            return operand;
        return operands.isEmpty() ? QString("0") : operands.top();
    }

    double operand2 = operands.pop().toDouble();   //栈顶是后输入的操作数
    double operand1 = operands.pop().toDouble();
    QString op = opcodes.pop();                    //取最近按下的运算符

    double result = 0;
    if (op == "+") {
        result = operand1 + operand2;
    } else if (op == "-") {
        result = operand1 - operand2;
    } else if (op == "*") {
        result = operand1 * operand2;
    } else if (op == "/") {
        result = operand1 / operand2;
    }

    QString text = fromNumber(result);
    operands.push_back(text);                      //结果回栈，可以继续参与后面的运算

    ui->statusbar->showMessage(QString("calculation is in progress: operands is %1, opcode is %2")
                                   .arg(operands.size()).arg(opcodes.size()));
    return text;
}

//把 double 结果格式化成显示文本。QString::number() 默认只有 6 位有效数字，
//123456789 × 9 会被显示成 1.11111e+09，这里提高到 15 位，并把 "-0" 规范成 "0"。
QString MainWindow::fromNumber(double value)
{
    QString text = QString::number(value, 'g', 15);
    if (text == "-0")
        text = "0";
    return text;
}

//界面上的按钮文本和代码里比较的符号并不一致：× ÷ 1/X X^2 对 * / 1/x x^2，
//原来直接用按钮文本去比较，导致乘法、平方、倒数按了没反应，这里统一做一次翻译。
QString MainWindow::operatorToken(const QString &buttonText)
{
    if (buttonText == "×")
        return "*";
    if (buttonText == "÷")
        return "/";
    if (buttonText == "1/X")
        return "1/x";
    if (buttonText == "X^2")
        return "x^2";
    return buttonText;
}

//数字键统一响应函数：把被按下按钮上的数字追加到当前操作数 operand 后面
void MainWindow::btnNum_clicked()
{
    //原实现读取的是 ui->display->text()，拼出来的其实是上一次的显示内容，
    //所以点击数字键没有效果；这里改为用 sender() 取出真正被点击的按钮文本
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    if (btn == nullptr)
        return;

    if (m_startNewOperand) {            //按完 "=" 之后重新输入数字，开始一条新算式
        operands.clear();
        opcodes.clear();
        operand.clear();
        m_startNewOperand = false;
    }

    if (operand == "0")                 //去掉多余的前导 0，避免出现 "012"
        operand.clear();

    operand += btn->text();

    if (operand.length() > 15)          //限制输入长度，防止显示区被撑爆
        operand.chop(operand.length() - 15);

    ui->display->setText(operand);
}

void MainWindow::on_point_clicked()
{
    //原实现中的局部变量 QString operand 遮蔽了同名成员变量，
    //导致小数点只写进 display，成员 operand 一直是空的
    if (!operand.contains("."))
        operand += qobject_cast<QPushButton *>(sender())->text();

    if (operand == ".")          //一开始就按小数点时补成 "0."
        operand = "0.";

    ui->display->setText(operand);
}


void MainWindow::on_del_clicked()
{
    operand = operand.left(operand.length() - 1);
    ui->display->setText(operand);
}


void MainWindow::on_C_clicked()
{
    operand.clear();
    ui->display->setText(operand);
}

void MainWindow::binaryOperatorClicked()
{
    QString opcode = operatorToken(qobject_cast<QPushButton *>(sender())->text());
    ui->statusbar->showMessage("last operand " + operand);

    if (!operand.isEmpty()) {
        operands.push_back(operand);      //当前输入的数字作为左操作数入栈
        operand.clear();

        //如果栈里已经有一条待算的式子，先算出中间结果显示出来（连续运算）
        ui->display->setText(calculation());
        opcodes.push_back(opcode);
    } else if (!opcodes.isEmpty()) {
        //连着按运算符时只保留最后一个，例如 "5+*" 应该等价于 "5*"
        opcodes.pop();
        opcodes.push_back(opcode);
        if (!operands.isEmpty())
            ui->display->setText(operands.top());
    } else if (!operands.isEmpty()) {
        //按完 "=" 之后接着按运算符，用刚算出的结果继续往下算
        opcodes.push_back(opcode);
    }
}

//一元运算：±、%、1/X、X^2、√ 都只作用在当前显示的数字上
void MainWindow::btnUnaryoperatorClicked()
{
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    if (btn == nullptr)
        return;

    //正在输入时用 operand，刚按完 "=" 或运算符时用显示区的结果
    QString current = operand.isEmpty() ? ui->display->text() : operand;
    if (current.isEmpty())
        return;

    double result = current.toDouble();
    QString op = operatorToken(btn->text());

    if (op == "±") {                  //原来 ± 键虽然 connect 了，但 if 链里没有分支，按了没反应
        result = -result;
    } else if (op == "%") {
        result /= 100.0;
    } else if (op == "1/x") {
        result = 1 / result;
    } else if (op == "x^2") {
        result *= result;
    } else if (op == "√") {
        result = sqrt(result);
    }

    //运算结果继续作为当前操作数，这样 "√9 + 1 =" 还能接着算
    operand = fromNumber(result);
    m_startNewOperand = false;
    ui->display->setText(operand);
}

//CE（clear entry）：只清掉当前正在输入的这一项，
//之前已经入栈的操作数和运算符保持不变，这是它和 C 的区别
void MainWindow::on_CE_clicked()
{
    operand.clear();
    ui->display->setText(operand);
}


void MainWindow::on_sum_clicked()
{
    if (!operand.isEmpty()) {
        operands.push_back(operand);
        operand.clear();
    }

    //只有运算符没有右操作数时（"5+="），把左操作数当右操作数补上，
    //结果和 Windows 计算器一致：5+ = 得到 10
    if (operands.size() == opcodes.size() && !operands.isEmpty())
        operands.push_back(operands.top());

    ui->display->setText(calculation());
    m_startNewOperand = true;             //再按数字键时重新开始一条算式
}

