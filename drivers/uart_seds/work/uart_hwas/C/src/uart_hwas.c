#include "uart_hwas.h"
#include "SAMV71-Runtime/Hal/Hal.h"

#include <Nvic/Nvic.h>

#include <string.h>
#include <assert.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

static volatile bool interruptSubscribe[Nvic_InterruptCount] = { 0 };

#define HWAS_INTERRUPT_QUEUE_SIZE 100
#define HWAS_INTERRUPT_QUEUE_ITEM_SIZE (sizeof(asn1SccInterruptNumber))
#define HWAS_INTERRUPT_STACK_SIZE 8192
#define HWAS_INTERRUPT_PRIORITY 1

#define PERIPH_INTERRUPT_PRIORITY 1

__attribute__((section(".sdramMemorySection"))) static volatile QueueHandle_t hwasInterruptQueueHandle;
__attribute__((section(".sdramMemorySection"))) static uint8_t
        hwasInterruptQueueStorageBuffer[HWAS_INTERRUPT_QUEUE_SIZE * HWAS_INTERRUPT_QUEUE_ITEM_SIZE];
__attribute__((section(".sdramMemorySection"))) static StaticQueue_t hwasInterruptQueueBuffer;

__attribute__((section(".sdramMemorySection"))) static StackType_t hwasTaskStackBuffer[HWAS_INTERRUPT_STACK_SIZE];
__attribute__((section(".sdramMemorySection"))) static StaticTask_t hwasTaskBuffer;

void
uart_HwasHandleInterrupt(asn1SccInterruptNumber* irq)
{
    xQueueSendFromISR(hwasInterruptQueueHandle, irq, NULL);
    uart_hwas_PI_InterruptManagement_DisableInterrupt_Pi(irq);
    portEND_SWITCHING_ISR(true);
}

void
uart_HwasUART0_Handler(void* args)
{
    (void)args;
    asn1SccInterruptNumber irq = Nvic_Irq_Uart0;
    uart_HwasHandleInterrupt(&irq);
}

void
uart_HwasUART1_Handler(void* args)
{
    (void)args;
    asn1SccInterruptNumber irq = Nvic_Irq_Uart1;
    uart_HwasHandleInterrupt(&irq);
}

void
uart_HwasUART2_Handler(void* args)
{
    (void)args;
    asn1SccInterruptNumber irq = Nvic_Irq_Uart2;
    uart_HwasHandleInterrupt(&irq);
}

void
uart_HwasUART3_Handler(void* args)
{
    (void)args;
    asn1SccInterruptNumber irq = Nvic_Irq_Uart3;
    uart_HwasHandleInterrupt(&irq);
}

void
uart_HwasUART4_Handler(void* args)
{
    (void)args;
    asn1SccInterruptNumber irq = Nvic_Irq_Uart4;
    uart_HwasHandleInterrupt(&irq);
}

void
uart_HwasInterruptHandlerTask(void* args)
{
    (void)args;
    while(true) {
        asn1SccInterruptNumber value;
        if(xQueueReceive(hwasInterruptQueueHandle, &value, portMAX_DELAY) == pdTRUE) {
            uart_hwas_RI_InterruptSubscription_Interrupt_Ri(&value);
            taskYIELD();
        } else {
            assert(false && "Error while reading the queue");
            break;
        }
    }
}

void
uart_hwas_startup(void)
{
    memset((void*)interruptSubscribe, 0, Nvic_InterruptCount);
    hwasInterruptQueueHandle = xQueueCreateStatic(HWAS_INTERRUPT_QUEUE_SIZE,
                                                  HWAS_INTERRUPT_QUEUE_ITEM_SIZE,
                                                  hwasInterruptQueueStorageBuffer,
                                                  &hwasInterruptQueueBuffer);
    assert(hwasInterruptQueueHandle != NULL);
    TaskHandle_t taskHandle = xTaskCreateStatic(uart_HwasInterruptHandlerTask,
                                                "uart_HwasInterruptHandlerTask",
                                                HWAS_INTERRUPT_STACK_SIZE,
                                                NULL,
                                                HWAS_INTERRUPT_PRIORITY,
                                                hwasTaskStackBuffer,
                                                &hwasTaskBuffer);
    assert(taskHandle != NULL);
}

void
uart_hwas_PI_InterruptManagement_DisableInterrupt_Pi(const asn1SccInterruptNumber* IN_interrupt)
{
    Nvic_Irq irqNumber = (Nvic_Irq)*IN_interrupt;
    switch(irqNumber) {
        case Nvic_Irq_Usart1:
        case Nvic_Irq_Xdmac:
            break;
        default:
            Nvic_disableInterrupt(irqNumber);
    }
}

