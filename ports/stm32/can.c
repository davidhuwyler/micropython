/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2018 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "py/objtuple.h"
#include "py/objarray.h"
#include "py/runtime.h"
#include "py/gc.h"
#include "py/binary.h"
#include "py/stream.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "bufhelper.h"
#include "can.h"
#include "irq.h"

#if MICROPY_HW_ENABLE_CAN && !(MICROPY_HW_ENABLE_FDCAN)

void can_init0(void) {
    for (uint i = 0; i < MP_ARRAY_SIZE(MP_STATE_PORT(pyb_can_obj_all)); i++) {
        MP_STATE_PORT(pyb_can_obj_all)[i] = NULL;
    }
}

void can_deinit_all(void) {
    for (int i = 0; i < MP_ARRAY_SIZE(MP_STATE_PORT(pyb_can_obj_all)); i++) {
        pyb_can_obj_t *can_obj = MP_STATE_PORT(pyb_can_obj_all)[i];
        if (can_obj != NULL) {
            can_deinit(can_obj);
        }
    }
}

// assumes Init parameters have been set up correctly
bool can_init(pyb_can_obj_t *can_obj) {
    CAN_TypeDef *CANx = NULL;
    uint32_t sce_irq = 0;
    const pin_obj_t *pins[2];

    switch (can_obj->can_id) {
        #if defined(MICROPY_HW_CAN1_TX)
        case PYB_CAN_1:
            CANx = CAN1;
            sce_irq = CAN1_SCE_IRQn;
            pins[0] = MICROPY_HW_CAN1_TX;
            pins[1] = MICROPY_HW_CAN1_RX;
            __HAL_RCC_CAN1_CLK_ENABLE();
            break;
        #endif

        #if defined(MICROPY_HW_CAN2_TX)
        case PYB_CAN_2:
            CANx = CAN2;
            sce_irq = CAN2_SCE_IRQn;
            pins[0] = MICROPY_HW_CAN2_TX;
            pins[1] = MICROPY_HW_CAN2_RX;
            __HAL_RCC_CAN1_CLK_ENABLE(); // CAN2 is a "slave" and needs CAN1 enabled as well
            __HAL_RCC_CAN2_CLK_ENABLE();
            break;
        #endif

        #if defined(MICROPY_HW_CAN3_TX)
        case PYB_CAN_3:
            CANx = CAN3;
            sce_irq = CAN3_SCE_IRQn;
            pins[0] = MICROPY_HW_CAN3_TX;
            pins[1] = MICROPY_HW_CAN3_RX;
            __HAL_RCC_CAN3_CLK_ENABLE(); // CAN3 is a "master" and doesn't need CAN1 enabled as well
            break;
        #endif

        default:
            return false;
    }

    // init GPIO
    uint32_t mode = MP_HAL_PIN_MODE_ALT;
    uint32_t pull = MP_HAL_PIN_PULL_UP;
    for (int i = 0; i < 2; i++) {
        if (!mp_hal_pin_config_alt(pins[i], mode, pull, AF_FN_CAN, can_obj->can_id)) {
            return false;
        }
    }

    // init CANx
    can_obj->can.Instance = CANx;
    HAL_CAN_Init(&can_obj->can);

    can_obj->is_enabled = true;
    can_obj->num_error_warning = 0;
    can_obj->num_error_passive = 0;
    can_obj->num_bus_off = 0;

    __HAL_CAN_ENABLE_IT(&can_obj->can, CAN_IT_ERR | CAN_IT_BOF | CAN_IT_EPV | CAN_IT_EWG);

    NVIC_SetPriority(sce_irq, IRQ_PRI_CAN);
    HAL_NVIC_EnableIRQ(sce_irq);

    return true;
}

void can_deinit(pyb_can_obj_t *self) {
    self->is_enabled = false;
    HAL_CAN_DeInit(&self->can);
    if (self->can.Instance == CAN1) {
        HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
        HAL_NVIC_DisableIRQ(CAN1_RX1_IRQn);
        HAL_NVIC_DisableIRQ(CAN1_SCE_IRQn);
        __HAL_RCC_CAN1_FORCE_RESET();
        __HAL_RCC_CAN1_RELEASE_RESET();
        __HAL_RCC_CAN1_CLK_DISABLE();
    #if defined(CAN2)
    } else if (self->can.Instance == CAN2) {
        HAL_NVIC_DisableIRQ(CAN2_RX0_IRQn);
        HAL_NVIC_DisableIRQ(CAN2_RX1_IRQn);
        HAL_NVIC_DisableIRQ(CAN2_SCE_IRQn);
        __HAL_RCC_CAN2_FORCE_RESET();
        __HAL_RCC_CAN2_RELEASE_RESET();
        __HAL_RCC_CAN2_CLK_DISABLE();
    #endif
    #if defined(CAN3)
    } else if (self->can.Instance == CAN3) {
        HAL_NVIC_DisableIRQ(CAN3_RX0_IRQn);
        HAL_NVIC_DisableIRQ(CAN3_RX1_IRQn);
        HAL_NVIC_DisableIRQ(CAN3_SCE_IRQn);
        __HAL_RCC_CAN3_FORCE_RESET();
        __HAL_RCC_CAN3_RELEASE_RESET();
        __HAL_RCC_CAN3_CLK_DISABLE();
    #endif
    }
}

