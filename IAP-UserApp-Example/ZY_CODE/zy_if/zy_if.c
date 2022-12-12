/****************************************Copyright (c)****************************************************
**                            Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                                 http://www.embedtools.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               zy_if.c
** Latest modified Date:    2009-07-23
** Latest Version:          1.00
** Descriptions:            用户编写的环境接口函数,必须在特权模式运行
**
**--------------------------------------------------------------------------------------------------------
** Created by:              Chenmingji
** Created date:            2009-07-23
** Version:                 1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Descriptions:
**
*********************************************************************************************************/
#include "..\..\config.h"
#include ".\zy_if.h"
#include "..\..\cfg_file\zy_if\zy_if_cfg.h"
#include <string.h>

/*********************************************************************************************************
** Function name:           zyIfInit
** Descriptions:            接口初始化
** input parameters:        none
** output parameters:       none
** Returned value:          zy_OK: 成功
**                          负数:  错误,绝对值参考zy_if.h
** Created by:              Chenmingji
** Created Date:            2009-07-23
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
*********************************************************************************************************/
INT32S zyIfInit (void)
{
    return ZY_OK;
}

/*********************************************************************************************************
** Function name:           zyReset
** Descriptions:            系统复位
** input parameters:        uiMode: ZY_POWER_RESET: 上电复位
**                                  ZY_HARD_RESET:  硬件复位
**                                  ZY_SOFT_RESET:  软件复位
**                                  其它:           与系统相关
** output parameters:       none
** Returned value:          none
** Created by:              Chenmingji
** Created Date:            2009-07-23
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
*********************************************************************************************************/
void zyReset (unsigned int uiMode)
{
    switch (uiMode) {

    case ZY_POWER_RESET:                                                /*  此系统上电复位等同硬件复位   */

#if 0
        break;
#endif                                                                  /*  0                           */

    case ZY_HARD_RESET:
        AITCR = (0x05fa << 16) + 4;
        break;

    case ZY_SOFT_RESET:
        AITCR = (0x05fa << 16) + 1;
        break;
    
    default:                                                            /*  参数不正确不复位            */
        break;
    }
}

/*********************************************************************************************************
** Function name:           zyHeapMalloc
** Descriptions:            堆分配内存
** input parameters:        ulSize: 内存大小
** output parameters:       none
** Returned value:          内存地址,NULL为不成功
** Created by:              Chenmingji
** Created Date:            2009-07-23
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
*********************************************************************************************************/
void *zyHeapMalloc (INT32U ulSize)
{
    void *pvRt = NULL;                                                  /*  返回值                      */

    zyIrqDisable();
    pvRt = malloc((size_t)ulSize);
    zyIrqEnable();
    return pvRt;
}

/*********************************************************************************************************
** Function name:           zyHeapFree
** Descriptions:            堆释放内存
** input parameters:        pvPrt: 要释放的内存
** output parameters:       none
** Returned value:          zy_OK: 成功
**                          负数:  错误,绝对值参考zy_if.h
** Created by:              Chenmingji
** Created Date:            2009-07-23
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
*********************************************************************************************************/
INT32S zyHeapFree (void *pvPrt)
{
    zyIrqDisable();
    free(pvPrt);
    zyIrqEnable();
    return ZY_OK;
}

/*********************************************************************************************************
** Function name:           zyIrqDisable
** Descriptions:            禁止中断
** input parameters:        none
** output parameters:       none
** Returned value:          zy_OK: 成功
**                          负数:  错误,绝对值参考zy_if.h
** Created by:              Chenmingji
** Created Date:            2009-07-23
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
*********************************************************************************************************/
__asm INT32S zyIrqDisable (void)
{
    CPSID   I
    MOV     R0, #0
    BX      LR
}

/*********************************************************************************************************
** Function name:           zyIrqEnable
** Descriptions:            允许中断
** input parameters:        none
** output parameters:       none
** Returned value:          zy_OK: 成功
**                          负数:  错误,绝对值参考.h
** Created by:              Chenmingji
** Created Date:            2009-07-23
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
*********************************************************************************************************/
__asm INT32S zyIrqEnable (void)
{
    CPSIE   I
    MOV     R0, #0
    BX      LR
}

/*********************************************************************************************************
** Function name:           zyIsrSet
** Descriptions:            IO系统设置中断服务程序
** input parameters:        uiChannel:  中断通道号
**                          ulFunction: 中断服务程序地址
**                          uiPrio:     中断优先级
** output parameters:       none
** Returned value:          zy_OK: 成功
**                          负数:  错误,绝对值参考zy_if.h
** Created by:              Chenmingji
** Created Date:            2009-07-23
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
*********************************************************************************************************/
INT32S zyIsrSet (unsigned int uiChannel, unsigned long ulFunction, unsigned int uiPrio)
{
    unsigned int uiTmp1, uiTmp2, uiTmp3;
    
    if (uiChannel > MAX_VICS) {
        return -ZY_NOT_OK;
    }

    zyIrqDisable();

    if (uiChannel >= 16) {
        uiTmp3 = uiChannel - 16;
        uiTmp1 = uiTmp3 / 32;
        uiTmp2 = uiTmp3 % 32;
   
        ((INT32U *)0xE000E100)[uiTmp1] = 1ul << uiTmp2;
        ((INT8U *)0xE000E400)[uiTmp3]  = uiPrio;
    } else {
        switch (uiChannel) {
        
        case MMI:
            SHCSR = SHCSR | (1 << 16);
            break;

        case BFI:
            SHCSR = SHCSR | (1 << 17);
            break;

        case UFI:
            SHCSR = SHCSR | (1 << 18);
            break;

        default:
            break;
        }
        if (uiChannel >= MMI) {
            uiTmp3 = uiChannel - MMI;
            ((INT8U *)0xE000ED18)[uiTmp3]  = uiPrio;
        }
    }

#if VECTOR_TABLE_IN_FLASH == 0
    ((unsigned long *)VTOR)[uiChannel] = ulFunction;
#endif
    zyIrqEnable();
    return ZY_OK;
}

