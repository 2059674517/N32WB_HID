# HID 多点触摸屏功能说明

## 功能概述
本项目已成功实现HID多点触摸屏功能，支持1-5个手指的任意数量触摸操作。该实现完全符合USB HID规范，可以在Windows、Linux、macOS等操作系统上正常工作。

## 主要特性
- **支持1-5个触摸点**：可同时识别并处理最多5个手指的触摸
- **标准HID协议**：完全兼容USB HID触摸屏标准协议
- **多种手势支持**：包括单击、滑动、多指点击、缩放、旋转等
- **高精度坐标**：支持12位坐标精度（0-4095）
- **实时响应**：低延迟的触摸事件传输

## 技术实现

### 1. HID报告描述符
修改了HID报告描述符（Report ID 4），为每个触摸点定义独立的数据结构：
- 每个触摸点包含：状态位、接触ID、X/Y坐标
- 总共定义5个触摸点的完整结构
- 报告大小：31字节（6字节×5个触摸点 + 1字节计数）

### 2. 数据结构
```c
typedef struct {
    uint8_t tip_switch : 1;     // 手指触摸状态
    uint8_t in_range : 1;       // 手指在范围内
    uint8_t touch_valid : 1;    // 触摸数据有效
    uint8_t padding : 5;        // 填充位
    uint8_t contact_id;         // 接触ID (0-4)
    uint16_t x;                 // X坐标 (0-4095)
    uint16_t y;                 // Y坐标 (0-4095)
} hid_touch_point_t;
```

### 3. 主要API函数

#### 基础触摸功能
- `app_hid_send_multitouch()` - 发送多点触摸报告
- `app_hid_send_touchscreen()` - 发送单点触摸（便捷函数）
- `app_touchscreen_tap()` - 模拟点击
- `app_touchscreen_swipe()` - 模拟滑动

#### 多点触摸手势
- `app_touchscreen_multi_tap()` - 多指同时点击
- `app_touchscreen_pinch()` - 双指缩放手势
- `app_touchscreen_rotate()` - 双指旋转手势

## 使用示例

### 按键演示功能
按键3（KEY3）配置了完整的多点触摸演示序列，每次按下会循环执行以下操作：

1. **单指点击** - 在屏幕中心点击
2. **单指滑动** - 从左向右滑动
3. **双指点击** - 两个手指同时点击
4. **三指点击** - 三个手指同时点击
5. **四指点击** - 四个手指同时点击
6. **五指点击** - 五个手指同时点击（全手）
7. **缩放手势** - 双指捏合放大
8. **旋转手势** - 双指旋转90度

### 代码示例

#### 发送双指点击：
```c
uint16_t x_coords[2] = {1500, 2500};
uint16_t y_coords[2] = {2048, 2048};
app_touchscreen_multi_tap(2, x_coords, y_coords);
```

#### 执行缩放手势：
```c
// 中心点(2048,2048)，从800像素距离缩放到1600像素
app_touchscreen_pinch(2048, 2048, 800, 1600, 500);
```

#### 执行旋转手势：
```c
// 中心点(2048,2048)，半径600，旋转90度
app_touchscreen_rotate(2048, 2048, 600, 90, 800);
```

## 文件结构

- `user/inc/app_hid_touchscreen.h` - 触摸屏API声明和数据结构定义
- `user/src/app_hid_touchscreen.c` - 多点触摸功能实现
- `user/src/app_profile/app_hid.c` - HID报告描述符定义
- `user/src/app_gpio.c` - 按键中断处理和演示代码

## 兼容性
- Windows 8及以上版本（原生支持）
- Linux（需要内核支持HID多点触摸）
- macOS（支持多点触摸的版本）
- Android（通过USB OTG）

## 注意事项
1. 坐标范围为0-4095，应用程序需要根据实际屏幕分辨率进行映射
2. 同时最多支持5个触摸点
3. 每个触摸点必须有唯一的contact_id（0-4）
4. 释放所有触摸时，发送contact_count=0的报告

## 调试信息
使用NS_LOG查看调试信息：
- `NS_LOG_INFO` - 显示触摸操作信息
- `NS_LOG_DEBUG` - 显示详细的触摸点数据
- `NS_LOG_WARNING` - 显示错误和警告信息