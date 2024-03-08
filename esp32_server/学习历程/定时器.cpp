

// 预先定义一个指针来存放定时器的位置

// hw_timer_t *Timer = NULL;

// 备用知识：定时器的型号选用 预分频【主频：80MHz】 定时器上下计数【true？】
//     Timer = timerBegin(0, 80, true);

// 初始化完毕候，将定时器连接到中断： 定时器地址指针 中断处理函数 中断边沿触发类型
//     timerAttachInterrupt(Timer, &onTimer, true);

// 定时： 操作的定时器 定时时长 数值是否重载【周期定时？】
//     timerAlarmWrite(Timer, time, true);

// 开始启动： 启动哪一个定时器？
//     timerAlarmEnable(Timer);

// 定义中断函数：【中断应加载到IRAM中，且无返回值】
// void IRAM_ATTR onTimer()
// {
//     中断函数区域
// }