void can_clearfilter(pyb_can_obj_t *self, uint32_t f, uint8_t bank) {
    CAN_FilterConfTypeDef filter;

    filter.FilterIdHigh         = 0;
    filter.FilterIdLow          = 0;
    filter.FilterMaskIdHigh     = 0;
    filter.FilterMaskIdLow      = 0;
    filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    filter.FilterNumber         = f;
    filter.FilterMode           = CAN_FILTERMODE_IDMASK;
    filter.FilterScale          = CAN_FILTERSCALE_16BIT;
    filter.FilterActivation     = DISABLE;
    filter.BankNumber           = bank;

    HAL_CAN_ConfigFilter(NULL, &filter);
}

int can_receive(CAN_HandleTypeDef *can, int fifo, CanRxMsgTypeDef *msg, uint8_t *data, uint32_t timeout_ms) {
    volatile uint32_t *rfr;
    if (fifo == CAN_FIFO0) {
        rfr = &can->Instance->RF0R;
    } else {
        rfr = &can->Instance->RF1R;
    }

    // Wait for a message to become available, with timeout
    uint32_t start = HAL_GetTick();
    while ((*rfr & 3) == 0) {
        MICROPY_EVENT_POLL_HOOK
        if (HAL_GetTick() - start >= timeout_ms) {
            return -MP_ETIMEDOUT;
        }
    }

    // Read message data
    CAN_FIFOMailBox_TypeDef *box = &can->Instance->sFIFOMailBox[fifo];
    msg->IDE = box->RIR & 4;
    if (msg->IDE == CAN_ID_STD) {
        msg->StdId = box->RIR >> 21;
    } else {
        msg->ExtId = box->RIR >> 3;
    }
    msg->RTR = box->RIR & 2;
    msg->DLC = box->RDTR & 0xf;
    msg->FMI = box->RDTR >> 8 & 0xff;
    uint32_t rdlr = box->RDLR;
    data[0] = rdlr;
    data[1] = rdlr >> 8;
    data[2] = rdlr >> 16;
    data[3] = rdlr >> 24;
    uint32_t rdhr = box->RDHR;
    data[4] = rdhr;
    data[5] = rdhr >> 8;
    data[6] = rdhr >> 16;
    data[7] = rdhr >> 24;

    // Release (free) message from FIFO
    *rfr |= CAN_RF0R_RFOM0;

    return 0; // success
}

// We have our own version of CAN transmit so we can handle Timeout=0 correctly.
HAL_StatusTypeDef CAN_Transmit(CAN_HandleTypeDef *hcan, uint32_t Timeout) {
    uint32_t transmitmailbox;
    uint32_t tickstart;
    uint32_t rqcpflag;
    uint32_t txokflag;

    // Check the parameters
    assert_param(IS_CAN_IDTYPE(hcan->pTxMsg->IDE));
    assert_param(IS_CAN_RTR(hcan->pTxMsg->RTR));
    assert_param(IS_CAN_DLC(hcan->pTxMsg->DLC));

    // Select one empty transmit mailbox
    if ((hcan->Instance->TSR&CAN_TSR_TME0) == CAN_TSR_TME0) {
        transmitmailbox = CAN_TXMAILBOX_0;
        rqcpflag = CAN_FLAG_RQCP0;
        txokflag = CAN_FLAG_TXOK0;
    } else if ((hcan->Instance->TSR&CAN_TSR_TME1) == CAN_TSR_TME1) {
        transmitmailbox = CAN_TXMAILBOX_1;
        rqcpflag = CAN_FLAG_RQCP1;
        txokflag = CAN_FLAG_TXOK1;
    } else if ((hcan->Instance->TSR&CAN_TSR_TME2) == CAN_TSR_TME2) {
        transmitmailbox = CAN_TXMAILBOX_2;
        rqcpflag = CAN_FLAG_RQCP2;
        txokflag = CAN_FLAG_TXOK2;
    } else {
        transmitmailbox = CAN_TXSTATUS_NOMAILBOX;
    }

    if (transmitmailbox != CAN_TXSTATUS_NOMAILBOX) {
        // Set up the Id
        hcan->Instance->sTxMailBox[transmitmailbox].TIR &= CAN_TI0R_TXRQ;
        if (hcan->pTxMsg->IDE == CAN_ID_STD) {
            assert_param(IS_CAN_STDID(hcan->pTxMsg->StdId));
            hcan->Instance->sTxMailBox[transmitmailbox].TIR |= ((hcan->pTxMsg->StdId << 21) | \
                                                        hcan->pTxMsg->RTR);
        } else {
            assert_param(IS_CAN_EXTID(hcan->pTxMsg->ExtId));
            hcan->Instance->sTxMailBox[transmitmailbox].TIR |= ((hcan->pTxMsg->ExtId << 3) | \
                                                        hcan->pTxMsg->IDE | \
                                                        hcan->pTxMsg->RTR);
        }

        // Set up the DLC
        hcan->pTxMsg->DLC &= (uint8_t)0x0000000F;
        hcan->Instance->sTxMailBox[transmitmailbox].TDTR &= (uint32_t)0xFFFFFFF0;
        hcan->Instance->sTxMailBox[transmitmailbox].TDTR |= hcan->pTxMsg->DLC;

        // Set up the data field
        hcan->Instance->sTxMailBox[transmitmailbox].TDLR = (((uint32_t)hcan->pTxMsg->Data[3] << 24) |
                                                ((uint32_t)hcan->pTxMsg->Data[2] << 16) |
                                                ((uint32_t)hcan->pTxMsg->Data[1] << 8) |
                                                ((uint32_t)hcan->pTxMsg->Data[0]));
        hcan->Instance->sTxMailBox[transmitmailbox].TDHR = (((uint32_t)hcan->pTxMsg->Data[7] << 24) |
                                                ((uint32_t)hcan->pTxMsg->Data[6] << 16) |
                                                ((uint32_t)hcan->pTxMsg->Data[5] << 8) |
                                                ((uint32_t)hcan->pTxMsg->Data[4]));
        // Request transmission
        hcan->Instance->sTxMailBox[transmitmailbox].TIR |= CAN_TI0R_TXRQ;

        if (Timeout == 0) {
            return HAL_OK;
        }

        // Get tick
        tickstart = HAL_GetTick();
        // Check End of transmission flag
        while (!(__HAL_CAN_TRANSMIT_STATUS(hcan, transmitmailbox))) {
            // Check for the Timeout
            if (Timeout != HAL_MAX_DELAY) {
                if ((HAL_GetTick() - tickstart) > Timeout) {
                    // When the timeout expires, we try to abort the transmission of the packet
                    __HAL_CAN_CANCEL_TRANSMIT(hcan, transmitmailbox);
                    while (!__HAL_CAN_GET_FLAG(hcan, rqcpflag)) {
                    }
                    if (__HAL_CAN_GET_FLAG(hcan, txokflag)) {
                        // The abort attempt failed and the message was sent properly
                        return HAL_OK;
                    } else {
                        return HAL_TIMEOUT;
                    }
                }
            }
        }
        return HAL_OK;
    } else {
        return HAL_BUSY;
    }
}

