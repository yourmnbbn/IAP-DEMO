#ifndef __IAP_H
#define	__IAP_H

#include "..\config.h"

/* 
 *  ����IAP������
 */                                     
#define     IAP_Prepare                                50               /* ѡ������                     */
                                                                        /* ����ʼ�����š����������š�   */                    
#define     IAP_RAMTOFLASH                             51               /* �������� FLASHĿ���ַ       */
                                                                        /* RAMԴ��ַ    ����д���ֽ���  */
                                                                        /* ϵͳʱ��Ƶ�ʡ�               */
#define     IAP_ERASESECTOR                            52               /* ��������    ����ʼ������     */
                                                                        /* ���������š�ϵͳʱ��Ƶ�ʡ�   */
#define     IAP_BLANKCHK                               53               /* �������    ����ʼ�����š�   */
                                                                        /* ���������š�                 */
#define     IAP_READPARTID                             54               /* ������ID    ���ޡ�           */
#define     IAP_BOOTCODEID                             55               /* ��Boot�汾�š��ޡ�           */
#define     IAP_COMPARE                                56               /* �Ƚ�����    ��Flash��ʼ��ַ  */
                                                                        /* RAM��ʼ��ַ����Ҫ�Ƚϵ�      */
                                                                        /* �ֽ�����                     */
/*
 *  ����IAP����״̬��
 */
#define     CMD_SUCCESS                                0
#define     INVALID_COMMAND                            1
#define     SRC_ADDR_ERROR                             2 
#define     DST_ADDR_ERROR                             3
#define     SRC_ADDR_NOT_MAPPED                        4
#define     DST_ADDR_NOT_MAPPED                        5
#define     COUNT_ERROR                                6
#define     INVALID_SECTOR                             7
#define     SECTOR_NOT_BLANK                           8
#define     SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION    9
#define     COMPARE_ERROR                              10
#define     BUSY                                       11

/* 
 *  ����CCLKֵ��С����λΪKHz 
 */
#define  IAP_FCCLK            (100000)
#define  IAP_ENTER_ADR        0x1FFF1FF1                                  /* IAP��ڵ�ַ����              */

extern void (*IAP_Entry) (INT32U param_tab[], INT32U result_tab[]);

/*********************************************************************************************************
** Function name:       sectorPrepare
** Descriptions:        IAP��������ѡ���������50
** input parameters:    sec1:           ��ʼ����
**                      sec2:           ��ֹ����
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ     
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ                     
*********************************************************************************************************/
extern INT32U  sectorPrepare (INT8U sec1, INT8U sec2);

/*********************************************************************************************************
** Function name:       ramCopy
** Descriptions:        ����RAM�����ݵ�FLASH���������51
** input parameters:    dst:            Ŀ���ַ����FLASH��ʼ��ַ����512�ֽ�Ϊ�ֽ�
**                      src:            Դ��ַ����RAM��ַ����ַ�����ֶ���
**                      no:             �����ֽڸ�����Ϊ512/1024/4096/8192
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ     
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ                     
*********************************************************************************************************/
extern INT32U  ramToFlash (INT32U dst, INT32U src, INT32U no);

/*********************************************************************************************************
** Function name:       sectorErase
** Descriptions:        �����������������52
** input parameters:    sec1            ��ʼ����
**                      sec2            ��ֹ����92
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ                     
*********************************************************************************************************/
extern INT32U  sectorErase (INT8U sec1, INT8U sec2);

/*********************************************************************************************************
** Function name:       blankChk
** Descriptions:        ������գ��������53
** input parameters:    sec1:           ��ʼ����
**                      sec2:           ��ֹ����92
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ                     
*********************************************************************************************************/
extern INT32U  blankChk (INT8U sec1, INT8U sec2);

/*********************************************************************************************************
** Function name:       parIdRead
** Descriptions:        ������գ��������54
** input parameters:    ��
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ                     
*********************************************************************************************************/
extern  INT32U  parIdRead (void);

/*********************************************************************************************************
** Function name:       codeIdBoot
** Descriptions:        ������գ��������55
** input parameters:    ��
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ                     
*********************************************************************************************************/
extern INT32U  codeIdBoot (void);

/*********************************************************************************************************
** Function name:       dataCompare
** Descriptions:        У�����ݣ��������56
** input parameters:    dst:            Ŀ���ַ����RAM/FLASH��ʼ��ַ����ַ�����ֶ���
**                      src:            Դ��ַ����FLASH/RAM��ַ����ַ�����ֶ���
**                      no:             �����ֽڸ����������ܱ�4����
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ                     
*********************************************************************************************************/
extern INT32U  dataCompare (INT32U dst, INT32U src, INT32U no);

#endif

