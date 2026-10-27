// tools/shot.cpp —— 界面截图辅助工具（不参与程序发布）
//
// 用 Qt 的 offscreen 平台把若干典型输入状态渲染成 PNG，供 README 与实验报告引用，
// 好处是截图可以随代码一键重新生成，不会出现「代码改了、截图还是旧的」。
//
// 用法：
//   cmake -S . -B build -DBUILD_SHOTS=ON
//   cmake --build build --target shots
//   build\shots.exe docs\screenshots        // 不带参数时默认输出到 docs/screenshots
#include <QApplication>
#include <QDir>
#include <QPixmap>
#include <QPushButton>
#include <QStringList>

#include "mainwindow.h"

namespace {

struct Scene {
    const char *file;         // 输出文件名（不含扩展名）
    const char *caption;      // 截图说明，同时打印到控制台
    QStringList keys;         // 依次点击的按钮对象名
};

void clickAll(MainWindow &window, const QStringList &names)
{
    for (const QString &name : names) {
        QPushButton *btn = window.findChild<QPushButton *>(name);
        if (btn == nullptr) {
            qWarning("找不到按钮：%s", qPrintable(name));
            continue;
        }
        btn->click();                     //与鼠标点击走完全相同的路径
    }
    QCoreApplication::processEvents();
}

} // namespace

int main(int argc, char *argv[])
{
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }     //没有显示器/远程终端时也能出图

    QApplication app(argc, argv);

    const QString dir = (argc > 1) ? QString::fromLocal8Bit(argv[1])
                                   : QStringLiteral("docs/screenshots");
    QDir().mkpath(dir);

    const Scene scenes[] = {
        {"01-idle", "浅色界面，显示区白底右对齐", {}},
        {"02-decimal", "小数输入 3.5", {"num3", "point", "num5"}},
        {"03-chain-reuse", "5 + 3 = 之后再 + 2 = 得 10（结果可续算）",
         {"num5", "addition", "num3", "sum", "addition", "num2", "sum"}},
        {"04-divzero", "除数为 0：显示区「错误」，原因在状态栏",
         {"num5", "divite", "num0", "sum"}},
        {"05-unary-sqrt", "键盘 r 等价的开平方：sqrt(9) = 3", {"num9", "root"}},
        {"06-backspace", "退格只删当前输入这一项", {"num1", "num2", "num3", "del"}},
        {"07-percent", "百分号与系统计算器同语义：100 + 10 % = 110",
         {"num1", "num0", "num0", "addition", "num1", "num0", "present", "sum"}},
        {"08-consecutive-op", "连按运算符只保留最后一个：5 + * 3 = 15",
         {"num5", "addition", "multiple", "num3", "sum"}},
    };

    for (const Scene &scene : scenes) {
        MainWindow window;                      //每个场景都用全新实例，避免状态串味
        window.resize(360, 520);
        window.show();
        QCoreApplication::processEvents();

        clickAll(window, scene.keys);

        const QString file = dir + '/' + scene.file + ".png";
        if (!window.grab().save(file, "PNG"))
            qWarning("截图失败：%s", qPrintable(file));
        else
            qInfo("%-24s %s", scene.file, scene.caption);
    }

    qInfo("共输出到 %s", qPrintable(QDir(dir).absolutePath()));
    return 0;
}