/*********************************************************************************************************
** Function name:           zdIsrClr
** Descriptions:            IO系统清除中断服务程序
** input parameters:        uiChannel:  中断通道号
** output parameters:       none
** Returned value:          zy_OK: 成功
**                          负数:  错误,绝对值参考zy_if.h
** Created by:              Chenmingji
** Created Date:            2009-07-23
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
*********************************************************************************************************/
INT32S zyIsrClr (unsigned int uiChannel)
{
    unsigned int uiTmp1, uiTmp2, uiTmp3;
    
    if (uiChannel > MAX_VICS) {
        return -ZY_NOT_OK;
    }

    zyIrqDisable();
    
    if (uiChannel >= 16) {
        uiTmp3 = uiChannel - 16;
        uiTmp1 = uiTmp3 / 32;
        uiTmp2 = uiTmp3 % 32;
    
        ((INT32U *)0xE000E180)[uiTmp1] = 1ul << uiTmp2;
        ((INT8U *)0xE000E400)[uiTmp3]  = 0;

    } else {

        switch (uiChannel) {
        
        case MMI:
            SHCSR = SHCSR & ~(1 << 16);
            break;

        case BFI:
            SHCSR = SHCSR & ~(1 << 17);
            break;

        case UFI:
            SHCSR = SHCSR & ~(1 << 18);
            break;

        default:
            break;
        }
        if (uiChannel >= MMI) {
            uiTmp3 = uiChannel - MMI;
            ((INT8U *)0xE000ED18)[uiTmp3]  = 0;
        }
    }

#if VECTOR_TABLE_IN_FLASH == 0
    ((unsigned long *)VTOR)[uiChannel] = 0;
#endif
    zyIrqEnable();
    return ZY_OK;
}

/*********************************************************************************************************
** Function name:           zyIsrDisable
** Descriptions:            禁止指定中断
** input parameters:        uiChannel:  中断通道号
** output parameters:       none
** Returned value:          zy_OK: 成功
**                          负数:  错误,绝对值参考zy_if.h
** Created by:              Chenmingji
** Created Date:            2009-07-23
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
*********************************************************************************************************/
INT32S zyIsrDisable (unsigned int uiChannel)
{
    unsigned int uiTmp1, uiTmp2, uiTmp3;
    
    if (uiChannel > MAX_VICS) {
        return -ZY_NOT_OK;
    }
    
    if (uiChannel < 16) {
        return -ZY_NOT_OK;
    }

    zyIrqDisable();
    
    if (uiChannel >= 16) {
        uiTmp3 = uiChannel - 16;
        uiTmp1 = uiTmp3 / 32;
        uiTmp2 = uiTmp3 % 32;
    
        ((INT32U *)0xE000E180)[uiTmp1] = 1ul << uiTmp2;

    } else {

        switch (uiChannel) {
        
        case MMI:
            SHCSR = SHCSR & ~(1 << 16);
            break;

        case BFI:
            SHCSR = SHCSR & ~(1 << 17);
            break;

        case UFI:
            SHCSR = SHCSR & ~(1 << 18);
            break;

        default:
            break;
        }
    }
    zyIrqEnable();
    return ZY_OK;
}

/*********************************************************************************************************
** Function name:           zyIsrEnable
** Descriptions:            允许指定中断
** input parameters:        uiChannel:  中断通道号
** output parameters:       none
** Returned value:          ZY_OK: 成功
**                          负数:  错误,绝对值参考zy_if.h
** Created by:              Chenmingji
** Created Date:            2009-07-23
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
*********************************************************************************************************/
INT32S zyIsrEnable (unsigned int uiChannel)
{
    unsigned int uiTmp1, uiTmp2, uiTmp3;
    
    if (uiChannel > MAX_VICS) {
        return -ZY_NOT_OK;
    }

    zyIrqDisable();
    
    if (uiChannel >= 16) {
        uiTmp3 = uiChannel - 16;
        uiTmp1 = uiTmp3 / 32;
        uiTmp2 = uiTmp3 % 32;
        ((INT32U *)0xE000E100)[uiTmp1] = 1ul << uiTmp2;

    } else {

        switch (uiChannel) {
        
        case MMI:
            SHCSR = SHCSR | (1 << 16);
            break;

        case BFI:
            SHCSR = SHCSR | (1 << 17);
            break;

        case UFI:
            SHCSR = SHCSR | (1 << 18);
            break;

        default:
            break;
        }
    }
    zyIrqEnable();
    return ZY_OK;
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
