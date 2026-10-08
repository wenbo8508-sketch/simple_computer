#include "mainwindow.h"

#include <QtTest>
#include <QLineEdit>
#include <QPushButton>
#include <QStatusBar>

//用 QtTest 模拟鼠标点击界面按钮，检查计算器的状态转移是否符合预期
class TestCalculator : public QObject
{
    Q_OBJECT

private:
    MainWindow *window = nullptr;

    QString display() const
    {
        QLineEdit *disp = window->findChild<QLineEdit *>("display");
        return disp ? disp->text() : QString();
    }

    QString status() const
    {
        QStatusBar *bar = window->findChild<QStatusBar *>("statusbar");
        return bar ? bar->currentMessage() : QString();
    }

    //按对象名依次点击按钮，等价于用鼠标逐个按键
    void press(const QStringList &names)
    {
        for (const QString &name : names) {
            QPushButton *btn = window->findChild<QPushButton *>(name);
            QVERIFY2(btn != nullptr, qPrintable(QStringLiteral("找不到按钮 %1").arg(name)));
            QTest::mouseClick(btn, Qt::LeftButton);
        }
    }

private slots:
    void init()
    {
        window = new MainWindow;
        window->show();
    }

    void cleanup()
    {
        delete window;
        window = nullptr;
    }

    void digits_append();
    void leading_zero_replaced();
    void decimal_point();
    void add_two_numbers();
    void subtract_gives_negative();
    void multiply_uses_times_symbol();
    void divide();
    void chained_operations();
    void floating_point_result_format();
    void unary_root_and_square();
    void unary_reciprocal_percent_and_sign();
    void unary_after_equals();
    void clear_entry_keeps_expression();
    void clear_all_resets_everything();
    void backspace_edits_current_input();
    void consecutive_operators_keep_last();
    void equals_without_right_operand();
    void operator_after_equals_reuses_result();
    void typing_after_equals_starts_new_expression();
    void divide_by_zero_shows_error();
    void error_state_blocks_operators();
    void sqrt_of_negative_shows_error();
};

void TestCalculator::digits_append()
{
    press({"num1", "num2", "num3"});
    QCOMPARE(display(), QStringLiteral("123"));
}

void TestCalculator::leading_zero_replaced()
{
    press({"num0", "num5"});
    QCOMPARE(display(), QStringLiteral("5"));
}

void TestCalculator::decimal_point()
{
    press({"num3", "point", "num5"});
    QCOMPARE(display(), QStringLiteral("3.5"));

    press({"point"});                                  //小数点不重复输入
    QCOMPARE(display(), QStringLiteral("3.5"));

    press({"C", "point", "num5"});
    QCOMPARE(display(), QStringLiteral("0.5"));
}

void TestCalculator::add_two_numbers()
{
    press({"num5", "addition", "num3", "sum"});
    QCOMPARE(display(), QStringLiteral("8"));
}

void TestCalculator::subtract_gives_negative()
{
    press({"num3", "subtraction", "num5", "sum"});
    QCOMPARE(display(), QStringLiteral("-2"));
}

void TestCalculator::multiply_uses_times_symbol()
{
    //按钮文本是 ×，代码里要能识别成乘法
    press({"num6", "multiple", "num7", "sum"});
    QCOMPARE(display(), QStringLiteral("42"));
}

void TestCalculator::divide()
{
    press({"num9", "divite", "num3", "sum"});
    QCOMPARE(display(), QStringLiteral("3"));
}

void TestCalculator::chained_operations()
{
    //连续运算：5 + 3 - 2 = 6
    press({"num5", "addition", "num3", "subtraction", "num2", "sum"});
    QCOMPARE(display(), QStringLiteral("6"));
}

void TestCalculator::floating_point_result_format()
{
    press({"point", "num1", "addition", "point", "num2", "sum"});
    QCOMPARE(display(), QStringLiteral("0.3"));         //不能出现 0.30000000000000004

    //计算器按输入顺序从左到右运算（和 Windows 标准型一致，不做先乘除后加减）
    press({"C", "num9", "addition", "num9", "multiple", "num9", "sum"});
    QCOMPARE(display(), QStringLiteral("162"));
}

