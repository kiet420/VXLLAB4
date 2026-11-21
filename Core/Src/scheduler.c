#include "scheduler.h"
#include "main.h"

#define NIL 0xFF

typedef struct {
    void (*pTask)(void);
    uint32_t Delay;      // delta (tick) so với task đứng trước
    uint32_t Period;     // chu kỳ (tick), 0 = one-shot
    uint32_t TaskID;
    uint8_t  Next;       // chỉ số task kế tiếp trong danh sách
    uint8_t  Used;
} sTask;

static sTask SCH_tasks_G[SCH_MAX_TASKS];
static uint8_t  head = NIL;
static uint32_t next_id = 1;
static volatile uint32_t sys_ticks = 0;

uint8_t Error_code_G = 0;          // 1 = hết chỗ, 2 = xóa task không tồn tại

/* ---------- Tiện ích ---------- */
uint32_t get_time(void) {
    return sys_ticks * TICK_MS;    // ms
}

/* Chèn task idx vào danh sách đã sắp xếp theo delta (gọi khi đã tắt ngắt) */
static void SCH_Insert(uint8_t idx, uint32_t delay) {
    uint8_t prev = NIL, cur = head;

    // dùng >= để task cùng thời điểm giữ thứ tự FIFO
    while (cur != NIL && delay >= SCH_tasks_G[cur].Delay) {
        delay -= SCH_tasks_G[cur].Delay;
        prev = cur;
        cur  = SCH_tasks_G[cur].Next;
    }
    SCH_tasks_G[idx].Delay = delay;
    SCH_tasks_G[idx].Next  = cur;
    if (cur != NIL) {
        SCH_tasks_G[cur].Delay -= delay;   // bù trừ delta cho task phía sau
    }
    if (prev == NIL) head = idx;
    else             SCH_tasks_G[prev].Next = idx;
}

/* ---------- API ---------- */
void SCH_Init(void) {
    for (uint8_t i = 0; i < SCH_MAX_TASKS; i++) {
        SCH_tasks_G[i].pTask  = 0;
        SCH_tasks_G[i].Used   = 0;
        SCH_tasks_G[i].Next   = NIL;
        SCH_tasks_G[i].Delay  = 0;
        SCH_tasks_G[i].Period = 0;
        SCH_tasks_G[i].TaskID = NO_TASK_ID;
    }
    head = NIL;
    next_id = 1;
    sys_ticks = 0;
    Error_code_G = 0;
}

/* O(1): chỉ đụng tới task đầu danh sách */
void SCH_Update(void) {
    sys_ticks++;
    if (head != NIL && SCH_tasks_G[head].Delay > 0) {
        SCH_tasks_G[head].Delay--;
    }
}

uint32_t SCH_Add_Task(void (*pFunction)(void), uint32_t DELAY, uint32_t PERIOD) {
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    uint8_t idx = 0;
    while (idx < SCH_MAX_TASKS && SCH_tasks_G[idx].Used) idx++;

    if (idx == SCH_MAX_TASKS) {          // hết chỗ
        Error_code_G = 1;
        __set_PRIMASK(primask);
        return NO_TASK_ID;
    }

    SCH_tasks_G[idx].pTask  = pFunction;
    SCH_tasks_G[idx].Period = PERIOD / TICK_MS;
    SCH_tasks_G[idx].TaskID = next_id++;
    SCH_tasks_G[idx].Used   = 1;
    SCH_Insert(idx, DELAY / TICK_MS);

    uint32_t id = SCH_tasks_G[idx].TaskID;
    __set_PRIMASK(primask);
    return id;
}

uint8_t SCH_Delete_Task(uint32_t taskID) {
    uint8_t ret = 0;
    if (taskID == NO_TASK_ID) return 0;

    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    uint8_t prev = NIL, cur = head;
    while (cur != NIL && SCH_tasks_G[cur].TaskID != taskID) {
        prev = cur;
        cur  = SCH_tasks_G[cur].Next;
    }
    if (cur == NIL) {
        Error_code_G = 2;                // không tìm thấy
    } else {
        uint8_t nxt = SCH_tasks_G[cur].Next;
        if (nxt != NIL) SCH_tasks_G[nxt].Delay += SCH_tasks_G[cur].Delay;
        if (prev == NIL) head = nxt;
        else             SCH_tasks_G[prev].Next = nxt;

        SCH_tasks_G[cur].Used   = 0;
        SCH_tasks_G[cur].pTask  = 0;
        SCH_tasks_G[cur].TaskID = NO_TASK_ID;
        ret = 1;
    }
    __set_PRIMASK(primask);
    return ret;
}

void SCH_Dispatch_Tasks(void) {
    while (1) {
        uint32_t primask = __get_PRIMASK();
        __disable_irq();

        if (head == NIL || SCH_tasks_G[head].Delay > 0) {
            __set_PRIMASK(primask);
            break;                       // chưa có task nào đến hạn
        }

        uint8_t idx = head;
        head = SCH_tasks_G[idx].Next;    // lấy task ra khỏi đầu danh sách
        void (*fn)(void) = SCH_tasks_G[idx].pTask;
        uint32_t period  = SCH_tasks_G[idx].Period;

        if (period > 0) {
            SCH_Insert(idx, period);     // task tuần hoàn: chèn lại
        } else {
            SCH_tasks_G[idx].Used   = 0; // one-shot: giải phóng
            SCH_tasks_G[idx].pTask  = 0;
            SCH_tasks_G[idx].TaskID = NO_TASK_ID;
        }
        __set_PRIMASK(primask);

        fn();                            // chạy task NGOÀI vùng tắt ngắt
    }
}
