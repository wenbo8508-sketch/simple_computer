# 带键盘事件的计算器（simple_computer）

《Qt应用程序开发》实验一：用 Qt 6 Widgets + CMake 实现的桌面计算器。
鼠标点击与键盘输入**共用同一套槽函数**，并用 QtTest 写了 41 个自动化用例，
覆盖四则运算、小数输入、连续运算、一元运算、退格/清除、除零等异常输入、
输入长度边界和键盘事件。

## 1. 开发环境

| 项目 | 说明 |
| --- | --- |
| Qt | 6.11.2 MinGW 64-bit（实验要求 6.9.2，本机为 6.11.2，向下兼容） |
| 编译器 | MinGW 13.1，C++17 |
| 构建 | CMake ≥ 3.16 + Ninja |
| IDE | Qt Creator |
| 版本管理 | Git（提交历史见第 8 节） |

## 2. 目录结构

```text
simple_computer/
├── CMakeLists.txt            工程配置（含 BUILD_TESTS 选项、CTest 注册）
├── main.cpp                  程序入口：QApplication + a.exec()
├── mainwindow.h              槽函数声明 + 私有状态成员
├── mainwindow.cpp            计算逻辑、键盘事件、异常处理
├── mainwindow.ui             UI 设计器生成的界面（QVBoxLayout + QGridLayout）
├── style.qss                 样式表
├── resources.qrc             资源文件，把 style.qss 打包成 :/style/style.qss
├── tests/tst_mainwindow.cpp  QtTest 自动化用例（41 个）
└── docs/调试与修复记录.md     缺陷清单：现象 → 原因 → 修改 → 验证
```

## 3. 界面设计

- 主窗口 `MainWindow`（`QMainWindow`），窗口标题「带键盘事件的计算器」。
- 中央控件 `centralwidget` → `verticalLayout`（QVBoxLayout）→ `gridLayout_2`（QGridLayout），
  按钮全部按行列放进网格，窗口缩放时按钮自动等比拉伸，不需要写死坐标。
- 显示区 `display`（`QLineEdit`）：只读、右对齐、`placeholderText` 为 `0`，
  放在网格第 0 行并跨 4 列（`colspan="4"`），所以显示区宽度始终和按键区一致。
- 提示 `hintLabel`（`QLabel`）：说明可用的键盘按键。
- 状态栏 `statusbar` 用中文打印操作数栈/运算符栈的实时状态，出错时也在这里给出原因，方便调试和课堂演示。
- 模板自带的空 `menubar` 没有菜单项，构造函数里 `hide()` 隐藏。

| 类别 | 控件对象名 | 按钮文本 |
| --- | --- | --- |
| 数字 | `num0` … `num9` | 0 ~ 9 |
| 输入 | `point` / `sum` | . / = |
| 二元运算符 | `addition` / `subtraction` / `multiple` / `divite` | + − × ÷ |
| 一元运算 | `present` / `fraction` / `square` / `root` / `zhengfu` | % 1/X X^2 √ ± |

> 百分号 `present` 与 Windows 计算器同语义：紧跟在 `+` / `-` 后面时以左操作数为基准
> （`100 + 10 % = 110`），其余情况只做除以 100（`5 % = 0.05`、`100 × 10 % = 10`）。
| 清除 | `C` / `CE` / `del` | 全部清除 / 清除当前输入 / 退格 |

## 4. 键盘事件与按键对照

`MainWindow::keyPressEvent()` 只做一件事：把按键翻译成按钮的**对象名**，
再 `findChild<QPushButton *>(name)->click()`。这样键盘触发的是和鼠标完全相同的槽函数，
`sender()` 仍然指向那个按钮，计算逻辑一行都不用重复写，两种输入方式不可能出现结果不一致。

| 键盘按键 | 等价按钮 | 键盘按键 | 等价按钮 |
| --- | --- | --- | --- |
| `0` ~ `9` | num0…num9 | `=` / `Enter` / `Return` | sum |
| `.` 或 `,` | point | `Backspace` | del（退格） |
| `+` `-` `*` `/` | addition / subtraction / multiple / divite | `Delete` | CE |
| `x` `X` `×` | multiple（乘） | `Esc` | C（全清） |
| `÷` | divite | `%` | present |
| `^` | square（平方） | `r` `R` | root（开平方） |
| Ctrl / Alt 组合键、未映射按键 | 交回基类默认处理，不吞事件 | | |

