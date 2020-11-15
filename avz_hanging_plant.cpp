/*
 * @Coding: utf-8
 * @Author: elovi
 * @Date: 2020-11-15 13:25:30
 * @Description: avz extension: hanging plant manager
 * 版本：2
 * 功能：种植悬空植物
 * hangingPlantManager.start() 即开始
 * 进入游戏后，移动鼠标按 Q 设置悬空植物位置，按下 数字键 添加卡槽，设置完成时按下 W 确认
 * 再次修改位置，按 Q
 * 再次修改卡槽及确认卡槽修改，按 W
 * 任何时刻均可按下 E 获取帮助
 * 更新说明： 将暂停键和设置参数键合并，功能精简。
 * 添加更多提示内容，不会操作，按 E 即可获得帮助
 */

// 你可以自己设定按键绑定:
#define POS_CHAR 'Q'  // 鼠标设置位置键
#define PARA_CHAR 'W' // 修改卡槽键
#define INFO_CHAR 'E' // 获取帮助键

namespace AvZ
{
    class HangingPlantManager
    {
    private:
        bool setting_parameter;
        bool have_set_pos;
        bool first_enter;
        bool just_want_to_pause;

        int hangingrow;
        int hangingcolumn;

        int to_hang_plant_num;
        int cards[3];

        int flower_pot_index;
        bool found;
        bool planted;

        bool is_debuging;
        bool is_paused;

        TickRunner hangingPlantPlanter;

        bool is_using_default_key_bind;
        const char posChar = POS_CHAR;
        const char paraChar = PARA_CHAR;
        const char infoChar = INFO_CHAR;

        //主函数
        void plant_hanging_plant()
        {
            //刚进入游戏, 还未设置参数, 须返回
            if (first_enter)
            {
                return;
            }

            //若已种植则返回
            if (planted)
            {
                return;
            }

            //开始寻找花盆
            if (!found)
            {
                auto plant = mainObject()->plantArray();
                auto plantmax = mainObject()->plantTotal();
                for (int i = 0; i < plantmax; ++i, ++plant)
                {
                    if (plant->type() == FLOWER_POT && plant->row() + 1 == hangingrow && plant->col() + 1 == hangingcolumn && !plant->isDisappeared() && !plant->isCrushed())
                    {
                        found = true;
                        flower_pot_index = i;
                        break;
                    }
                }

                if (!found)
                {
                    //没找到
                    if (mainObject()->gameClock() % 500 == 0)
                    {
                        //每隔5秒提示一次
                        ShowErrorNotInQueue("花盆未找到. 请检查你是否已种下.");
                    }
                    return;
                }
                if (is_debuging)
                    ShowErrorNotInQueue("花盆已找到. 位置: (#, #).\n 正在等待时机种植悬空植物.", hangingrow, hangingcolumn);
            }

            //已找到，检查是否可以种植悬空植物
            auto plant = mainObject()->plantArray() + flower_pot_index;
            auto plantmax = mainObject()->plantCountMax();
            if (flower_pot_index < 0 || flower_pot_index >= plantmax)
            {
                //安全检查，以防Access Violation错误
                ShowErrorNotInQueue("Access Violation. index: #.\n 如果你搞不清发生了什么，请联系我.", flower_pot_index);
                return;
            }
            //检查植物是否仍然存在
            if (plant->isDisappeared() || plant->isCrushed())
            {
                ShowErrorNotInQueue("你的花盆忽然不见了! 怎么回事?");
                found = false;
                return;
            }
            //生命值小于零，此时种植即可悬空
            if (plant->hp() < 0)
            {
                for (int i = 0; i < to_hang_plant_num; ++i)
                {
                    CardNotInQueue(cards[i], hangingrow, hangingcolumn);
                }
                planted = true;
                if (is_debuging)
                    ShowErrorNotInQueue("悬空植物已种下.");
            }
        }

        void refresh()
        {
            found = false;
            planted = false;
        }

    public:
        //开始扫描辅助种植悬空植物
        //第一个参数: 是否使用默认按键绑定, 默认为是, 你也可以自己定义按键绑定.
        //第二个参数: 是否使用debug模式, 默认为否, 主要为我调试时所使用, 输出信息很多很烦人, 建议关闭.
        void start(bool _use_default_key_bind = true, bool debug = false)
        {
            setting_parameter = true;
            have_set_pos = false;
            first_enter = true;
            just_want_to_pause = false;

            to_hang_plant_num = 0;

            is_debuging = debug;

            hangingPlantPlanter.pushFunc([=]() { plant_hanging_plant(); });

            if (_use_default_key_bind)
            {
                is_using_default_key_bind = _use_default_key_bind;
                use_default_key_bind();
            }

            if (is_debuging)
            {
                ShowErrorNotInQueue("开始...");
            }
        }

        //设置悬空植物位置, 鼠标放到想要的位置即可设置
        void setPosition() { setPosition(MouseRow(), (int)(MouseCol() + 0.5)); }

