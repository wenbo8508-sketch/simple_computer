#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <cmath>
#include <QKeyEvent>
#include <QPushButton>
#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //------ 键盘事件的前提设置 ------
    //显示框改成只读：否则它会把键盘输入的字符直接吃掉，窗口收不到按键
    ui->display->setReadOnly(true);
    ui->display->setFocusPolicy(Qt::NoFocus);    //显示框不参与焦点竞争，键盘事件才能落到主窗口
    //QMainWindow 默认不接收键盘焦点，要显式打开
    setFocusPolicy(Qt::StrongFocus);
    //按钮一律不抢焦点，否则鼠标点过一次之后，那个按钮会截走后续的所有按键
    const QList<QPushButton *> buttons = findChildren<QPushButton *>();
    for (QPushButton *btn : buttons)
        btn->setFocusPolicy(Qt::NoFocus);
    setFocus();                                   //程序启动后焦点就在主窗口上，可以直接用键盘输入

    setWindowTitle(QStringLiteral("带键盘事件的计算器"));
    ui->menubar->hide();                          //模板自带的空菜单栏没有菜单项，隐藏掉

    //样式表美化界面：深色计算器屏幕 + 数字键/运算符键/功能键分色
    QFile qss(QStringLiteral(":/style/style.qss"));
    if (qss.open(QIODevice::ReadOnly)) {
        setStyleSheet(QString::fromUtf8(qss.readAll()));
        qss.close();
    }

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

//------ 键盘事件处理 ------
//把键盘按键翻译成一个按钮对象名，然后调用该按钮的 click()，
//这样键盘和鼠标走的是完全相同的槽函数路径（sender() 仍然指向那个按钮），
//不需要把计算逻辑重写一遍，也不会出现两种输入方式结果不一致的问题。
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    //带 Ctrl / Alt 的组合键（Ctrl+C、Alt+F4 等）交回默认处理
    if (event->modifiers().testFlag(Qt::ControlModifier)
        || event->modifiers().testFlag(Qt::AltModifier)) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    QString buttonName;
    const QString text = event->text();     //已经考虑了 Shift 等修饰键的字符

    if (!text.isEmpty()) {
        const QChar ch = text.at(0).unicode();
        if (ch.isDigit())
            buttonName = QStringLiteral("num%1").arg(ch.digitValue());
        else if (ch == QLatin1Char('.') || ch == QLatin1Char(','))
            buttonName = QStringLiteral("point");
        else if (ch == QLatin1Char('+'))
            buttonName = QStringLiteral("addition");
        else if (ch == QLatin1Char('-'))
            buttonName = QStringLiteral("subtraction");
        else if (ch == QLatin1Char('*') || ch == QLatin1Char('x') || ch == QLatin1Char('X')
                 || ch == QChar(0x00D7))            //×
            buttonName = QStringLiteral("multiple");
        else if (ch == QLatin1Char('/') || ch == QChar(0x00F7))   //÷
            buttonName = QStringLiteral("divite");
        else if (ch == QLatin1Char('='))
            buttonName = QStringLiteral("sum");
        else if (ch == QLatin1Char('%'))
            buttonName = QStringLiteral("present");
        else if (ch == QChar(0x00B1))               //±
            buttonName = QStringLiteral("zhengfu");
        else if (ch == QLatin1Char('r') || ch == QLatin1Char('R'))
            buttonName = QStringLiteral("root");    //R 键开平方
        else if (ch == QLatin1Char('^'))
            buttonName = QStringLiteral("square");  //^ 键求平方
    }

    //功能键没有可打印字符，只能按 key 判断
    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        buttonName = QStringLiteral("sum");
        break;
    case Qt::Key_Backspace:
        buttonName = QStringLiteral("del");
        break;
    case Qt::Key_Delete:
        buttonName = QStringLiteral("CE");
        break;
    case Qt::Key_Escape:
        buttonName = QStringLiteral("C");
        break;
    default:
        break;
    }

    if (buttonName.isEmpty()) {                     //没映射到的按键不能把事件吞掉
        QMainWindow::keyPressEvent(event);
        return;
    }

    QPushButton *btn = findChild<QPushButton *>(buttonName);
    if (btn)
        btn->click();                               //复用按钮已有的处理逻辑
    else
        event->ignore();
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
        if (qFuzzyIsNull(operand2)) {       //除数为 0，原来会显示 inf
            showError(QStringLiteral("除数不能为 0"));
            return QStringLiteral("错误");
        }
        result = operand1 / operand2;
    }

    if (!std::isfinite(result)) {           //结果溢出或变成 NaN
        showError(QStringLiteral("数值溢出"));
        return QStringLiteral("错误");
    }

    QString text = fromNumber(result);
    operands.push_back(text);                      //结果回栈，可以继续参与后面的运算

    ui->statusbar->showMessage(QString("calculation is in progress: operands is %1, opcode is %2")
                                   .arg(operands.size()).arg(opcodes.size()));
    return text;
}

