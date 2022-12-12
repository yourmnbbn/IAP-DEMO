# IAP基础框架
## 功能
- 实现BootLoader功能，并封装对flash的更新操作，后续开发可在此基础上进行，只需加入针对不同更新方式对应的数据传输协议即可。
- 实现多段程序储存在flash上，由BootLoader决定执行哪一段程序。
- 同时支持`ARM Compiler 5`和`ARM Compiler 6`编译器。

## IAP-BootLoader 程序结构
```
IAP-BootLoader
│
└───cfg_file    //启动文件
│   │...
│   │...
│   └───
└───ZY_CODE     //启动文件
│   │...
│   │...
│   └───
└───USER_CODE
    │   bootloader.c/h      //Bootloader 相关函数实现
    │   bootloadercfg.h     //Bootloader 配置头文件
    │   IAP.c/h             //IAP 接口封装
    │   main.c              //Bootloader程序入口所在
    └───
```

## bootloadercfg.h 配置说明
- `configCHIP_TOTAL_FLASH_SIZE`  
片上Flash总容量， 以KB为单位。  
- `configIAP_CONTROL_BLOCK_SIZE`  
BootLoader信息储存块的大小。  
- `configIAP_CONTROL_BLOCK_SECTOR`  
BootLoader信息储存块储存在Flash的哪一个扇区。  
- `configIAP_USER_APP_SECTOR`  
用户应用程序起始扇区。  
- `configCHIP_FLASH_SECTOR_SIZE`  
片上Flash扇区大小，以KB为单位。  
- `configUSER_CODE_START_ADDR`  
BootLoader程序起始地址。也是裸机用户程序的起始地址。  
- `configIAP_BOOTLOADER_CODE_SIZE`  
BootLoader和BootLoader信息储存块一共占用多少Flash空间，以KB为单位。  
- `configTEMP_RAM_ADDR`  
用来进行Flash写操作的临时RAM地址。  
- `configTEMP_RAM_SIZE`  
用来进行Flash写操作的临时RAM大小。  
- `configPROGRAM_MAX_SLOT`  
Flash上最多可以储存多少段用户程序（BootLoader除外）。  
- `configUSING_ASM_CONTEXT_SWITCH`  
是否用汇编实现用户程序跳转。  


## 部分关键实现细节
1. **用户程序跳转**  
此BootLoader里实现了两种用户程序跳转方式，第一种为汇编实现:
``` c
__asm void RunApplicationAtAddress(uint32_t address)
{
    //关闭优先级在HardFault以下的中断，在进行上下文切换时不希望有中断进行
    cpsid i
    
    //周立功的固件会在targetInit设置向量表重定向的值，但是标准库不会。所以为了兼容所有用户程序需要设置向量表重定向到用户程序
    ldr r1, =0xE000ED08
    str r0, [ r1 ]
    
    //向量表起始位置储存MSP的初始值，获取用户程序栈指针初始值
    ldr r1, [ r0 ]
    
    //将栈指针初始值赋给MSP，并等待上述指令与数据操作全部执行完毕
    msr msp, r1
    dsb
    isb
    
    //向量表第二项存放Reset handler也就是程序入口地址，设置PC寄存器来实现跳转， 至此BootLoader移交处理器控制权。
    ldr pc, [ r0, #4 ]
}
```
第二种为C实现：
``` c
void RunApplicationAtAddress(uint32_t address)
{
    //重定向向量表
    *(uint32_t*)0xE000ED08 = address;
    //向量表第二项存放Reset handler也就是程序入口地址，调用此函数永远不会返回， 至此BootLoader移交处理器控制权。
    (*(void (**)(void))(address + 4))();
}
```
第一种实现方式优于第二种，因为纯C无法设置MSP的值，栈里还存放着BootLoader的局部变量的信息以及部分函数调用信息，会减少用户程序可用栈空间大小。