        //设置悬空植物位置, 允许自己设定参数
        void setPosition(int r, int c)
        {
            int rowUpperbound = 5, colUpperbound = 9;
            if (mainObject()->scene() == 2 || mainObject()->scene() == 3)
            {
                rowUpperbound = 6;
            }
            if (r <= 0 || r > rowUpperbound || c <= 0 || c >= colUpperbound)
            {
                // 九列不可能种悬空植物, 篮球直接压
                ShowErrorNotInQueue("无效的行与列参数: (#, #)", r, c);
                return;
            }
            hangingrow = r;
            hangingcolumn = c;
            have_set_pos = true;
            refresh();
            if (is_debuging)
                ShowErrorNotInQueue("悬空植物位置已成功设定于: (#, #)", hangingrow, hangingcolumn);
        }

        //设置哪个卡槽种植悬空植物
        //如果是第一次进入, 则还需要设置位置
        //依次按下你需要种植的悬空植物的卡槽, 最多三个.
        void setParameters()
        {
            if (just_want_to_pause && !setting_parameter)
            {
                just_want_to_pause = false;
                hangingPlantPlanter.goOn();
                is_paused = false;
                refresh();
                if (is_debuging)
                {
                    ShowErrorNotInQueue("结束暂停, 即将恢复...");
                }
                return;
            }
            if (setting_parameter)
            {
                if (to_hang_plant_num == 0)
                {
                    ShowErrorNotInQueue("你还未设定悬空植物卡槽.");
                    return;
                }
                if (!have_set_pos)
                {
                    ShowErrorNotInQueue("你还未设置悬空植物位置.");
                    return;
                }
                if (first_enter)
                {
                    first_enter = false;
                }
                setting_parameter = false;
                if (is_paused)
                {
                    is_paused = false;
                    hangingPlantPlanter.goOn();
                }
                if (is_debuging)
                    ShowErrorNotInQueue("悬空植物参数设置完毕. 开始扫描...");
            }
            else
            {
                if (!first_enter)
                {
                    just_want_to_pause = true;
                }
                hangingPlantPlanter.pause();
                is_paused = true;
                if (is_debuging)
                    ShowErrorNotInQueue("已暂停.");
            }
        }

        //设置卡槽
        void setCard(int i)
        {
            if (!is_paused && !first_enter)
            {
                ShowErrorNotInQueue("需要先暂停才能设置卡槽.");
                return;
            }
            if (i <= 0 || i > 10)
            {
                ShowErrorNotInQueue("你设置的卡槽号码只能是从1到10之间的数.");
                return;
            }
            if (!setting_parameter)
            {
                setting_parameter = true;
                to_hang_plant_num = 0;
                just_want_to_pause = false;
            }
            if (to_hang_plant_num >= 0 && to_hang_plant_num <= 2)
            {
                bool overlapping = false;
                for (int j = 0; j < to_hang_plant_num; ++j)
                {
                    if (cards[j] == i)
                    {
                        overlapping = true;
                    }
                }
                if (overlapping)
                {
                    ShowErrorNotInQueue("你已经设置过种植该卡片!");
                    return;
                }
                cards[to_hang_plant_num] = i;
                to_hang_plant_num++;
                if (is_debuging)
                    ShowErrorNotInQueue("卡槽 # 已成功设置为第 # 种植的悬空植物.", i, to_hang_plant_num);
                refresh();
            }
            else if (to_hang_plant_num == 3)
            {
                ShowErrorNotInQueue("你最多只能设置 3 个悬空植物!");
            }
            else
            {
                ShowErrorNotInQueue("未料想到的错误. 请联系我.");
            }
        }

        std::string getArrayString()
        {
            if (to_hang_plant_num == 0)
            {
                return "";
            }
            if (to_hang_plant_num < 0 && to_hang_plant_num > 3)
            {
                ShowErrorNotInQueue("未料想到的错误. 请联系我.");
                return "";
            }
            std::stringstream arraystream;
            for (int i = 0; i < to_hang_plant_num; ++i)
            {
                if (i != 0)
                {
                    arraystream << ", ";
                }
                arraystream << cards[i];
            }
            return arraystream.str();
        }

