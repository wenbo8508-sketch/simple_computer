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

}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::calculation(bool *ok)
{
    double result = 0;
    if (operands.size() == 2 && opcodes.size() > 0 ) {
        //取操作数
        double operand1 = operands.front().toDouble();
        operands.pop_front();
        double operand2 = operands.front().toDouble();
        operands.pop_front();

        //取操作符
        QString op = opcodes.front();
        opcodes.pop_front();

        if (op == "+") {
            result = operand1 + operand2;
        } else if (op == "-") {
            result = operand1 - operand2;
        } else if (op == "*") {
            result = operand1 * operand2;
        } else if (op == "/") {
            result = operand1 / operand2;
        }


        ui->statusbar->showMessage(QString("calcuation is in progress: operands is %1, opcode is %2")
                                       .arg(operands.size()).arg(opcodes.size()));
    } else {
        ui->statusbar->showMessage(QString("operands is %1, opcode is %2")
                                       .arg(operands.size()).arg(opcodes.size()));
    }
    return QString::number(result);
}

//数字键统一响应函数：把被按下按钮上的数字追加到当前操作数 operand 后面
void MainWindow::btnNum_clicked()
{
    //原实现读取的是 ui->display->text()，拼出来的其实是上一次的显示内容，
    //所以点击数字键没有效果；这里改为用 sender() 取出真正被点击的按钮文本
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    if (btn == nullptr)
        return;

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
    ui->statusbar->showMessage("last operand "+ operand);
    QString opcode = qobject_cast<QPushButton *>(sender())->text();
    if (operand != "") {
        operands.push_back(operand);
        operand = "";

        opcodes.push_back(opcode);
    }

    QString result=calculation();
    ui->display->setText(result);

}

void MainWindow::btnUnaryoperatorClicked()
{
    if (operand != "") {
        double result = operand.toDouble();
        operand = "";

        QString op = qobject_cast<QPushButton *>(sender())->text();

        if (op == "%")
            result /= 100.0;
        else if (op == "1/x")
            result = 1 / result;
        else if (op == "x^2")
            result *= result;
        else if (op == "√")   // ❗原图这里写成 ""，是错误，应该是平方根符号"√"
            result = sqrt(result);

        ui->display->setText(QString::number(result));
    }

}


void MainWindow::on_sum_clicked()
{

    if (operand != "") {
        operands.push_back(operand);
        operand = "";
    }

    QString result=calculation();
    ui->display->setText(result);
}