2. **BootLoader信息储存块BCB（BootLoader Control Block）**  
BCB的定义如下：
``` c
//以下结构体以1字节的方式对齐
#pragma pack(1)
	typedef struct PorgramInfo_s
	{
		uint32_t 	start_addr;					//程序镜像储存在片上Flash中的起始地址
		uint32_t 	version;					//程序版本
		uint32_t 	size;						//程序镜像大小
		uint32_t 	crc32;						//程序镜像的CRC32
		uint8_t 	valid;						//这个程序槽是否有效？（是否是可以运行的程序）
		uint8_t 	run_this;					//BootLoader是否要运行这个程序？（一般来说只有一个槽的程序此成员置1， BootLoader会在程序表中找到此成员为1的程序运行）
	} ProgramInfo_t;

	//BCB(Bootloader control block) definition
	typedef struct BCB_s
	{
		uint32_t 	magic;							//BCB的magic number
		uint32_t 	version;						//BCB的版本
		uint8_t 	max_slots;					    //BootLoader最多允许flash上储存多少段可执行的程序。
		uint8_t 	update_required;		        //是否需要更新的标志
		
		ProgramInfo_t slot[configPROGRAM_MAX_SLOT];	//程序信息表
	} BCB_t;
#pragma pack()
```
BCB中储存BootLoader信息以及储存在片上Flash的程序信息，BCB也储存在片上Flash中的单独一块，紧挨着Bootloader程序所在扇区， 在用户程序所在扇区之前。整个Bootloader由BCB中的信息来控制，是否更新程序，运行哪一段程序等等。

3.**BootLoader 主函数**   
``` c
int main (void)
{
	BCB_t bcb;
	BCB_t* bcb_read = NULL;

	targetInit();
	pinInit(); 

    //使用Systick实现delay函数。由于ARM Compiler 5 和ARM Compiler 6 对于代码的优化算法不同，在v6中空循环实现的延时函数会被优化掉， 
    //故使用Systick来实现延迟，此延时更加精准。
	sysTickInit();
	
    //初始化open1768上的KEY1 KEY2按键
	FIO2DIR &= ~KEY1; 
	FIO2DIR &= ~KEY2; 
	
	//初始化IAP接口函数入口地址
	IAP_Entry = (void(*)())IAP_ENTER_ADR;
	
	//检查Flash中的BCB
	bcb_read = GetBootloaderControlBlock();
	if(bcb_read->magic != configBCB_MAGIC || bcb_read->version != BCB_VERSION)
	{
        //BCB版本不一样或目标位置未找到BCB, 初始化一个新的保存进去
		InitializeNewBCB(&bcb);
		SaveBootloaderControlBlock(&bcb);
	}
	
	//将Flash中的BCB信息映射到局部变量bcb里
	memcpy(&bcb, bcb_read, sizeof(BCB_t));
	
	//在OPEN1768上电复位时按住KEY1进入更新模式，此时LED4会一直闪烁
	if((FIO2PIN & KEY1) == 0)
	{
        //初始化串口
		uart0Init();
		
		for( ;; )
		{
			LED_Blink(1, 250);
			
			//若无更新，按住KEY2退出更新模式
			if((FIO2PIN & KEY2) == 0)
				break;
			
            //如果收到了完整的程序镜像，则写入Flash中
			if(g_RecvBinaryImage == 1)
			{
				g_RecvBinaryImage = 0;
				
				//擦除相应扇区并写入新程序
				EraseTargetSectors(bcb_read->slot[0].start_addr, g_RecvBinaryImage);
				WriteDataToFlash(bcb_read->slot[0].start_addr, configTEMP_RAM_ADDR, g_RecvLength);
				
				//将新的程序信息写入BCB, 保存到Flash中
				bcb.slot[0].size = g_RecvLength;
				bcb.slot[0].valid = 1;
				bcb.slot[0].run_this = 1;
				
				SaveBootloaderControlBlock(&bcb);
				break;
			}
		}
		
        //在中断控制器中清除UART0的信息
		zyIsrClr(NVIC_UART0);
	}
	
	//LED4快闪5次，表示此时进入了Bootloader并完成了更新
	LED_Blink(5, 100);
	
	//关闭Systick定时器
	sysTickShut();
	
	//Bootloader程序在此结束，程序永远不会返回。
	RunApplicationAtAddress(FindValidProgramEntry(&bcb));
}
```
此Bootloader更新方式：在目标板上电复位时按住KEY1进入更新模式，使用串口发送程序镜像文件，接收完毕后Bootloader会自动将接收到的程序镜像写入Flash中并更新BCB中的信息，然后运行此用户程序。

