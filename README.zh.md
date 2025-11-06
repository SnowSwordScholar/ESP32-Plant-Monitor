# ESP32 植物看护器（植物护理装置）

> 中文说明 — 同时提供英文版：`README.md`

---

## 项目简介

这是一个基于 ESP32 的植物护理装置。它通过一个模拟土壤湿度传感器检测土壤湿度，并可以驱动水泵（通过继电器或 MOSFET）对植物进行浇水。设备可接入 Home Assistant（通过 MQTT），并提供以下实体：

- 土壤湿度传感器（0–100 %）
- 浇水泵开关（手动/远程控制）
- 状态指示 LED 开关


![Home Assistant 示例：](image/screenshot.png)


## 硬件接线示例

- ESP32 GPIO34 (ADC1_CH6) -> 土壤湿度传感器模拟输出
- ESP32 GPIO32 -> 继电器模块输入（控制水泵）
- ESP32 GPIO2 -> 状态 LED（可选）
- 水泵 -> 外部电源，由继电器或 MOSFET 开关控制

重要：不要直接用 ESP32 GPIO 驱动水泵。请使用继电器模块或合适的 MOSFET 驱动，并为感性负载加反向二极管或合适的抑制措施。

## 软件与实体

项目通过 `ArduinoHA` 实现 Home Assistant（MQTT）集成。默认已实现的实体如下：

- `sensor.plant_soil_moisture` — 土壤湿度（%）
- `switch.plant_pump` — 浇水泵（开/关）
- `switch.plant_led` — 状态 LED

实体 ID 和 MQTT 相关配置在 `src/main.cpp` 中（WiFi 与 Broker 设置）。


### 初始化步骤

1. 克隆仓库后，复制 `src/config.template.h` 为 `src/config.h`：
   ```bash
   cp src/config.template.h src/config.h
   ```

2. 编辑 `src/config.h`，填入你的真实 WiFi、MQTT 凭据：
   ```cpp
   #define WIFI_SSID       "your_wifi_name"
   #define WIFI_PASSWORD   "your_wifi_password"
   #define BROKER_ADDR     IPAddress(192,168,1,100)  // HA 服务器 IP
   #define BROKER_PORT     1883
   #define BROKER_USER     "mqtt_user"
   #define BROKER_PASS     "mqtt_password"
   ```

3. 保存后编译上传，系统将自动加载本地配置。


## 快速开始（构建与烧录）

克隆项目
```
git clone https://github.com/SnowSwordScholar/ESP32-Plant-Monitor
```
使用 VSCode 打开项目文件夹，之后安装 PlatformIO 插件  
在编辑器中修改自己的配置  
使用底栏的 → 按钮进行烧录

## 校准

库 `lib/SoilMoistureSensor` 将 ADC 原始值转换为电压，并使用两个参考电压将电压映射为 0–100 %：

- `wetVoltage` — 探头在非常湿的土壤（或短接）时的电压，默认 0.0 V
- `dryVoltage` — 探头在空气中或非常干燥时的电压，默认 3.3 V

可通过 `soilSensor.setCalibration(wetV, dryV)` 在代码中设定校准值（参见 `lib/SoilMoistureSensor/SoilMoistureSensor.h`）。

校准建议：
1. 将探头插入湿土（或短接探针），记录电压，作为 `wetVoltage`。
2. 将探头置于空气中，记录电压，作为 `dryVoltage`。
3. 在代码中应用这些值并重新烧录设备。

## 安全与硬件注意事项

- 使用隔离继电器模块或带驱动的 MOSFET 来控制水泵。ESP32 GPIO 无法直接驱动泵电流。
- 确认继电器模块的触发逻辑（有些模块为高电平触发）。若为高电平触发，请在 `onPumpCommand` 中反转输出逻辑。
- 给水泵电源配置合适的保险丝并妥善布线。

## Home Assistant 集成

设备启动并连接到 MQTT 后，如果启用了发现（discovery），Home Assistant 应自动识别传感器与开关。也可查看串口输出以确认 MQTT/发现消息。

## 故障排查

- 无法连接 MQTT：检查 `BROKER_ADDR`、`BROKER_USER`、`BROKER_PASS` 与网络连接。
- 泵不动作：检查继电器接线、用万用表测试 GPIO32 是否有输出、确认继电器电源与共地。
