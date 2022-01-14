// Debug file

#ifndef H_DEBUG
#define H_DEBUG

// Define of dbg Macro

// Timer 70 ISR
// TIMER 70 start at 0x10 end at 0x1f
#define TIMER_70_ISR_BEGIN 0x0
#define TIMER_70_ISR_END 0x1
#define TIMER_70_QUEUE_SR_BEGIN 0x2
#define TIMER_70_QUEUE_SR_END 0x3

// TIMER 500 start at 0x20 end at 0x2f
// Timer 500 ISR
#define TIMER_500_ISR_BEGIN 0x4
#define TIMER_500_ISR_END 0x5
#define TIMER_500_QUEUE_SR_BEGIN 0x6
#define TIMER_500_QUEUE_SR_END 0x7

// Sensor Thread
#define SENSOR_TASK_ENTER 0x8
#define SENSOR_TASK_ENTER_WHILE 0x9
#define SENSOR_TASK_SR_QUEUE_BEGIN 0xa
#define SENSOR_TASK_SR_QUEUE_END 0xb

// UART Thread
#define UART_TASK_ENTER 0xc
#define UART_TASK_ENTER_WHILE 0xd
#define UART_TASK_SR_QUEUE_BEGIN 0xe
#define UART_TASK_SR_QUEUE_END 0xf

// UART init
#define ERR_CREATE_UART 0x10

// Timer init
#define ERR_CREATE_TIMER 0x11
#define ERR_START_TIMER 0x12

// ADC init
#define ERR_START_ADC 0x13

// Sensor task queue
#define ERR_SENSOR_TASK_QUEUE_CREATE 0x14

// Sensor FSM
#define ERR_FSM_SWITCH_OUT_OF_RANGE 0x15
#define ERR_SENSOR_MSG_TYPE_OUT_OF_RANGE 0x16

// Uart thread queue
#define ERR_UART_TASK_QUEUE_CREATE 0x17
#define ERR_UART_SEND_MSG 0x18
#define WARN_UART_QUEUE_FULL 0x19

// UART write and read
#define UART_START_WRITE 0x1a
#define UART_END_WRITE 0x1b

#define ERR_SENSOR_TASK_QUEUE_MSG 0x1c

#define SENSOR_FSM_INIT 0x1d
#define SENSOR_FSM_AVERAGE 0x1e

#define ERR_MQTT_TASK_QUEUE_CREATE 0x1f

void initDbg();
void dbgEvent(unsigned int event);
void fatalError(unsigned int event);

#endif
