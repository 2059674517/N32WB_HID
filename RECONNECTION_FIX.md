# 蓝牙HID键盘重连问题解决方案

## 问题描述
- 第一次连接设备蓝牙后键盘功能正常工作
- 重启单片机后按键无反应
- 需要删除配对重新连接才能工作

## 问题原因分析

### 根本原因
当设备已经绑定（bonded）并重新连接时，HID报告的通知（notification）配置没有正确恢复，特别是新增的键盘报告（Report ID 3）的通知没有被启用。

### 技术细节
1. **通知配置机制**：
   - 每个HID报告需要独立的通知配置
   - 首次连接时，客户端会启用需要的报告通知
   - 重连时应该恢复之前保存的通知配置

2. **原始问题**：
   - 原代码只配置了鼠标和多媒体报告的通知（ntf_cfg = 0xC2）
   - 没有包含键盘报告的通知配置
   - 设备配置只设置为鼠标（HOGPD_CFG_MOUSE）

## 实施的解决方案

### 1. 修改通知配置
```c
// 原代码
ntf_cfg = 0xC2;  // 只启用部分报告

// 修改后
ntf_cfg = 0xFFFF;  // 启用所有可能的报告通知
```

### 2. 更新设备类型配置
```c
// 原代码
db_cfg->cfg[0].svc_features = HOGPD_CFG_MOUSE;

// 修改后
db_cfg->cfg[0].svc_features = HOGPD_CFG_KEYBOARD | HOGPD_CFG_MOUSE;
```

### 3. 添加HID状态检查
在发送键盘报告前检查HID是否就绪：
```c
if (is_app_hid_ready()) {
    // 发送键盘报告
    app_hid_send_keyboard_report((uint8_t*)&kb_report);
} else {
    NS_LOG_WARNING("HID not ready, skipping keyboard send\r\n");
}
```

### 4. 增强调试日志
添加详细的调试信息以追踪问题：
```c
NS_LOG_DEBUG("HID ready for bonded device, enabling all notifications\r\n");
NS_LOG_DEBUG("Keyboard report state:%d, nb_report:%d\r\n", ...);
```

## 修改的文件

1. **app_hid.c**：
   - `app_hid_enable_prf()` - 修复通知配置
   - `app_hid_add_hids()` - 更新设备类型
   - `app_hid_send_keyboard_report()` - 添加调试日志

2. **app_gpio.c**：
   - `app_key_press_timeout_handler()` - 添加HID状态检查

## 测试步骤

### 初次配对测试
1. 编译并烧录固件
2. 在接收设备搜索并配对蓝牙
3. 测试所有按键功能正常

### 重连测试
1. 保持蓝牙配对状态
2. 重启单片机（断电重启）
3. 等待自动重连（通常3-5秒）
4. 测试按键功能应该正常工作

### 验证点
- [ ] 首次连接键盘功能正常
- [ ] 重启后自动重连
- [ ] 重连后键盘功能正常
- [ ] 无需重新配对

## 调试建议

### 1. 启用串口调试
观察以下关键日志：
```
HID ready for bonded device, enabling all notifications
Keyboard report state:3, nb_report:10
Button 1, sending keyboard 'ABC' with Shift
```

### 2. 检查通知状态
如果仍有问题，检查 `hogpd_ntf_cfg_ind_handler` 中的通知配置值：
```c
NS_LOG_DEBUG("ntf_cfg:0x%x\r\n", param->ntf_cfg[param->conidx]);
```
应该看到非零值，表示通知已启用。

### 3. 使用蓝牙调试工具
使用 nRF Connect 或类似工具：
- 查看HID服务的特征值
- 确认3个Report特征都存在
- 检查通知是否都已启用

## 进一步优化建议

### 1. 保存通知配置
可以考虑在NVDS中保存完整的通知配置：
```c
#if (NVDS_SUPPORT)
// 保存包含键盘报告的完整通知配置
nvds_put(NVDS_TAG_MOUSE_NTF_CFG, sizeof(ntf_cfg),
         (uint8_t *)&ntf_cfg);
#endif
```

### 2. 延迟发送
在重连后添加短暂延迟，确保通知完全配置：
```c
// 重连后等待100ms再发送第一个报告
ns_delay_ms(100);
```

### 3. 报告优先级
考虑为不同报告设置优先级，避免同时发送过多报告：
- 键盘报告：高优先级
- 鼠标报告：中优先级
- 多媒体报告：低优先级

## 注意事项

1. **清除旧配对**：如果之前已经配对，建议先删除旧配对再测试新固件
2. **兼容性**：不同操作系统处理HID重连的方式可能不同
3. **电源管理**：确保蓝牙模块的电源管理不会影响重连

## 故障恢复

如果修改后仍有问题：
1. 完全卸载设备驱动并重新安装
2. 清除蓝牙缓存（Windows: 设备管理器；手机: 清除蓝牙应用数据）
3. 使用不同的接收设备测试以排除兼容性问题