#include "IAP.h"

/*
 *  ���庯��ָ��  
 */
void (*IAP_Entry) (INT32U param_tab[], INT32U result_tab[]);

INT32U  paramin[8];                                                     /* IAP��ڲ���������            */
INT32U  paramout[8];                                                    /* IAP���ڲ���������            */

/*********************************************************************************************************
** Function name:       sectorPrepare
** Descriptions:        IAP��������ѡ���������50
** input parameters:    sec1:           ��ʼ����
**                      sec2:           ��ֹ����
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ     
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ                     
*********************************************************************************************************/
INT32U  sectorPrepare (INT8U sec1, INT8U sec2)
{  
    paramin[0] = IAP_Prepare;                                           /* ����������                   */
    paramin[1] = sec1;                                                  /* ���ò���                     */
    paramin[2] = sec2;                            
    (*IAP_Entry)(paramin, paramout);                                    /* ����IAP�������              */
   
    return (paramout[0]);                                               /* ����״̬��                   */
}

/*********************************************************************************************************
** Function name:       ramCopy
** Descriptions:        ����RAM�����ݵ�FLASH���������51
** input parameters:    dst:            Ŀ���ַ����FLASH��ʼ��ַ����512�ֽ�Ϊ�ֽ�
**                      src:            Դ��ַ����RAM��ַ����ַ�����ֶ���
**                      no:             �����ֽڸ�����Ϊ512/1024/4096/8192
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ     
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ                     
*********************************************************************************************************/
INT32U  ramToFlash (INT32U dst, INT32U src, INT32U no)
{  
    paramin[0] = IAP_RAMTOFLASH;                                        /* ����������                   */
    paramin[1] = dst;                                                   /* ���ò���                     */
    paramin[2] = src;
    paramin[3] = no;
    paramin[4] = IAP_FCCLK;
    (*IAP_Entry)(paramin, paramout);                                    /* ����IAP�������              */
    
    return (paramout[0]);                                               /* ����״̬��                   */
}

/*********************************************************************************************************
** Function name:       sectorErase
** Descriptions:        �����������������52
** input parameters:    sec1            ��ʼ����
**                      sec2            ��ֹ����92
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ                     
*********************************************************************************************************/
INT32U  sectorErase (INT8U sec1, INT8U sec2)
{  
    paramin[0] = IAP_ERASESECTOR;                                       /* ����������                   */
    paramin[1] = sec1;                                                  /* ���ò���                     */
    paramin[2] = sec2;
    paramin[3] = IAP_FCCLK;
    (*IAP_Entry)(paramin, paramout);                                    /* ����IAP�������              */
   
    return (paramout[0]);                                               /* ����״̬��                   */
}

/*********************************************************************************************************
** Function name:       blankChk
** Descriptions:        ������գ��������53
** input parameters:    sec1:           ��ʼ����
**                      sec2:           ��ֹ����92
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ                     
*********************************************************************************************************/
INT32U  blankChk (INT8U sec1, INT8U sec2)
{  
    paramin[0] = IAP_BLANKCHK;                                          /* ����������                   */
    paramin[1] = sec1;                                                  /* ���ò���                     */
    paramin[2] = sec2;
    (*IAP_Entry)(paramin, paramout);                                    /* ����IAP�������              */

    return (paramout[0]);                                               /* ����״̬��                   */
}

/*********************************************************************************************************
** Function name:       parIdRead
** Descriptions:        ������գ��������54
** input parameters:    ��
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ                     
*********************************************************************************************************/
INT32U  parIdRead (void)
{  
    paramin[0] = IAP_READPARTID;                                        /* ����������                   */
    (*IAP_Entry)(paramin, paramout);                                    /* ����IAP�������              */

    return (paramout[0]);                                               /* ����״̬��                   */
}

/*********************************************************************************************************
** Function name:       codeIdBoot
** Descriptions:        ������գ��������55
** input parameters:    ��
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ                     
*********************************************************************************************************/
INT32U  codeIdBoot (void)
{  
    paramin[0] = IAP_BOOTCODEID;                                        /* ����������                   */
    (*IAP_Entry)(paramin, paramout);                                    /* ����IAP�������              */

    return (paramout[0]);                                               /* ����״̬��                   */
}

/*********************************************************************************************************
** Function name:       dataCompare
** Descriptions:        У�����ݣ��������56
** input parameters:    dst:            Ŀ���ַ����RAM/FLASH��ʼ��ַ����ַ�����ֶ���
**                      src:            Դ��ַ����FLASH/RAM��ַ����ַ�����ֶ���
**                      no:             �����ֽڸ����������ܱ�4����
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ                     
*********************************************************************************************************/
INT32U  dataCompare (INT32U dst, INT32U src, INT32U no)
{  
    paramin[0] = IAP_COMPARE;                                           /* ����������                   */
    paramin[1] = dst;                                                   /* ���ò���                     */
    paramin[2] = src;
    paramin[3] = no;
    (*IAP_Entry)(paramin, paramout);                                    /* ����IAP�������              */

    return (paramout[0]);                                               /* ����״̬��                   */
}
