# 蓝牙HID键盘扩展功能说明

## 功能概述
本项目在原有蓝牙HID鼠标和多媒体控制功能的基础上，新增了键盘输入功能，支持：
- 单个按键输入
- 组合键输入（如Ctrl+A）
- 多按键同时按下
- 完整的键盘扫描码支持

## 主要修改

### 1. HID报告描述符
- 新增Report ID 3用于键盘输入
- 支持8位修饰键（Ctrl, Shift, Alt, GUI）
- 支持同时6个按键输入

### 2. 新增文件
- `inc/app_hid_keyboard.h` - 键盘HID扫描码定义和数据结构
- `src/app_hid_keyboard.c` - 键盘报告构建函数实现

### 3. 核心功能函数
- `app_hid_send_keyboard_report()` - 发送键盘HID报告
- `build_keyboard_report()` - 构建键盘报告数据

### 4. 按键功能映射

| 按键 | 功能 | 说明 |
|------|------|------|
| KEY1 (PB11) | 发送字母'A' | 模拟单个按键按下和释放 |
| KEY2 (PB12) | 发送"Hello" | 演示连续按键输入，包含大写字母（Shift+H） |
| KEY3 (PB13) | 发送Ctrl+A | 演示组合键功能（全选） |

## 使用方法

### 基本按键输入
```c
hid_keyboard_report_t kb_report;
uint8_t keys[] = {HID_KEY_A}; // 要按下的键
build_keyboard_report(&kb_report, 0, keys, 1);
app_hid_send_keyboard_report((uint8_t*)&kb_report);

// 释放按键
build_keyboard_report(&kb_report, 0, NULL, 0);
app_hid_send_keyboard_report((uint8_t*)&kb_report);
```

### 组合键输入
```c
hid_keyboard_report_t kb_report;
uint8_t keys[] = {HID_KEY_C}; // 按下C键
// 同时按下Ctrl
build_keyboard_report(&kb_report, HID_KEYBOARD_LEFT_CTRL, keys, 1);
app_hid_send_keyboard_report((uint8_t*)&kb_report);

// 释放所有键
build_keyboard_report(&kb_report, 0, NULL, 0);
app_hid_send_keyboard_report((uint8_t*)&kb_report);
```

### 多键同时输入
```c
hid_keyboard_report_t kb_report;
uint8_t keys[] = {HID_KEY_A, HID_KEY_B, HID_KEY_C}; // 同时按下A、B、C
build_keyboard_report(&kb_report, 0, keys, 3);
app_hid_send_keyboard_report((uint8_t*)&kb_report);

// 释放所有键
build_keyboard_report(&kb_report, 0, NULL, 0);
app_hid_send_keyboard_report((uint8_t*)&kb_report);
```

## 键盘扫描码参考

### 字母键
- HID_KEY_A (0x04) 到 HID_KEY_Z (0x1D)

### 数字键
- HID_KEY_1 (0x1E) 到 HID_KEY_0 (0x27)

### 修饰键
- HID_KEYBOARD_LEFT_CTRL (0x01)
- HID_KEYBOARD_LEFT_SHIFT (0x02)
- HID_KEYBOARD_LEFT_ALT (0x04)
- HID_KEYBOARD_LEFT_GUI (0x08)
- HID_KEYBOARD_RIGHT_CTRL (0x10)
- HID_KEYBOARD_RIGHT_SHIFT (0x20)
- HID_KEYBOARD_RIGHT_ALT (0x40)
- HID_KEYBOARD_RIGHT_GUI (0x80)

### 特殊键
- HID_KEY_ENTER (0x28)
- HID_KEY_ESC (0x29)
- HID_KEY_BACKSPACE (0x2A)
- HID_KEY_TAB (0x2B)
- HID_KEY_SPACE (0x2C)

### 功能键
- HID_KEY_F1 (0x3A) 到 HID_KEY_F12 (0x45)

## 注意事项

1. **按键释放**：每次按键后必须发送释放报告（全0报告），否则接收设备会认为按键一直被按下。

2. **报告发送间隔**：连续发送多个报告时，建议适当延时，确保接收设备能正确处理。

3. **最大按键数**：HID规范限制同时最多6个非修饰键，这是为了兼容性考虑。

4. **蓝牙连接**：确保设备已配对并连接后才能发送键盘报告。

5. **报告缓冲**：系统限制了可发送的报告数量（APP_HID_NB_SEND_REPORT），避免发送过快导致丢失。

## 扩展建议

1. **添加更多按键映射**：可以根据需要添加更多GPIO按键，映射到不同的键盘功能。

2. **宏功能**：可以实现按键宏，一键执行复杂的按键序列。

3. **键盘布局**：可以实现不同的键盘布局切换（如QWERTY、DVORAK等）。

4. **LED指示**：可以添加Caps Lock、Num Lock等状态LED指示。

## 编译和烧录

1. 使用MDK-ARM工程文件进行编译
2. 通过SWD接口烧录到N32WB031开发板
3. 配对蓝牙设备后即可使用键盘功能

## 测试步骤

1. 编译并烧录程序到开发板
2. 在电脑/手机上搜索并配对蓝牙设备
3. 打开文本编辑器
4. 按下开发板上的按键，观察输入效果：
   - KEY1：输入字母'a'
   - KEY2：输入"Hello"
   - KEY3：执行Ctrl+A（全选）