为了让按键能落到主窗口，构造函数里做了三件事：
`display` 设为只读 + `Qt::NoFocus`（否则 QLineEdit 会把字符吃掉）、
所有 `QPushButton` 设为 `Qt::NoFocus`（否则点过的按钮会截走后续按键）、
主窗口 `setFocusPolicy(Qt::StrongFocus)` + `setFocus()`。

## 5. 计算状态管理

程序用「两个栈 + 当前输入」管理状态，全部是 `private` 成员，外部只能靠按键驱动：

| 成员 | 含义 |
| --- | --- |
| `operand` | 当前正在输入、还没入栈的操作数 |
| `opcode` | 最近一次按下的运算符 |
| `operands` | 已入栈的操作数（`QStack<QString>`） |
| `opcodes` | 已入栈的运算符 |
| `m_startNewOperand` | 按完 `=` 之后，下一次输入数字要开一条新算式 |
| `m_hasError` | 错误状态：显示区为「错误」，只有 C / CE / 退格 / 数字 / 小数点能恢复 |

状态转移过程：

1. **第一个操作数**：数字键把字符追加到 `operand` 并实时刷新 `display`；
2. **按运算符**：`operand` 入栈 `operands` 后清空，运算符入栈 `opcodes`；
   如果此时栈里已经凑齐「数 运算符 数」，先调用 `calculation()` 算出中间结果显示出来（连续运算）；
3. **第二个操作数**：同样先进 `operand`；
4. **按 `=`**：把 `operand` 入栈后调用 `calculation()`，
   **结果重新压回 `operands`** 并置 `m_startNewOperand = true`，所以结果还能继续参与下一次运算；
5. **异常**：除数为 0、负数开平方、结果 `inf/nan` → `showError()`，
   显示区写「错误」，具体原因放状态栏，并屏蔽后续运算符按键；
   按 C / CE / 退格 / 数字 / 小数点任一按键即可恢复。

`calculation()` 只在栈里确实有「两个操作数 + 一个运算符」时才求值；
数量不足时返回当前值而不是 0，所以 `5+` `5=` 这类输入不会把显示刷成 0。

## 6. 构建与运行

### 方式一：Qt Creator

打开 `simple_computer/CMakeLists.txt`，选择 **Qt 6.11.2 MinGW 64-bit** 套件，
直接 构建 → 运行（默认已开启 `BUILD_TESTS`）。

### 方式二：命令行

```powershell
cd D:\QTwork\simple_computer
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH="C:/Qt/6.11.2/mingw_64" -DCMAKE_BUILD_TYPE=Release
cmake --build build
.\build\simple_computor.exe
```

### 自动化测试

```powershell
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH="C:/Qt/6.11.2/mingw_64" -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
# 想看每个用例的名字：$env:QT_QPA_PLATFORM="offscreen"; .\build\tst_simple_computor.exe
```

最近一次结果：

```text
Totals: 43 passed, 0 failed, 0 skipped, 0 blacklisted
```

用例分四类：输入与显示（数字拼接、前导 0、小数点去重、退格、CE/C）、
运算与状态（四则、连续运算、一元运算、百分号基准、`=` 后继续算、`5+ =`、连按运算符）、
异常与键盘（除 0、负数开平方、错误态屏蔽按键与恢复、8 组键盘用例验证键盘与鼠标结果一致）、
边界与回归（20 位输入截断、重复按 `=`、`=` 后退格再输入、一元运算接二元运算、
`=` 后直接按小数点、状态栏提示文案）。

## 7. 样式表美化

`style.qss` 通过 `resources.qrc`（prefix `/style`）打包成 `:/style/style.qss`，
构造函数里用 `QFile` 读出后 `setStyleSheet()` 应用。配色参照 **Windows 自带计算器的浅色模式**，整体以白色为主：

