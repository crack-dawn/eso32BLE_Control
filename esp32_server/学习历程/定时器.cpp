

// Ԥ�ȶ���һ��ָ������Ŷ�ʱ����λ��

// hw_timer_t *Timer = NULL;

// ����֪ʶ����ʱ�����ͺ�ѡ�� Ԥ��Ƶ����Ƶ��80MHz�� ��ʱ�����¼�����true����
//     Timer = timerBegin(0, 80, true);

// ��ʼ����Ϻ򣬽���ʱ�����ӵ��жϣ� ��ʱ����ַָ�� �жϴ����� �жϱ��ش�������
//     timerAttachInterrupt(Timer, &onTimer, true);

// ��ʱ�� �����Ķ�ʱ�� ��ʱʱ�� ��ֵ�Ƿ����ء����ڶ�ʱ����
//     timerAlarmWrite(Timer, time, true);

// ��ʼ������ ������һ����ʱ����
//     timerAlarmEnable(Timer);

// �����жϺ��������ж�Ӧ���ص�IRAM�У����޷���ֵ��
// void IRAM_ATTR onTimer()
// {
//     �жϺ�������
// }