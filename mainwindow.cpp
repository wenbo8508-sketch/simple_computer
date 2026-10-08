#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <math.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->num0, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    connect(ui->num1, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    connect(ui->num2, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    connect(ui->num3, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    connect(ui->num4, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    connect(ui->num5, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    connect(ui->num6, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    connect(ui->num7, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    connect(ui->num8, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    connect(ui->num9, SIGNAL(clicked()), this, SLOT(btnNumClicked()));

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

void MainWindow::btnNum_clicked()
{
    QString str = ui->display->text();
    if (str == "0" && operand == "0")
        str = "";

    if (operand == "0" && str != "0")
        operand = "";

    operand += str;
    ui->display->setText(operand);

}

void MainWindow::on_point_clicked()
{
    QString operand = ui->display->text();
    if (!operand.contains("."))
        operand += qobject_cast<QPushButton *>(sender())->text();
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