void
uart_hwas_PI_InterruptManagement_EnableInterrupt_Pi(const asn1SccInterruptNumber* IN_interrupt)
{
    Nvic_Irq irqNumber = (Nvic_Irq)*IN_interrupt;
    switch(irqNumber) {
        case Nvic_Irq_Usart1:
        case Nvic_Irq_Xdmac:
            break;
        default:
            Nvic_setInterruptPriority(irqNumber, PERIPH_INTERRUPT_PRIORITY);
            Nvic_enableInterrupt(irqNumber);
    }
}

void
uart_hwas_PI_InterruptSubscriptionManagement_SubscribeToInterrupt_Pi(const asn1SccInterruptNumber* IN_interrupt)
{
    interruptSubscribe[*IN_interrupt] = true;
    switch((Nvic_Irq)*IN_interrupt) {
        case Nvic_Irq_Uart0:
            Hal_subscribe_to_interrupt(Nvic_Irq_Uart0, uart_HwasUART0_Handler);
            break;
        case Nvic_Irq_Uart1:
            Hal_subscribe_to_interrupt(Nvic_Irq_Uart1, uart_HwasUART1_Handler);
            break;
        case Nvic_Irq_Uart2:
            Hal_subscribe_to_interrupt(Nvic_Irq_Uart2, uart_HwasUART2_Handler);
            break;
        case Nvic_Irq_Uart3:
            Hal_subscribe_to_interrupt(Nvic_Irq_Uart3, uart_HwasUART3_Handler);
            break;
        case Nvic_Irq_Uart4:
            Hal_subscribe_to_interrupt(Nvic_Irq_Uart4, uart_HwasUART4_Handler);
            break;
    }
}

void
uart_hwas_PI_RawMemoryAccess_ReadWord_Pi(const asn1SccSourceAddress* IN_address,
                                    const asn1SccWordMask* IN_mask,
                                    asn1SccWord* OUT_value)
{
    *OUT_value = 0;
    uint32_t* address = (uint32_t*)((uint32_t)*IN_address);
    uint32_t maskValue = (uint32_t)*IN_mask;
    uint32_t* addressOut = (uint32_t*)OUT_value;
    *addressOut = maskValue & *address;
}

void
uart_hwas_PI_RawMemoryAccess_WriteWord_Pi(const asn1SccDestinationAddress* IN_address,
                                     const asn1SccWordMask* IN_mask,
                                     const asn1SccWord* IN_value)
{
    uint32_t* address = (uint32_t*)((uint32_t)*IN_address);
    uint32_t regValue = (uint32_t)*address;
    uint32_t mask = (uint32_t)*IN_mask;
    uint32_t valueToSet = (uint32_t)*IN_value;
    regValue &= ~(mask);
    regValue |= (mask & valueToSet);

    *address = regValue;
}

void
uart_hwas_PI_RawMemoryAccess_ExclusiveReadWord_Pi(const asn1SccSourceAddress* IN_address,
                                             const asn1SccWordMask* IN_mask,
                                             asn1SccWord* OUT_value)
{
    volatile uint32_t memVal;
    uint32_t maskValue = (uint32_t)*IN_mask;
    volatile uint32_t* address = (uint32_t*)((uint32_t)*IN_address);

    /// Read
    __asm volatile("   ldrex  %[memVal],   [%[address]]  \n\r"
                   : [memVal] "=&r"(memVal)
                   : [address] "r"(address)
                   : "memory");

    uint32_t* addressOut = (uint32_t*)OUT_value;
    *addressOut = maskValue & memVal;
}

void
uart_hwas_PI_RawMemoryAccess_ExclusiveWriteWord_Pi(const asn1SccDestinationAddress* IN_address,
                                              const asn1SccWordMask* IN_mask,
                                              const asn1SccWord* IN_value,
                                              asn1SccByte* OUT_status)
{
    volatile uint32_t* address = (uint32_t*)((uint32_t)*IN_address);
    volatile uint32_t newValue = ((uint32_t)*IN_value) & ((uint32_t)*IN_mask);
    volatile uint32_t result = 1;
    /// Write
    __asm volatile("    strex   %[result],  %[newValue], [%[address]] \n\r"
                   "    dmb"
                   : [result] "=&r"(result)
                   : [newValue] "r"(newValue), [address] "r"(address)
                   : "memory");
    *OUT_status = (asn1SccByte)result;
}


