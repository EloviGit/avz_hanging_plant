#include "avz.h"
#include "avz_hanging_plant.cpp"

using namespace AvZ;

void Script()
{
    hangingPlantManager.start();
    OpenMultipleEffective('Z', MAIN_UI_OR_FIGHT_UI);
    SetErrorMode(POP_WINDOW);
}