4. **更新Flash封装**  
```c 
void WriteDataToFlash(uint32_t flash_address, uint32_t ram_address, uint32_t length)
{
    //计算出此次操作flash的起始扇区
	uint8_t start_sector = flash_address / configCHIP_FLASH_SECTOR_SIZE;
    //计算出此次操作flash的终止扇区
	uint8_t end_sector = (flash_address + length) / configCHIP_FLASH_SECTOR_SIZE;
    //共需要操作多少个扇区
	uint8_t sector_count = end_sector - start_sector;
    //扇区迭代计数器
	uint8_t itor = 0;
	//最后一次写入操作时还剩多少字节数据没有写入
	uint32_t bytes_left = 0;
    //最后一次写入操作需要写入的字节数
	uint16_t last_write_size = 512;
    //最后一次写操作多写入的字节数
	uint16_t bytes_fill = 0;
    //每次写入的Flash的基地址
	uint32_t base_addr = 0;
	
    //禁用中断，操作flash时禁止访问flash， 使用的是内联汇编
    //#define DISABLE_ISR __asm("cpsid i")
	DISABLE_ISR;

    //遍历每一个需要操作的扇区
	for(itor = 0; itor < sector_count + 1; ++itor)
	{
        //计算写入的基地址
		base_addr = flash_address + itor * configCHIP_FLASH_SECTOR_SIZE;
		
		//最后一次写操作
		if(itor == sector_count)
		{
            //计算我们还有多少字节数据没有写入
			bytes_left = length - configCHIP_FLASH_SECTOR_SIZE * sector_count;
			
            //计算最后一次要写入多少个字节进入flash
			if(bytes_left > 1024)
				last_write_size = 4096;
			else if(bytes_left > 512)
				last_write_size = 1024;
			
			//向多写入的区域充填0xFF，这些地址的数据属于未定义的数据
			//由于进行了填充操作，在对临时RAM分片写入时要注意，不合适的分片方式会导致正常数据被填充覆盖！
			bytes_fill = last_write_size - bytes_left;
			memset((void*)(ram_address + bytes_left), 0xFF, bytes_fill);
			
			//准备Flash相应的扇区并写入数据
			sectorPrepare(start_sector, end_sector);
			ramToFlash(base_addr, ram_address + itor * configCHIP_FLASH_SECTOR_SIZE, last_write_size);
			break;
		}
		
        //准备Flash相应的扇区并写入数据
		sectorPrepare(start_sector, end_sector);
		ramToFlash(base_addr, ram_address + itor * configCHIP_FLASH_SECTOR_SIZE, configCHIP_FLASH_SECTOR_SIZE);
	}
	
    //启用中断，使用的是内联汇编
    //#define ENABLE_ISR  __asm("cpsie i")
	ENABLE_ISR;
}
```
更新flash函数封装了对flash的更新过程，上层开发只需在用各种方式收到数据后调用此函数即可完成数据写入。

## Keil项目设置
### IAP-BootLoader
1. 设置Options -> Target -> Read/Only Memory Areas中IROM1的Size设置成0xF000
2. 设置Options -> Debug -> Debugger Setting-> Flash Download中的Size设置成0xF000

在LPC1768的Flash中为BootLoader预留了28KB的空间(0x00000000-0x0000F000), 为BCB预留了4KB的空间(0x0000F000-0x00010000). 若目标板仅刷入了BootLoader程序则程序会一直反复运行bootloader， LED4会一直闪烁。

### IAP-UserApp-Example
1. 设置Options -> Target -> Read/Only Memory Areas中IROM1的Start设置成0x10000, Size设置成0x70000
2. 设置Options -> Debug -> Debugger Setting-> Flash Download中的Start设置成0x10000, Size设置成0x70000
3. 设置Options -> User -> After Build/Rebuild-> 勾选Run #1, User Command 设置成`fromelf.exe --bin --output=@L.bin !L`, 使得keil生成最小可执行程序.bin文件。串口传入的即为bin文件。
4. 设置IAP-UserApp-Example/cfg_file/Startup/InChip.sct 中的ROM_LOAD 0x00010000和VECTOR 0x00010000，使得链接器计算出正确的复位程序地址存入向量表中。

在Example中只设置了一个用户程序，故此程序可占用剩下的所有Flash空间。

## 测试
用户APP使用定时器中断来控制LED闪烁。提前编译两个用户程序，一个仅控制闪烁LED1和LED2, 另一个控制闪烁LED1-4。  
1. 将bootloader刷入目标板后进入更新模式，使用串口调试助手发送其中一个bin文件，写入后bootloader自动转入最新用户程序，可以看到两个LED闪烁。复位不进入更新模式bootloader执行之前刷入的程序，可以看到两个LED闪烁。
2. 再复位并进入更新模式，发送另一个bin文件，bootloader转入最新程序后可以看到四个LED闪烁。复位不进入更新模式bootloader执行之前刷入的程序，可以看到四个LED闪烁。

测试现象结合debug中寄存器以及flash中的数据的信息可知bootloader成功更新了Flash上的程序，并且成功的执行了Bootloader和用户程序上下文的跳转。