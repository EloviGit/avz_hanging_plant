/*
 * @Coding: utf-8
 * @Author: elovi
 * @Date: 2020-11-15 13:25:30
 * @Description: avz extension: hanging plant manager -- usage example
 */

#include "avz.h"
#include "src/avz_hanging_plant.cpp" // 将 "avz_hanging_plant.cpp" 放进 src 文件并加上这句话

using namespace AvZ;

void Script()
{
    OpenMultipleEffective('Z', MAIN_UI_OR_FIGHT_UI);
    hangingPlantManager.start(); // 只需要这一句话插件就可开始运行
}
