// 夜棂依Yareiy 260114 v0.1

#include <Arduino.h>
#include <Wire.h>

#define SDA_PIN 21
#define SCL_PIN 22


const uint8_t BQ27742_I2C_ADDR = 0x55; // BQ27742的I2C地址

// 2-byte标准命令
uint16_t readWord(uint8_t reg) {
  // 写地址
  Wire.beginTransmission(BQ27742_I2C_ADDR);
  Wire.write(reg);
  uint8_t error = Wire.endTransmission(false);

  if (error != 0) {
    Serial.print("I2C错误 - 寄存器0x");
    Serial.println(reg, HEX);
    return 0xFFFF; // 错误
  }

  // 读2字节数据
  Wire.requestFrom(BQ27742_I2C_ADDR, (uint8_t)2);

  if (Wire.available() >= 2) {
    uint8_t lsb = Wire.read();
    uint8_t msb = Wire.read();
    return ((uint16_t)msb << 8) | lsb;
  }
  
  return 0xFFFF; // 错误
}

// “Control()”命令
uint16_t readControlSubcommand(uint16_t subcmd)
{
  Wire.beginTransmission(BQ27742_I2C_ADDR);
  Wire.write(0x00); // 发送“Control()”命令
  Wire.write(lowByte(subcmd));   // LSB在前
  Wire.write(highByte(subcmd));  // MSB
  Wire.endTransmission();

  delay(1); // 延迟1毫秒等待芯片处理

  Wire.beginTransmission(BQ27742_I2C_ADDR);
  Wire.write(0x00); // 发送“Control()”命令
  Wire.endTransmission(false);

  if (Wire.requestFrom(BQ27742_I2C_ADDR, (uint8_t)2) != 2) {
    return 0xFFFF; // 读取失败
  }

  uint8_t lsb = Wire.read();
  uint8_t msb = Wire.read();
  return ((uint16_t)msb << 8) | lsb;
}

// 命令函数
// 芯片状态
void GetStatus() {

  uint16_t status = readControlSubcommand(0x0000);
  Serial.print("控制状态 = 0x");
  Serial.println(status, HEX);

  // 高字节解析
  bool ocvtaken = status & (1 << 15);
  bool fas      = status & (1 << 14);
  bool ss       = status & (1 << 13);
  bool calmode  = status & (1 << 12);
  bool cca      = status & (1 << 11);
  bool bca      = status & (1 << 10);

  // 低字节
  bool shutdn   = status & (1 << 7);
  bool fettst   = status & (1 << 6);
  bool fullsleep = status & (1 << 5);
  bool sleep    = status & (1 << 4);
  bool ldmd     = status & (1 << 3);
  bool rup_dis  = status & (1 << 2);
  bool vok      = status & (1 << 1);
  bool qen      = status & (1 << 0);

  Serial.println("---------------------------");
  Serial.print("- OCVTAKEN: "); Serial.println(ocvtaken ? "是" : "否");
  Serial.print("- FAS: ");    Serial.println(fas ? "完全锁定" : "已解锁");
  Serial.print("- SS: ");        Serial.println(ss ? "锁定" : "已解锁");
  Serial.print("- CALMODE: ");    Serial.println(calmode ? "开启" : "关闭");
  Serial.print("- CCA: ");      Serial.println(cca ? "运行" : "未运行");
  Serial.print("- BCA: ");      Serial.println(bca ? "运行" : "未运行");
  Serial.print("- SHUTDOWN_EN: "); Serial.println(shutdn ? "有" : "无");
  Serial.print("- FETTST: "); Serial.println(fettst ? "已启用" : "已禁用");
  Serial.print("- FULLSLEEP: "); Serial.println(fettst ? "已启用" : "已禁用");
  Serial.print("- SLEEP: ");       Serial.println(sleep ? "已启用" : "已禁用");
  Serial.print("- LDMD: ");       Serial.println(ldmd ? "恒功率" : "恒电流");
  Serial.print("- RUP_DIS: ");         Serial.println(rup_dis ? "禁用" : "启用");
  Serial.print("- VOK: ");         Serial.println(vok ? "已就绪" : "未就绪");
  Serial.print("- QEN: ");     Serial.println(qen ? "启用" : "禁用");
  Serial.println("---------------------------");
}

// 获取电压，单位毫伏
uint16_t Voltage() {
  return readWord(0x08);
}

// 获取电池温度
float Temperature() {
  uint16_t tempRaw = readWord(0x06); 
  float tempK = tempRaw * 0.1;
  float tempC = tempK - 273.15;
  return tempC;
}

// 获取芯片温度
float InternalTemperature() {
  uint16_t tempRaw = readWord(0x28); 
  float tempK = tempRaw * 0.1;
  float tempC = tempK - 273.15;
  return tempC;
}

// 获取当前电量百分比
uint16_t StateOfCharge() {
  return readWord(0x2C);
}

// 获取当前电量毫安时
uint16_t RemainingCapacity() {
  return readWord(0x10);
}

// 获取理论当前电量毫安时
uint16_t NomAvailableCapacity() {
  return readWord(0x0C);
}

// 获取充满容量
uint16_t FullChargeCapacity() {
  return readWord(0x12);
}

// 获取理论充满容量
uint16_t FullAvailableCapacity() {
  return readWord(0x0E);
}

// 获取预计剩余使用时间
uint16_t TimeToEmpty() {
  return readWord(0x16);
}

// 获取循环次数
uint16_t CycleCount() {
  return readWord(0x2A);
}

// 获取电池健康度百分比
uint16_t StateOfHealth() {
  return readWord(0x2E);
}

// 获取预估自放电电流
int16_t SelfDischargeCurrent() {
  return (int16_t)readWord(0x38);
}

// 获取当前电池电流，单位毫安
int16_t AverageCurrent() {
  return (int16_t)readWord(0x14);
}

void setup()
{
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  delay(1500);
  Serial.println("#############################");
  Serial.println("开始：<(￣︶￣)↗[GO!]");

  //获取状态
  GetStatus(); 
  delay(100); 

  Serial.printf("电压: %u mV\n", Voltage());
  delay(1); 
  Serial.printf("电流: %d mA\n", AverageCurrent());
  delay(1);
  Serial.printf("电量(%): %u %%\n", StateOfCharge());
  delay(1);
  Serial.printf("电量(mAh): %u mAh\n", RemainingCapacity());
  delay(1);
  Serial.printf("理论电量(mAh): %u mAh\n", NomAvailableCapacity());
  delay(1);
  Serial.printf("满充容量: %u mAh\n", FullChargeCapacity());
  delay(1);
  Serial.printf("理论满充容量: %u mAh\n", FullAvailableCapacity());
  delay(1);
  Serial.printf("电芯温度: %.1f °C\n", Temperature());
  delay(1);
  Serial.printf("芯片温度: %.1f °C\n", InternalTemperature());
  delay(1);
  Serial.printf("循环次数: %u 次\n", CycleCount());
  delay(1);
  Serial.printf("健康度: %u %%\n", StateOfHealth());
  delay(1);
  Serial.printf("自放电电流: %d mA\n", SelfDischargeCurrent());
  delay(1);

  // 剩余时间
  uint16_t tte = TimeToEmpty();
  if (tte == 65535) {
    Serial.printf("预计剩余使用时间: 未在放电\n");
  } else {
    Serial.printf("预计剩余使用时间: %u 分钟\n", tte);
  }

  Serial.println("搞定(≧∇≦)ﾉ");
  Serial.println("#############################");

  delay(3000); 

}

void loop()
{
}