void TestCalculator::unary_root_and_square()
{
    press({"num9", "root"});
    QCOMPARE(display(), QStringLiteral("3"));

    press({"C", "num2", "square"});                     //按钮文本是 X^2
    QCOMPARE(display(), QStringLiteral("4"));
}

void TestCalculator::unary_reciprocal_percent_and_sign()
{
    press({"num4", "fraction"});                        //按钮文本是 1/X
    QCOMPARE(display(), QStringLiteral("0.25"));

    press({"C", "num5", "present"});
    QCOMPARE(display(), QStringLiteral("0.05"));

    press({"C", "num5", "zhengfu"});
    QCOMPARE(display(), QStringLiteral("-5"));

    press({"zhengfu"});
    QCOMPARE(display(), QStringLiteral("5"));
}

void TestCalculator::unary_after_equals()
{
    //先算出结果，再对结果做一元运算
    press({"num3", "multiple", "num3", "sum", "root"});
    QCOMPARE(display(), QStringLiteral("3"));
}

void TestCalculator::clear_entry_keeps_expression()
{
    //CE 只清当前输入：1 + 2 CE 3 = 4
    press({"num1", "addition", "num2", "CE", "num3", "sum"});
    QCOMPARE(display(), QStringLiteral("4"));
}

void TestCalculator::clear_all_resets_everything()
{
    press({"num1", "addition", "num2", "C", "num3", "sum"});
    QCOMPARE(display(), QStringLiteral("3"));
}

void TestCalculator::backspace_edits_current_input()
{
    press({"num1", "num2", "num3", "del"});
    QCOMPARE(display(), QStringLiteral("12"));

    press({"del", "del"});
    QCOMPARE(display(), QStringLiteral("0"));           //退到空时回落到 0，不会出现空白
}

void TestCalculator::consecutive_operators_keep_last()
{
    //5 + * 3 = 15，只保留最后按下的运算符
    press({"num5", "addition", "multiple", "num3", "sum"});
    QCOMPARE(display(), QStringLiteral("15"));
}

void TestCalculator::equals_without_right_operand()
{
    press({"num5", "addition", "sum"});
    QCOMPARE(display(), QStringLiteral("10"));
}

void TestCalculator::operator_after_equals_reuses_result()
{
    //5 + 3 = 8，再按 + = 得到 16
    press({"num5", "addition", "num3", "sum", "addition", "sum"});
    QCOMPARE(display(), QStringLiteral("16"));
}

void TestCalculator::typing_after_equals_starts_new_expression()
{
    press({"num5", "addition", "num3", "sum", "num7", "addition", "num1", "sum"});
    QCOMPARE(display(), QStringLiteral("8"));
}

void TestCalculator::divide_by_zero_shows_error()
{
    press({"num5", "divite", "num0", "sum"});
    QCOMPARE(display(), QStringLiteral("错误"));
    QVERIFY(status().contains(QStringLiteral("除数不能为 0")));

    //按数字键可以恢复，重新从 3 开始输入
    press({"num3"});
    QCOMPARE(display(), QStringLiteral("3"));
}

void TestCalculator::error_state_blocks_operators()
{
    press({"num5", "divite", "num0", "sum", "addition", "sum"});
    QCOMPARE(display(), QStringLiteral("错误"));          //出错后继续按运算符不应该算出东西

    press({"C"});
    QCOMPARE(display(), QString());                       //C 之后回到空白（placeholderText 显示 0）
    press({"num2", "addition", "num2", "sum"});
    QCOMPARE(display(), QStringLiteral("4"));             //清屏后可以正常使用
}

void TestCalculator::sqrt_of_negative_shows_error()
{
    press({"num4", "zhengfu", "root"});
    QCOMPARE(display(), QStringLiteral("错误"));
    QVERIFY(status().contains(QStringLiteral("负数不能开平方")));

    press({"CE"});
    press({"num4", "root"});
    QCOMPARE(display(), QStringLiteral("2"));             //CE 之后还能接着算 √4 = 2
}

QTEST_MAIN(TestCalculator)
#include "tst_mainwindow.moc"