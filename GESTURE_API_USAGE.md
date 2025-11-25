# 触摸屏手势API使用说明

## 概述

本项目提供了一套简单易用的触摸屏手势API，封装了常用的多点触控手势操作。

## 屏幕尺寸配置

在 `app_hid_touchscreen.h` 中定义了屏幕尺寸：

```c
#define SCREEN_WIDTH  32767   // X轴最大值 (0-32767)
#define SCREEN_HEIGHT 32767   // Y轴最大值 (0-32767)
```

**注意**：修改这些宏定义后，HID报告描述符会自动同步更新。

## 高级手势API

### 1. 缩放手势 (Zoom)

```c
void app_gesture_zoom(uint8_t zoom_level, uint8_t is_zoom_in);
```

**功能**：执行二指缩放手势（捏合/拉开）

**参数**：
- `zoom_level`: 缩放等级 (1-10)，等级越高，缩放幅度越大
- `is_zoom_in`: 1 = 放大（手指拉开），0 = 缩小（手指捏合）

**示例**：
```c
// 放大 - 等级 5
app_gesture_zoom(5, 1);

// 缩小 - 等级 3
app_gesture_zoom(3, 0);

// 最大放大
app_gesture_zoom(10, 1);
```

---

### 2. 旋转手势 (Rotate)

```c
void app_gesture_rotate(int16_t angle_degrees);
```

**功能**：执行二指旋转手势

**参数**：
- `angle_degrees`: 旋转角度（-180 到 +180 度）
  - 正值 = 顺时针旋转
  - 负值 = 逆时针旋转

**示例**：
```c
// 顺时针旋转 90 度
app_gesture_rotate(90);

// 逆时针旋转 45 度
app_gesture_rotate(-45);

// 顺时针旋转 180 度
app_gesture_rotate(180);
```

---

### 3. 截屏手势 (Screenshot)

```c
void app_gesture_screenshot(uint8_t start_percent);
```

**功能**：执行三指下滑截屏手势

**参数**：
- `start_percent`: 起始位置占屏幕高度的百分比 (0-100)
  - 例如：20 表示从屏幕高度的 20% 处开始
  - 手势会向下滑动屏幕高度的 30%

**特点**：
- 三个触点会自动添加随机偏移量（±500像素），模拟真实手指触摸
- 每个触点的起始和结束位置都有轻微的Y轴偏移（±200像素）
- 自动避免超出屏幕边界

**示例**：
```c
// 从屏幕顶部 20% 开始三指下滑
app_gesture_screenshot(20);

// 从屏幕顶部 30% 开始三指下滑
app_gesture_screenshot(30);

// 从屏幕中部 50% 开始三指下滑
app_gesture_screenshot(50);
```

---

### 4. 单指滑动 (Swipe)

```c
void app_gesture_swipe(int16_t distance_pixels, uint8_t is_vertical);
```

**功能**：执行单指滑动手势

**参数**：
- `distance_pixels`: 滑动距离（像素）
  - 正值 = 向下/向右滑动
  - 负值 = 向上/向左滑动
- `is_vertical`: 1 = 垂直滑动，0 = 水平滑动

**特点**：
- 从屏幕中心开始滑动
- 滑动速度自动根据距离调整（200-600ms）

**示例**：
```c
// 向下滑动 5000 像素
app_gesture_swipe(5000, 1);

// 向上滑动 8000 像素
app_gesture_swipe(-8000, 1);

// 向右滑动 3000 像素
app_gesture_swipe(3000, 0);

// 向左滑动 6000 像素
app_gesture_swipe(-6000, 0);
```

---

## 实际应用示例

### 图片查看器操作

```c
// 放大图片
app_gesture_zoom(5, 1);
delay_n_ms(500);

// 缩小图片
app_gesture_zoom(5, 0);
delay_n_ms(500);

// 旋转图片 90度
app_gesture_rotate(90);
delay_n_ms(500);

// 向右滑动切换到下一张
app_gesture_swipe(8000, 0);
```

### 网页浏览操作

```c
// 向下滚动页面
app_gesture_swipe(10000, 1);
delay_n_ms(300);

// 向上滚动页面
app_gesture_swipe(-10000, 1);
delay_n_ms(300);

// 放大网页内容
app_gesture_zoom(7, 1);
```

### 截屏操作

```c
// 从屏幕顶部 20% 处三指下滑截屏
app_gesture_screenshot(20);
```

---

## 技术细节

### 随机偏移生成

三指滑动截屏使用伪随机数生成器为每个触点添加偏移：
- X轴偏移: ±500像素
- Y轴起始偏移: ±200像素
- Y轴结束偏移: ±200像素
- 水平移动: ±100像素

这样可以更真实地模拟人手触摸的自然变化。

### 坐标系统

- X轴: 0 到 SCREEN_WIDTH (默认 32767)
- Y轴: 0 到 SCREEN_HEIGHT (默认 32767)
- 所有API会自动处理边界检查，确保坐标不会超出范围

### HID报告描述符同步

修改 `SCREEN_WIDTH` 或 `SCREEN_HEIGHT` 后：
1. HID报告描述符会自动使用新的最大值
2. 不需要手动修改描述符中的硬编码值
3. 编译时会计算正确的字节表示

---

## 底层API（高级用户）

如果需要更精细的控制，仍然可以使用底层API：

```c
// 自定义二指捏合
app_touchscreen_pinch(center_x, center_y, start_distance, end_distance, duration_ms);

// 自定义二指旋转
app_touchscreen_rotate(center_x, center_y, radius, angle_degrees, duration_ms);

// 自定义单指滑动
app_touchscreen_swipe(x_start, y_start, x_end, y_end, duration_ms);

// 自定义多指滑动
app_multi_touchscreen_swipe(count, x_start, y_start, x_end, y_end, duration_ms);
```

---

## 注意事项

1. **延迟时间**：连续执行手势时建议添加适当延迟（300-500ms），让系统有时间处理
2. **坐标范围**：确保自定义坐标在 0 到 SCREEN_WIDTH/HEIGHT 范围内
3. **手势识别**：不同操作系统和应用对手势的识别阈值可能不同
4. **触点限制**：最多支持 3 个同时触点（MAX_TOUCH_POINTS）

---

## 编译配置

确保项目中包含以下文件：
- `app_hid_touchscreen.h` - 头文件
- `app_hid_touchscreen.c` - 实现文件
- `app_hid.c` - HID报告描述符

编译时会自动根据宏定义生成正确的HID描述符。