- 窗口底色浅灰 `#f3f3f3`，文字近黑 `#1a1a1a`，字体 Microsoft YaHei UI / Segoe UI；
- 显示区白底深字、右对齐、30px 半粗，只用一根浅描边与按键区分隔，不再是黑底屏幕；
- 数字键纯白、运算符键极浅蓝灰 `#f0f4fa`、功能键浅灰 `#f6f6f6`，用三档亮度区分功能，不刺眼；
- `=` 用系统强调蓝 `#0067c0` + 白字，是界面上唯一的深色块；`C` 用淡红底 + 红字提示危险操作；
- 所有按键圆角 6px + 1px 描边，`:hover` 提亮、`:pressed` 变灰，点击反馈明显；
- 状态栏白底 + 顶部细线 + 中灰文字，`hintLabel` 用灰色小字，不抢视觉重点。

## 8. Git 提交历史（GitHub 迭代过程）

仓库：**https://github.com/wenbo8508-sketch/simple_computer** （分支 `master`，共 21 次提交）
提交按「一次提交只做一件事」拆分，可清晰看出 功能 → 测试 → 修复 → 重构 → 文档 的迭代过程：

| # | 提交 | 目的 |
| --- | --- | --- |
| 1 | `f7b1ef7` | 初始提交：导入原始 simple_computer 工程 |
| 2 | `6035bda` | 修复数字键与小数点无法正确输入的问题 |
| 3 | `0f52e34` | 修复运算符匹配与连续运算错误 |
| 4 | `2f74c31` | 补全 CE、±、百分号、倒数、平方、开方按键功能 |
| 5 | `27452ea` | 补齐异常输入处理：除数为 0、开方负数、溢出、退格与全清 |
| 6 | `af73ee8` | 增加 QtTest 自动化用例并接入 CMake/CTest |
| 7 | `306a9e0` | 增加键盘事件处理，键盘与鼠标复用同一套槽函数 |
| 8 | `eaee68e` | 使用样式表美化界面，并整理窗口与成员访问权限 |
| 9 | `17e6aec` | 修复算完结果后按运算符续算被清空的问题 |
| 10 | `55fde01` | 重构：精简 `calculation()` 接口并整理注释位置 |
| 11 | `79202ea` | 样式：界面改为以白色为主的浅色主题（仿系统计算器） |
| 12 | `63fc420` | 文档：新增 README 与调试修复记录 |
| 13 | `a3938da` | 界面：窗口标题改为个人标识并精简键盘提示文字 |
| 14 | `e0d0f8c` | 测试：同步窗口标题断言，33 项用例全部通过 |
| 15 | `de407e8` | 构建：忽略命令行构建目录 `build-cli/`，工作区保持干净 |
| 16 | `13a1156` | **功能**：百分号与系统计算器同语义（`100+10%=` 得 110），补 2 条用例 |
| 17 | `4bc2708` | **修复**：错误状态下按 CE 不能恢复，与注释、退格行为自相矛盾，补回归用例 |
| 18 | `ec74612` | **健壮性**：未知运算符安全兜底、`sender()` 判空、状态栏提示统一中文 |
| 19 | `3b76763` | **测试**：补 6 组边界与回归用例（43 项断言全绿），把验收结论固化成断言 |
| 20 | `4705939` | **文档/工具**：新增实验要求验收对照、界面截图工具 `tools/shot.cpp` 与 8 张截图，修正 README 中的中文路径示例 |
| 21 | 本次提交 | **文档**：使 README 与远端仓库一致（默认分支为 `master`、补齐第 20 次提交哈希、推送命令按实际操作写法） |

本地推送到 GitHub：

```powershell
git remote add origin https://github.com/wenbo8508-sketch/simple_computer.git
git push -u origin master   # 远端默认分支即首次推送的 master
```

## 9. 已知取舍

- 运算按**输入顺序从左到右**求值（和 Windows 计算器标准型一致），不做先乘除后加减；
- 显示最多 15 位有效数字，超长输入截断，避免撑爆显示区；
- 按 `=` 不记忆上一条算式：重复按 `=` 结果保持不变（Windows 计算器会重复上次求值）；
- 百分号与系统计算器同语义（加减法取左操作数的百分比，乘除法只除以 100）；
- `C` 之后显示区是空字符串，靠 `placeholderText` 显示 `0`，与 Windows 计算器行为一致。