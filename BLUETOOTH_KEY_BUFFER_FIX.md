# 蓝牙HID设备按键缓冲机制修复

## 问题描述

N32WB031蓝牙鼠标/键盘设备存在以下问题：
1. 设备掉电重启后，若立即点击多次按键，连接的蓝牙设备无响应且会导致蓝牙HID设备不可用
2. 设备保持供电但断开蓝牙配对后，立即点击多次按键再进行配对连接，配对连接后也会导致蓝牙HID设备不可用
3. 日志显示：`HID not ready, skipping keyboard send` - 按键事件被丢弃

## 根本原因

1. **HID服务状态管理问题**：
   - 设备重启或蓝牙断开连接后，HID服务状态为`APP_HID_IDLE`
   - HID服务需要时间完成初始化并转换到`APP_HID_READY`状态
   - 只有在`APP_HID_READY`状态下才能发送HID报告

2. **按键事件丢失**：
   - 在HID服务未就绪时收到的按键事件直接被丢弃
   - 没有缓冲机制来暂存这些早期的按键事件
   - 导致用户按键无响应，影响用户体验

## 解决方案

实现了一个按键事件缓冲机制：

### 1. 按键缓冲队列（app_gpio.c）

```c
// 按键事件缓冲结构
#define KEY_BUFFER_SIZE 32
typedef struct {
    uint8_t key_id;     // 1 for KEY1, 2 for KEY2, 3 for KEY3
    uint32_t timestamp; // 可选：记录按键时间
} key_event_t;

typedef struct {
    key_event_t events[KEY_BUFFER_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
} key_buffer_t;
```

### 2. 缓冲机制工作流程

1. **HID未就绪时**：
   - 按键事件被添加到缓冲队列
   - 设置定时器定期检查HID状态（200ms）
   - 记录日志：`HID not ready, buffering KEY[x]`

2. **HID就绪时**：
   - 自动处理缓冲队列中的所有按键事件
   - 按顺序发送每个缓冲的按键
   - 缓冲事件之间添加50ms延迟，确保稳定传输

3. **HID状态变化回调**：
   - 当HID服务从其他状态转换到`APP_HID_READY`时
   - 自动调用`app_process_buffered_keys()`处理缓冲的按键

### 3. 修改的文件

1. **user/src/app_gpio.c**：
   - 添加按键缓冲队列数据结构
   - 实现缓冲队列的push/pop操作
   - 修改`app_key_press_timeout_handler()`处理缓冲逻辑
   - 添加`app_process_buffered_keys()`函数

2. **user/inc/app_gpio.h**：
   - 声明`app_process_buffered_keys()`函数

3. **user/src/app_profile/app_hid.c**：
   - 包含`app_gpio.h`头文件
   - 在HID变为READY状态时调用`app_process_buffered_keys()`
   - 两处调用点：初始连接和通知使能

## 测试验证

### 测试场景1：设备重启
1. 断电重启设备
2. 立即快速按键多次
3. 等待蓝牙连接建立
4. **预期结果**：所有按键事件被缓冲并在连接后正确发送

### 测试场景2：蓝牙重连
1. 保持设备供电
2. 断开蓝牙连接
3. 立即快速按键多次
4. 重新连接蓝牙
5. **预期结果**：所有按键事件被缓冲并在重连后正确发送

### 测试场景3：正常使用
1. 蓝牙已连接且HID就绪
2. 按键操作
3. **预期结果**：按键立即响应，无延迟

## 优势

1. **无按键丢失**：所有按键事件都被缓冲，不会丢失
2. **用户体验改善**：即使在设备初始化期间，用户的操作也会被记录
3. **向后兼容**：不影响正常连接状态下的按键响应
4. **可配置**：缓冲区大小可通过`KEY_BUFFER_SIZE`调整

## 注意事项

1. 缓冲区大小为32个按键事件，超出会丢弃最早的事件
2. HID未就绪时的重试间隔为200ms
3. 缓冲事件之间的发送间隔为50ms
4. 需要确保`ns_timer`和`ns_delay`功能正常工作

## 编译和部署

1. 重新编译项目
2. 下载固件到设备
3. 测试各种场景确保问题已解决

## 日志输出示例

```
// HID未就绪时
HID not ready, buffering KEY1
HID not ready, buffering KEY2

// HID就绪后
HID Ready
HID is ready, processing 2 buffered keys
Processing buffered key: 1
Processing buffered key: 2
```