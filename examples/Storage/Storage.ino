/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with M5Core sample source code
*                          配套  M5Core 示例源代码
* Visit the website for more
information: https://docs.m5stack.com/en/core/gray
* 获取更多资料请访问: https://docs.m5stack.com/zh_CN/core/gray
*
* describe: NVS Storage.
* date: 2022/5/1
*******************************************************************************
*/

#include <M5Stack.h>
#include <M5BurnerNVS.h>

M5BurnerNVS burner_nvs;
TFT_eSprite img = TFT_eSprite(&M5.Lcd);

void setup() {
    M5.begin();
    M5.Power.begin();
    burner_nvs.init();
    burner_nvs.listen();
    img.setColorDepth(8);
    img.createSprite(320, 240);  //创建一块320x240的画布
    img.setTextSize(1);
    img.drawString("GET                             SET", 55, 220);
    img.setTextSize(2);
    img.drawString("NVS Example", 90, 0);
    img.pushSprite(0, 0);
}

const char* value = "M5Stack";

void loop() {
    M5.update();
    img.setCursor(0, 100);
    if (M5.BtnA.wasPressed()) {
        img.fillRect(0, 20, 320, 200, BLACK);
        img.printf("The 'Key' value is:");
        img.print(burner_nvs.get("key"));
    }
    if (M5.BtnC.wasPressed()) {
        img.fillRect(0, 20, 320, 200, BLACK);
        img.printf("Set 'Key' value is:%s", value);
        burner_nvs.set("key", value);
    }
    img.pushSprite(0, 0);
    delay(10);
}