        //告诉你悬空花盆的信息, 或者获得现在该如何操作的指示
        void debuggingInfo()
        {
            if (first_enter)
            {
                if (!have_set_pos && to_hang_plant_num == 0)
                {
                    ShowErrorNotInQueue("刚进游戏啥都不会? 你现在需要设置卡槽和位置参数. 按 # 设置鼠标所在格为位置. 依次按 数字键 设置卡槽, 最后按 # 确认.", posChar, paraChar);
                }
                if (!have_set_pos && to_hang_plant_num != 0)
                {
                    ShowErrorNotInQueue("你现在需要设置位置. 按 # 设置鼠标所在格为位置.", posChar);
                }
                if (have_set_pos && to_hang_plant_num == 0)
                {
                    ShowErrorNotInQueue("你现在需要设置卡槽. 依次按 数字键 设置卡槽, 最后按 # 确认.", paraChar);
                }
                if (have_set_pos && to_hang_plant_num != 0)
                {
                    ShowErrorNotInQueue("你已经设置过位置: (#, #). 你已设置过卡槽: (" + getArrayString() + "). 按 数字键 继续设置卡槽, 按 # 重新修改位置, 如已设置完毕按 # 确认.", hangingrow, hangingcolumn, posChar, paraChar);
                }
                return;
            }
            if (is_paused && !setting_parameter)
            {
                if (is_using_default_key_bind)
                {
                    ShowErrorNotInQueue("你已暂停进程. 按 数字键 设定植物, 按 # 确认. 不按 数字键 并直接按 # 则沿用之前的设定并继续.", paraChar, paraChar);
                }
                else
                {
                    ShowErrorNotInQueue("你已暂停进程. 你可以设定植物或沿用之前的设定并继续.");
                }
                return;
            }
            if (setting_parameter)
            {
                if (is_using_default_key_bind)
                {
                    ShowErrorNotInQueue("你已设置卡槽: (" + getArrayString() + "). 按 数字键 继续设置卡槽, 如已设置完毕按 # 确认.", paraChar);
                }
                else
                {
                    ShowErrorNotInQueue("你已设置卡槽: (" + getArrayString() + "). 如已设置完毕可以确认.");
                }
                return;
            }
            if (!found)
            {
                ShowErrorNotInQueue("位于 (#, #) 的花盆未找到. 你还没种, 或者被啃\\砸\\碾了?", hangingrow, hangingcolumn);
                return;
            }
            if (planted)
            {
                if (is_using_default_key_bind)
                {
                    ShowErrorNotInQueue("你的悬空植物已经种植成功. 按 # 选择下一个要种植悬空植物的位置.", posChar);
                }
                else
                {
                    ShowErrorNotInQueue("你的悬空植物已经种植成功. 你可以选择下一个要种悬空植物的位置或者植物的种类.");
                }
                return;
            }

            std::string info_first_part = "一切已准备就绪, 设定位置: (#, #), 设定依次种植卡槽: (";
            std::string array = getArrayString();
            std::string info_second_part = ")";
            std::string info_third_part = ", 等待篮球砸花盆. \n若想修改卡槽请按 #. 若想修改位置请按 #";
            std::string info_fourth_part = ".";
            std::string infoStr;
            if (is_using_default_key_bind)
            {
                infoStr = "一切已准备就绪, 设定位置: (#, #), 设定依次种植卡槽: (" + getArrayString() + "), 等待篮球砸花盆. \n若想修改卡槽请按 #. 若想修改位置请按 #.";
            }
            else
            {
                infoStr = "一切已准备就绪, 设定位置: (#, #), 设定依次种植卡槽: (" + getArrayString() + "), 等待篮球砸花盆.";
            }
            ShowErrorNotInQueue(infoStr, hangingrow, hangingcolumn, paraChar, posChar);
            if (is_debuging)
            {
                auto plant = mainObject()->plantArray() + flower_pot_index;
                ShowErrorNotInQueue("种悬空植物的花盆信息: 类型: #(33为花盆), 血量: #,\n 所在行: #, 所在列: #, 横坐标: #, 纵坐标: #\n, 已消失: #, 已被砸: #, 状态: #",
                                    plant->type(), plant->hp(),
                                    plant->row(), plant->col(), plant->xi(), plant->yi(),
                                    plant->isDisappeared(), plant->isCrushed(), plant->state());
            }
        }

        //使用默认按键绑定
        //按下 Q 设置鼠标所在格为所需种植悬空植物的格
        //按下 W 暂停扫描，开始设置种植悬空植物的卡槽，设置悬空植物位置，设置完成时再次按下 W 重新开始扫描
        //按下 数字键 添加卡槽
        //按下 E 获取提示信息
        void
        use_default_key_bind()
        {
            KeyConnect(posChar, [=]() { setPosition(); });
            KeyConnect(paraChar, [=]() { setParameters(); });
            KeyConnect('1', [=]() { setCard(1); });
            KeyConnect('2', [=]() { setCard(2); });
            KeyConnect('3', [=]() { setCard(3); });
            KeyConnect('4', [=]() { setCard(4); });
            KeyConnect('5', [=]() { setCard(5); });
            KeyConnect('6', [=]() { setCard(6); });
            KeyConnect('7', [=]() { setCard(7); });
            KeyConnect('8', [=]() { setCard(8); });
            KeyConnect('9', [=]() { setCard(9); });
            KeyConnect('0', [=]() { setCard(10); });
            KeyConnect(infoChar, [=]() { debuggingInfo(); });
        }
    };

    HangingPlantManager hangingPlantManager;

} // namespace AvZ