void can_rx_irq_handler(uint can_id, uint fifo_id) {
    mp_obj_t callback;
    pyb_can_obj_t *self;
    mp_obj_t irq_reason = MP_OBJ_NEW_SMALL_INT(0);
    byte *state;

    self = MP_STATE_PORT(pyb_can_obj_all)[can_id - 1];

    if (fifo_id == CAN_FIFO0) {
        callback = self->rxcallback0;
        state = &self->rx_state0;
    } else {
        callback = self->rxcallback1;
        state = &self->rx_state1;
    }

    switch (*state) {
        case RX_STATE_FIFO_EMPTY:
            __HAL_CAN_DISABLE_IT(&self->can,  (fifo_id == CAN_FIFO0) ? CAN_IT_FMP0 : CAN_IT_FMP1);
            irq_reason = MP_OBJ_NEW_SMALL_INT(0);
            *state = RX_STATE_MESSAGE_PENDING;
            break;
        case RX_STATE_MESSAGE_PENDING:
            __HAL_CAN_DISABLE_IT(&self->can, (fifo_id == CAN_FIFO0) ? CAN_IT_FF0 : CAN_IT_FF1);
            __HAL_CAN_CLEAR_FLAG(&self->can, (fifo_id == CAN_FIFO0) ? CAN_FLAG_FF0 : CAN_FLAG_FF1);
            irq_reason = MP_OBJ_NEW_SMALL_INT(1);
            *state = RX_STATE_FIFO_FULL;
            break;
        case RX_STATE_FIFO_FULL:
            __HAL_CAN_DISABLE_IT(&self->can, (fifo_id == CAN_FIFO0) ? CAN_IT_FOV0 : CAN_IT_FOV1);
            __HAL_CAN_CLEAR_FLAG(&self->can, (fifo_id == CAN_FIFO0) ? CAN_FLAG_FOV0 : CAN_FLAG_FOV1);
            irq_reason = MP_OBJ_NEW_SMALL_INT(2);
            *state = RX_STATE_FIFO_OVERFLOW;
            break;
        case RX_STATE_FIFO_OVERFLOW:
            // This should never happen
            break;
    }

    pyb_can_handle_callback(self, fifo_id, callback, irq_reason);
}

void can_sce_irq_handler(uint can_id) {
    pyb_can_obj_t *self = MP_STATE_PORT(pyb_can_obj_all)[can_id - 1];
    if (self) {
        self->can.Instance->MSR = CAN_MSR_ERRI;
        uint32_t esr = self->can.Instance->ESR;
        if (esr & CAN_ESR_BOFF) {
            ++self->num_bus_off;
        } else if (esr & CAN_ESR_EPVF) {
            ++self->num_error_passive;
        } else if (esr & CAN_ESR_EWGF) {
            ++self->num_error_warning;
        }
    }
}

#endif // MICROPY_HW_ENABLE_CAN
