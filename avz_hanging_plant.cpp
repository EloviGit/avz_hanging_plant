/*
 * @Coding: utf-8
 * @Author: elovi
 * @Date: 2020-11-14 23:55:35
 * @Description: avz extension: hanging plant manager
 * 功能：种植悬空植物
 * hangingPlantManager.start() 即开始
 * 按下 H 设置鼠标所在格为所需种植悬空植物的格
 * 按下 J 开始设置种植悬空植物的卡槽，或者表示设置完成
 * 按下 K 暂停，若已经暂停，则恢复
 * 按下 数字键 添加卡槽
 * 按下 Q 获取提示信息
 * 你可以根据自己的需求更改按键绑定
 */

namespace AvZ
{
    class HangingPlantManager
    {
    private:
        int hangingrow;
        int hangingcolumn;

        int to_hang_plant_num;
        int cards[3];

        bool setting_parameter;
        int flower_pot_index;
        bool found;
        bool planted;

        bool is_debuging;
        bool is_paused;

        TickRunner hangingPlantPlanter;

        //主函数
        void plant_hanging_plant()
        {
            //若已种植或正在设置参数则返回
            if (planted || setting_parameter)
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
                    ShowErrorNotInQueue("花盆已找到. 坐标: (#, #).\n 正在等待时机种植悬空植物.", hangingrow, hangingcolumn);
            }

            //已找到，检查是否可以种植悬空植物
            auto plant = mainObject()->plantArray() + flower_pot_index;
            auto plantmax = mainObject()->plantCountMax();
            if (flower_pot_index < 0 || flower_pot_index >= plantmax)
            {
                //安全检查，以防Access Violation错误
                ShowErrorNotInQueue("Access Violation. index: #.\n 如果逆搞不清发生了什么，请联系我.", flower_pot_index);
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
            hangingrow = -1;
            hangingcolumn = -1;

            to_hang_plant_num = 0;

            setting_parameter = true;

            is_debuging = debug;
            is_paused = false;

            hangingPlantPlanter.pushFunc([=]() { plant_hanging_plant(); });

            if (_use_default_key_bind)
            {
                use_default_key_bind();
            }
        }

        //设置悬空植物位置, 鼠标放到想要的位置即可设置
        void setPosition()
        {
            hangingrow = MouseRow();
            hangingcolumn = (int)(MouseCol() + 0.5);
            refresh();
            if (is_debuging)
                ShowErrorNotInQueue("悬空植物位置已成功设定于: (#, #)", hangingrow, hangingcolumn);
        }

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
                ShowErrorNotInQueue("无效的行与列参数: (#, #)", r, c);
                return;
            }
            hangingrow = r;
            hangingcolumn = c;
            refresh();
            if (is_debuging)
                ShowErrorNotInQueue("悬空植物位置已成功设定于: (#, #)", hangingrow, hangingcolumn);
        }

        //设置哪个卡槽种植悬空植物
        //如果是第一次进入, 则还需要设置位置
        //依次按下你需要种植的悬空植物的卡槽, 最多三个.
        void setParameters()
        {
            if (!setting_parameter)
            {
                setting_parameter = true;
                to_hang_plant_num = 0;
                if (is_debuging)
                    ShowErrorNotInQueue("即将设定新参数...");
            }
            else
            {
                if (hangingrow == -1 && hangingcolumn == -1)
                {
                    ShowErrorNotInQueue("你还未设置悬空植物位置.");
                    return;
                }
                if (to_hang_plant_num == 0)
                {
                    ShowErrorNotInQueue("你还未设定悬空植物卡槽.");
                    return;
                }
                setting_parameter = false;
                if (is_debuging)
                    ShowErrorNotInQueue("悬空植物参数设置完毕.");
            }
        }

        //暂停和恢复,切换
        void pauseAndGoOn()
        {
            if (is_paused)
            {
                is_paused = false;
                InsertGuard ig(true);
                SetNowTime();
                hangingPlantPlanter.goOn();
                refresh();
                if (is_debuging)
                    ShowErrorNotInQueue("即将恢复...");
            }
            else
            {
                is_paused = true;
                InsertGuard ig(true);
                SetNowTime();
                hangingPlantPlanter.pause();
                if (is_debuging)
                    ShowErrorNotInQueue("已暂停.");
            }
        }

        //设置卡片
        void setCard(int i)
        {
            if (to_hang_plant_num >= 0 && to_hang_plant_num <= 2)
            {
                if (i <= 0 || i >= 10)
                {
                    ShowErrorNotInQueue("你设置的卡槽号码只能是从1到10之间的数.");
                    return;
                }
                cards[to_hang_plant_num] = i;
                to_hang_plant_num++;
                if (is_debuging)
                    ShowErrorNotInQueue("卡槽 # 已成功设置为第 # 种植的悬空植物.", i, to_hang_plant_num);
                refresh();
            }
            else
            {
                ShowErrorNotInQueue("你最多只能设置 3 个悬空植物!");
            }
        }

        //告诉你悬空花盆的信息, 或者获得现在该如何操作的指示
        void debuggingInfo()
        {
            if (setting_parameter)
            {
                ShowErrorNotInQueue("你现在需要设置卡槽和位置参数.");
                return;
            }
            if (!found)
            {
                ShowErrorNotInQueue("位于 (#, #) 的花盆未找到.", hangingrow, hangingcolumn);
                return;
            }
            if (planted)
            {
                ShowErrorNotInQueue("你的悬空植物已经种植成功.");
                return;
            }

            //懒得翻译了, 你自己看吧
            auto plant = mainObject()->plantArray() + flower_pot_index;
            ShowErrorNotInQueue("plant info:\nType: #, hp: #\n, row: #, col: #, xi: #, yi: #\n, disappeared: #, crushed: #, state: #",
                                plant->type(), plant->hp(),
                                plant->row(), plant->col(), plant->xi(), plant->yi(),
                                plant->isDisappeared(), plant->isCrushed(), plant->state());
        }

        //使用默认按键绑定
        //按下 H 设定鼠标所在格为悬空植物种植格
        //按下 J 开始设定要种悬空植物的卡槽, 或示意你已经选择完毕
        //按下 数字键 添加你想要种植的悬空植物
        //按下 K 暂停, 若已经暂停, 则恢复
        //按下 Q 获取提示信息
        void use_default_key_bind()
        {
            KeyConnect('H', [=]() { setPosition(); });
            KeyConnect('J', [=]() { setParameters(); });
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
            KeyConnect('K', [=]() { pauseAndGoOn(); });
            KeyConnect('Q', [=]() { debuggingInfo(); });
        }
    };

    HangingPlantManager hangingPlantManager;

} // namespace AvZ