//统一处理出错：显示区写"错误"，具体原因放到状态栏，并进入错误状态。
//在用户按 C / CE / 数字 / 小数点之前，其它按键都不起作用，避免在错误结果上继续算。
void MainWindow::showError(const QString &message)
{
    m_hasError = true;
    m_startNewOperand = true;
    ui->display->setText(QStringLiteral("错误"));
    ui->statusbar->showMessage(message);
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

    m_hasError = false;                     //按数字就能恢复，可以重新输入

    if (m_startNewOperand) {                //按完 "=" 之后重新输入数字，开始一条新算式
        operands.clear();
        opcodes.clear();
        operand.clear();
        m_startNewOperand = false;
    }

    if (operand == "0")                     //去掉多余的前导 0，避免出现 "012"
        operand.clear();

    operand += btn->text();

    if (operand.length() > 15)              //限制输入长度，防止显示区被撑爆
        operand.chop(operand.length() - 15);

    ui->display->setText(operand);
}

void MainWindow::on_point_clicked()
{
    m_hasError = false;

    if (m_startNewOperand) {                //按完 "=" 之后重新输入，开始一条新算式
        operands.clear();
        opcodes.clear();
        operand.clear();
        m_startNewOperand = false;
    }

    if (operand.isEmpty())                  //直接按小数点时补出 "0."
        operand = QStringLiteral("0");

    //原实现里的局部变量 QString operand 遮蔽了同名成员变量，
    //导致小数点只写进 display，成员 operand 一直是空的
    if (!operand.contains("."))
        operand += qobject_cast<QPushButton *>(sender())->text();

    ui->display->setText(operand);
}


void MainWindow::on_del_clicked()
{
    if (m_hasError) {                       //出错后按退格，直接回到可输入状态
        on_C_clicked();
        return;
    }

    //退格只删除当前正在输入的这一项，不影响已经入栈的操作数和运算符
    operand = operand.left(operand.length() - 1);
    m_startNewOperand = false;

    //原来可以一直删成空字符串，显示区一片空白；这里删空后回落到 "0"
    ui->display->setText(operand.isEmpty() ? QStringLiteral("0") : operand);
}


//C（clear all）：整条算式全部清空
void MainWindow::on_C_clicked()
{
    operand.clear();
    operands.clear();                       //原来只清了 operand，栈里的操作数和运算符还留着，
    opcodes.clear();                        //于是 "1+2 C 3=" 会算出 4，现在一起清掉
    m_startNewOperand = false;
    m_hasError = false;
    ui->display->setText(operand);
    ui->statusbar->showMessage("cleared");
}

void MainWindow::binaryOperatorClicked()
{
    if (m_hasError)                         //错误状态下先按运算符没有意义，直接忽略
        return;

    opcode = operatorToken(qobject_cast<QPushButton *>(sender())->text());   //记录本次按下的运算符
    ui->statusbar->showMessage("last operand " + operand);

    if (!operand.isEmpty()) {
        operands.push_back(operand);        //当前输入的数字作为左操作数入栈
        operand.clear();

        //如果栈里已经有一条待算的式子，先算出中间结果显示出来（连续运算）
        ui->display->setText(calculation());
        if (m_hasError)
            return;

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

    //按完 "=" 之后 m_startNewOperand 仍然是 true，这里必须把它清掉：
    //否则「结果 → 按运算符 → 输入数字」时，数字键会误以为在开一条新算式，
    //把刚算出来的结果连同运算符一起清空（5 + 3 = + 2 = 只显示 2 就是这个原因）
    m_startNewOperand = false;
}

//一元运算：±、%、1/X、X^2、√ 都只作用在当前显示的数字上
void MainWindow::btnUnaryoperatorClicked()
{
    if (m_hasError)
        return;

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
        if (qFuzzyIsNull(result)) {   //1/0
            showError(QStringLiteral("除数不能为 0"));
            return;
        }
        result = 1 / result;
    } else if (op == "x^2") {
        result *= result;
    } else if (op == "√") {
        if (result < 0) {             //负数不能开平方
            showError(QStringLiteral("负数不能开平方"));
            return;
        }
        result = sqrt(result);
    }

    if (!std::isfinite(result)) {
        showError(QStringLiteral("数值溢出"));
        return;
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
    if (m_hasError)
        return;

    if (!operand.isEmpty()) {
        operands.push_back(operand);
        operand.clear();
    }

    //只有运算符没有右操作数时（"5+="），把左操作数当右操作数补上，
    //结果和 Windows 计算器一致：5+ = 得到 10
    if (operands.size() == opcodes.size() && !operands.isEmpty())
        operands.push_back(operands.top());

    QString result = calculation();
    if (!m_hasError)
        ui->display->setText(result);
    m_startNewOperand = true;               //再按数字键时重新开始一条算式
}