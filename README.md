# MELOXOS
MY FIRST OS
# instructions
我的第一个操作系统内核MELOX今天就收工啦 <br />
从7月开始每天出差9点回到酒店看书 <br />
8月出差太累生病住院看书和视频 <br />
9月开始在滨海出差期间每天晚上10点后回到酒店实操 <br />
到现在只能说先这样吧，毕竟后面还有毕业论文，找工作，项目出差 <br />
目前以grub启动，支持分页与虚拟空间，8259a中断，堆管理器malloc，采用中断方式实现多进程，实现系统调用fork，psk2键盘，时钟，pci协议，磁盘等 <br />
当然还是有很多不足，最重要的是还没实现文件系统，在这个毕业加找工作加项目完结的节点实在没有时间和精力了。 <br />
中断用的老古董pic而不是apic，键盘支持扫描码不完善，堆管理器用的最简单的隐式链表，多进程的内核与用户态空间没有区分，磁盘没有用sata而是ata，系统调用还有很多没实现等许多问题。 <br />
到此先告一段落，后面会考虑重构，春节可能会考虑文件系统(不忙的话)。下面是几个不可能的目标🤡 <br />
远期目标1:实现文件系统 <br />
远期目标2:实现ls,cd等，可以装gcc编译在此内核写的程序 <br />
远期目标3:图形化 <br />
# FUNCTION PICTURES
![image](https://github.com/JayZhouReverseClock/MELOXOS/blob/hd_blk/pictures/JVW%7B98XRURZ%24F%60704_Q.png)
![image](https://github.com/JayZhouReverseClock/MELOXOS/blob/hd_blk/pictures/J_O0TTDCZDOD_94L3%5D%2551JM.png)
![image](https://github.com/JayZhouReverseClock/MELOXOS/blob/hd_blk/pictures/%7D%5DVHLZKHCHTW3S535XJ%7D_0W.png)
![image](https://github.com/JayZhouReverseClock/MELOXOS/blob/hd_blk/pictures/MVUFFUX%40G766M8%5DSMGR_4P.png)
![image](https://github.com/JayZhouReverseClock/MELOXOS/blob/hd_blk/pictures/VHLZKHCHTW3S535XJ_0W.png)
![image](https://github.com/JayZhouReverseClock/MELOXOS/blob/hd_blk/pictures/ZGHOR%25YK82U8VBQSGW.png)


