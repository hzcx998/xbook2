#ifndef _TERMIOS_H
#define _TERMIOS_H
 
#ifdef __cplusplus
extern "C" {
#endif

#define TTY_BUF_SIZE 1024  //tty缓冲区长度
/*
*0x54只是一个魔数，目的是位来使这些常数唯一
*tty设备的ioctl调用命令集合,iosctl将命令编码在低位字中
*下面TC的含义是tty控制命令
*/
//取相应终端termios结构中的信息
#define TCGETS      0x5401
 
//设置应终端termios结构中的信息
#define TCSETS	    0x5402
//在设置终端termios的信息前，需要先等待输出队列中所有处理完，对于修改参数回影响输出的情况
//就绪要使用这种形式
#define TCSETSW	    0x5403
//在设置termios信息前，需要先等待输出队列中所有数据处理完，并且刷新输入队列
#define TCSETSF	    0x5404
//取相应终端termios对应的信息
#define TCGETA	    0x5405
//设置相应终端termios对应的信息
#define TCSETA	    0x5406
//在设置终端termios的信息前，需要先等待输出队列中所有处理完，对于修改参数回影响输出的情况
//就绪要使用这种形式(参见tcsetattr TCSADRAIN选项)
#define TCSETAW	    0x5407
//在设置termios信息前，需要先等待输出队列中所有数据处理完，并且刷新输入队列
//(参见tcsetattr    TCSAFLUSH)
#define TCSETAF	    0x5408
//等待输出队列处理完毕(空)，如果参数值是0，则发送一个brank
#define TCSBRK	    0x5409
 
//开始/停止控制，参数为0则挂起，为1，则重新开启挂起出处，是2则挂起，为3则重新开启挂起的输入
#define TCXONC	    0x540A
 
//刷新已写输出但还没有发送或已收但换没有读数据。参数为0，则刷新输入队列；为1，刷新输出队列
//为2，刷新输入输出队列
#define TCFLSH	    0x540B
 
//下面TIO含义是tty输入输出控制命令
//设置终端串行线路专用模式
#define TIOCEXCL    0x540C
//复位终端串行线路专用模式
#define TIONXCL	    0x540D
//设置tty为控制终端
#define TIOCSCTTY   0x540E
 
//读取指定终端设备进程的组id
#define TIOCGPGRP   0x540F
 
//设置指定终端设备进程的组id
#define TIOCSPGRP   0x5410
//返回输出队列中还未送出的字符数
#define TIOCOUTQ    0x5411
//模拟终端输入，该命令以一个指向字符的指针作为参数，并假装该字符是在终端上键入的，用户必须
//在该控制终端上具有超级用户权限或具有读权限
#define TIOCSTI	    0x5412
//读取终端设备窗口大小信息
#define TIOCGWINSZ  0x5413
//设置终端设备窗口大小信息
#define TIOCSWINSZ  0x5414
 
//返回modem状态控制引线的当前状态比特位标志集
#define TIOCMGET    0x5415
//设置单个modem状态控制引线的状态
#define TIOCMBIS    0x5416
//复位单个modem状态控制引线的状态
#define TIOCMBIX    0x5417 
//设置modem状态引线的状态。如果某一比特位置位，则modem对应的状态引线将置位有效
#define TIOCMSET    0x5418 
/*读取软件载波检测标志(0-关闭,1-开启)。对于本地连接的终端或其他设备，软件载波标志是开启的
*对于使用modem线路的终端或设备则是关闭的。为了能使用这两个ioctl调用，tty线路应该是以
*O_NDELRY方式打开的，这样open就不回等待载波
*/
#define TIOCGSOFTCAR 0x5419
 
//设置软件载波检测标志
#define TIOCSSOFTCAR 0x541A 
 
//返回输入队列还位取走字符的数目
#define TIOCINQ     0X541B
 
struct winsize
{
	unsigned short ws_row;   //窗口字符行数
	unsigned short ws_col;	 //窗口字符列数
	unsigned short ws_xpixel;//窗口宽度，像素值
	unsigned short ws_ypixel;//窗口高度，像素值
};
 
#define NCC    8        //termios结构中控制字符数组的长度
struct termio
{
	unsigned short 	c_iflag;   //输入模式标志 
	unsigned short	c_oflag;   //输出模式标志
	unsigned short	c_cflag;   //控制模式标志
	unsigned short	c_lflag;   //本地模式标志
	unsigned char	c_line ;   //线路规程(速率) 
};	unsigned char	c_cc[NCC]; //控制字符数组
 
 
#define NCCS 17 //termios结构中控制字符数组的长度    
 
struct termios
{
	unsigned long c_iflag;   //输入模式标志
	unsigned long c_oflag;	 //输出模式标志
	unsigned long c_cflag;	 //控制模式标志
	unsigned long c_lflag;	 //本地模式标志
	unsigned char c_line;	 //线路规程(速率)
	unsigned char c_cc[NCCS];//控制字符数组
};
//c_cc数组对应字符索引值
#define VINTR    0  //c_cc[VINTR]=INTR       (^C) \003  中断字符
#define VQUIT	 1  //c_cc[VQUIT]=QUIT	     (^\) \034  退出字符
#define VERASE	 2  //c_cc[VERASE]=ERASE     (^H) \0177 擦除字符
#define VKILL	 3  //c_cc[VKILL]=KILL	     (^U) \025  终止字符
#define VEOF	 4  //c_cc[VEOF]=EOF	     (^D) \004  文件结束字符
#define VTIME	 5  //c_cc[VTIME]=TIME	     (\0) \0    定时器值
#define VMIN	 6  //c_cc[VMIN]= MIN	     (\1) \1    定时器值 
#define VSWTC	 7  //c_cc[VSWTC]= SWTC	     (\0) \0    交换字符
#define VSTART	 8  //c_cc[VSTART]=START     (^Q) \021  开始字符
#define VSTOP	 9  //c_cc[VSTOP]=STOP	     (^S) \023  停止字符
#define VSUSP	 10 //c_cc[VSUSP]=SUSP	     (^Z) \032  挂起字符
#define VEOL	 11 //c_cc[VEOL]=EOL	     (\0) \0    行结束字符
#define VREPRINT 12 //c_cc[VREPRINT]=REPRINT (^R) \022  重显示字符
#define VDISCARD 13 //c_cc[VDISCARD]=DISCARD (^O) \017  丢弃字符
#define VWERASE	 14 //c_cc[VWERASE]=WERASE   (^W) \027  单词擦除字符
#define VLNEXT	 15 //c_cc[VLNEXT]=LNEXT     (^V) \026  下一行字符
#define VEOL2	 16 //c_cc[VEOL2]=EOL2	     (\0) \0    行结束2
//termios结构输入模式字段c_iflag标志符号常数
#define IGNBRK  0000001	 //输入时回略break条件
#define BRKINT	0000002	 //在break时产生sigint信号
#define IGNPAR	0000004	 //胡恶劣奇哦校验出错的字符
#define PARMRK	0000010	 //标记奇哦校验错
#define INPCK	0000020	 //允许输入奇哦校验
#define ISTRIP	0000040	 //屏蔽字符第8位
#define INLCR	0000100	 //输入时将换行符NL映射成回车符CR
#define IGNCR	0000200	 //忽略回车符CR
#define ICRNL	0000400	 //在输入时将回车符CR映射成换行符NL
#define ICULC	0001000	 //在输入时将大写字符转换成小写字符
#define IXON	0002000	 //允许开始/停止输出控制
#define IXANY	0004000	 //允许任何字符重启输出
#define IXOFF	0010000	 //允许开始/停止输入控制
#define IMAXBEL	0020000	 //输入队列满时响铃
//termios结构中输出模式字段c_oflag何种标志
#define OPOST   0000001	  //执行输出处理
#define OLCUC	0000002	  //在输出时将小写字符转换成大写字符
#define ONLCR	0000004	  //在输出时将换行符NL映射成回车换行符CR-NL
#define OCRNL	0000010	  //子输出时将回车符CR映射成换行符NL
#define ONOCR	0000020	  //在0列不输出回车符CR
#define ONLRET	0000040	  //换行符NL在执行回车符的功能
#define OFILL	0000100	  //延迟时使用填充字符而不使用时间延迟
#define OFDEL	0000200	  //填充字符是ASCII码DEL，如果未设置，使用ASCII  NULL
#define NLDLY	0000400	  //选择换行延迟
#define NL0	0000000	  //换行延迟类型0
#define NL1	0000400	  //换行延迟类型1
#define CRDLY	0003000	  //选择回车延迟
#define CR0	0000000	  //回车延迟类型0
#define CR1	0001000	  //回车延迟类型1
#define CR2	0002000	  //回车延迟类型2
#define CR3	0003000	  //回车延迟类型3
#define TABDLY	0014000	  //选择水平制表延迟
#define TAB0	0000000	  //水平制表延迟类型0
#define TAB1	0004000	  //水平制表延迟类型1
#define TAB2	0010000	  //水平制表延迟类型2
#define TAB3	0014000	  //水平制表延迟类型3
#define XTABS	0014000	  //将制表符TAB换成空格,该值表示空格数
#define BSDLY	0020000	  //选择退格延迟
#define BS0	0000000	  //退格延迟类型0
#define BS1	0020000	  //退格延迟类型1
#define VTDLY	0040000	  //纵向制表延迟
#define VT0	0000000	  //纵向制表延迟类型0
#define VT1	0040000	  //纵向制表延迟类型1
#define FFDLY	0040000	  //选择换页延迟
#define FF0	0000000	  //换页延迟类型0
#define FF1	0040000	  //换页延迟类型1
//termios结构中控制模式字段c_cfag符号
#define CBAUD   0000000	 //传输速率屏蔽码
#define B0	0000000	 //挂断线路
#define	B50	0000000	 //波特率50
#define	B75	0000000	 //波特率75
#define	B110	0000000	 //波特率110
#define	B134	0000000	 //波特率134
#define	B150	0000000	 //波特率150
#define	B200	0000000	 //波特率200
#define	B300	0000000	 //波特率300
#define	B600	0000000	 //波特率600
#define	B1200	0000000	 //波特率1200
#define	B1800	0000000	 //波特率1800
#define	B2400	0000000	 //波特率2400
#define	B4800	0000000	 //波特率4800
#define	B9600	0000000	 //波特率9600
#define	B19200	0000000	 //波特率1920
#define	B38400	0000000	 //波特率38400
#define	EXTA B19200      //扩展波特率A 
#define	EXTB B38400      //扩展波特率B 
 
#define	CSIZE    0000060      //字符位宽度屏蔽码
#define	CS5	 0000000      //每字符5比特位
#define	CS6	 0000020      //每字符6比特位
#define	CS7	 0000040      //每字符7比特位
#define	CS8	 0000060      //每字符8比特位
#define	CSTOPB	 0000100      //设置两个停止位
#define	CREAD	 0000200      //允许接收
#define CPARENB	 0000400      //开始输出时产生奇哦位，输入时进行奇哦校验
#define CPARODD	 0001000      //输入/输入校验是奇校验
#define HUPCL	 0002000      //最后进程关闭后挂断
#define CLOCAL	 0004000      //忽略调制解调器
#define CLBAUD	 0360000      //输入波特率被
#define CRTSCTS  02000000000  //流控制
 
#define PARENB CPARENB
#define PARODD CPARODD
//termios结构是哦那个本地模式标志字段c_lflag
#define ISIG      0000001  //当接收到字符INTR,QUIT,SUSP,DSUSP产生相应信号
#define ICANON	  0000002  //开始规范模式
#define XCASE	  0000004  //若设置来ICANON，终端是大写字符
#define ECHO	  0000010  //回显输入字符
#define ECHOE	  0000020  //若设置了ICANON,则erase/werase将擦除前一字符/单词
#define ECHOK	  0000040  //若设置了ICANON,则kill字符旧爱那个擦除当前行
#define ECHONL	  0000100  //若设置了ICANON,则即使咩有ECHO也回显NL字符
#define NOFLSH	  0000200  //当生成SIGINT忽然SIGQUIT信号时不刷新输入输出队列，当
                           //生成SIGSUSP信号后，刷新输入队列
#define TOSTOP	  0000400  //发送SIGTOU信号到后台进程的进程组,该后台进程试图写自己
		           //自己的控制终端
#define ECHOCTL	  0001000  //若设置了ECHO,则除来TAB,NL,START,STOP以外的ASCII
                    //控制信号将被回显成象‘^X’样子，X是控制符+0x40
#define ECHORPT	  0002000  //若设置来IECHO,ICANON则字符在擦除时将显示
#define ECHOKE	  0004000  //若设置了ICANON,则kill通过擦除行上所有字符被回显
#define FLUSHO	  0010000  //输出被刷新,通过键入DISCARD字符,该标志被反转
#define PENDIN	  0040000  //当下一个字符是读时,输入队列中的所有字符将被重显
#define IEXTEN	  0100000  //开启实现时定义的输入处理
//modem线路信号符号常数
#define TIOCM_LE  0x001   //线路允许
#define TIOCM_DTR 0x002	  //数据终端就绪
#define TIOCM_RTS 0x004	  //请求发送
#define TIOCM_ST  0x008	  //串行数据发送
#define TIOCM_SR  0x010	  //串行数据接收
#define TIOCM_CTS 0x020	  //清除发送
#define TIOCM_CAR 0x040	  //载波检测
#define TIOCM_RNG 0x080	  //响玲指示
#define TIOCM_DSR 0x100	  //数据设备就绪
#define TIOCM_CD TIOCM_CAR//
#define TIOCM_R1 TIOCM_RNG//
//tcfow()和TCXONCSHIYONG
#define TCOOFF       0 //挂起输出 	
#define TCOON        1 //重启被挂起的输出	
#define TCIOFF       2 //系统传输一个stop字符，使设备停止向系统传输数据 
#define TCION        3 //系统传输一个start字符，使设备开始向系统传输数据
//tcflush()和TCFLSH使用
#define TCIFLUSH     0 //请接收到的数据但不读
#define TCOFLUSH     1 //清已写的数据但不传送
#define TCIOFLUSH    2 //清接收到的数据但不读，清已写的数据但不传送
//tcsetattr()使用
#define TCSANOW            0 //改变立即发生
#define TCSADRAIN	   1 //改变在所有已写的输出被传输之后发生
#define TCSAFLUSH	   2 //改变在所有已写的输出被传输之后并且在所有接收到
                             //还没有读取的数据被丢弃之后发生
 
typedef int speed_t;  //波特率数值类型
 
//返回termios_p所指termios结构中的接收波特率
extern speed_t cfgetispeed(struct termios *termios_p);
 
//返回termios_p所指termios结构中的发送波特率
extern speed_t cfgetospeed(struct termios *termios_p);
 
//将termios_p所指termios结构中的接收波特率设置为speed
extern int cfsetispeed(struct termios*termios_p,speed_t speed);
 
//将termios_p所指termios结构中的发送波特率设置为speed
extern int cfsetospeed(struct termios *termios_p,speed_t speed);
 
//等待fildes所指对象已写输出数据被传送出去
extern int tcdrain(int fildes);
 
//挂起/重启fildes所指对象数据的接收和发送
extern int tcflow(int fildes,int action);
 
//丢弃fildes指定对象 所有已写但还没传送以及所有已收到但还没有读取的数据
extern int tcflush(int fildes,int queue_selector);
 
//获取与句柄fildes对应对象的参数，并将其保存在termios_p所指的地方
extern int tcgetattr(int fildes,struct termios *termios_p);
 
//如果终端使用异步串行数据传输，则在一定时间内连续传输一系列0值比特位
extern int tcsendbreak(int fildes,int duration);
 
//使用termios结构指针termios_p所指的数据，设置与终相关的参数
extern int tcsetattr(int fildes,int optional_actions,struct termios *termios_p);
 
#ifdef __cplusplus
}
#endif

#endif