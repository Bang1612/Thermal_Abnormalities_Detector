#include "Arduino.h"
#include <Wire.h>
#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"

#define EMMISIVITY 0.95
#define TA_SHIFT 8 
#define LED_BUILTIN 2

paramsMLX90640 mlx90640;
const byte MLX90640_address = 0x33; // Địa chỉ mặc định của MLX90640
static float tempValues[32 * 24];

void setup() {
  Serial.begin(115200);
  // Serial.println("ESP32 đã bắt đầu chạy.");
  Wire.begin(21, 22); // Chân SDA = 21, SCL = 22 (ESP32)
  Wire.setClock(400000); 
  pinMode(LED_BUILTIN, OUTPUT); 
  Wire.beginTransmission((uint8_t)MLX90640_address);
  if (Wire.endTransmission() != 0) {
    while (1) {
      Serial.println("Không phát hiện thấy MLX90640.");
      delay(2000);
    }
  } else {
    Serial.println("MLX90640 đã kết nối!");
  }

  uint16_t eeMLX90640[832];
  int status = MLX90640_DumpEE(MLX90640_address, eeMLX90640);
  if (status != 0) Serial.println("Không thể tải tham số hệ thống");
  status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
  if (status != 0) Serial.println("Trích xuất tham số thất bại");

  MLX90640_SetRefreshRate(MLX90640_address, 0x05);
  Wire.setClock(800000);
}

void loop() {
  readTempValues();
  digitalWrite(LED_BUILTIN, HIGH);  // Bật LED
  // delay(1000);                      // Chờ 1 giây
  digitalWrite(LED_BUILTIN, LOW);   // Tắt LED
  delay(1000);       
  delay(2000);
}

void readTempValues() {
  uint16_t mlx90640Frame[834];
  int status = MLX90640_GetFrameData(MLX90640_address, mlx90640Frame);
  if (status < 0) {
    Serial.print("Lỗi lấy dữ liệu khung: ");
    Serial.println(status);
    return;
  }

  float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
  float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);
  float tr = Ta - TA_SHIFT;

  MLX90640_CalculateTo(mlx90640Frame, &mlx90640, EMMISIVITY, tr, tempValues);
  

  Serial.println("\r\n=========================== Nhiệt độ đo được từ MLX90640 ===============================");
  for (int i = 0; i < 768; i++) {
    if ((i % 32) == 0 && i != 0) {
      Serial.println();
    }
    Serial.print(tempValues[i], 1); // Hiển thị nhiệt độ với 1 chữ số thập phân
    Serial.print(" ");
  }
  Serial.println("\r\n========================================================================================");
}
