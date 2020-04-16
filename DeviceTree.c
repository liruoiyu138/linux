一.绪论
1.Device Tree 是什么？
	Device Tree可以描述的信息包括CPU的数量和类型, 内存基地址大小，总线和桥,外设连接
中断控制器,和中断使用情况,GPIO控制器和使用情况
2.设备数信息保存在一个ASCII文本文件中,适合人类的阅读习惯
3.设备数是一种数据结构,用于描述设备信息的语言,用于操作系统中描述硬件,使得不需要对设备的信息进行硬编码（hard code）
在 arch/arm/boot/dts 文件中
4.Device Tree 由一系列被命名的节点和属性构成,节点本身可以包含子节点
5.设备数源文件dts被编译成dtb二进制文件,在bootloader运行时传递给操作系统

二.由来
1.在Linux2.6中, arch/arm/plat-xxx arch/arm/math-xxx中充斥着大量描述硬件细节的代码,可移植性很差,这些代码对内核来说是垃圾代码
2.在3.1中 Device Tree作为一个独立的文件，将硬件的配置抽象出来

三.编译以及使用 -- DTC (device tree compiler)
1.DTC是将 .dts 编译成 .dtb 的工具
2.DTC的源代码位于内核的script/dtc目录,在Linux内核使能了Device Tree的情况下,编译内核时主机工具dtc会被编译出来
3.在Linux内核的arch/arm/boot/dts/Makefile 中,描述了当某种Soc被选中后,哪些.dtb文件会被编译出来
eg:  dtb-#(CONFIG_ARCH_EXYNOS) += exynos4210-origen.dtb
4.编译过程
	1）.参考板origin的设备文件为参考
		cp arch/arm/boot/dts/exynos4412-origen.dts arch/arm/boot/dts/exynos4412-fs4412.dts
	2）.修改Makefile
		vim arch/arm/boot/dts/Makefile，在exynos4412-origin.dtb \ 中添加
		exynos4412-fs4412.dtb
	3）.编译设备树 （将 dts 编译成 dtb）
		make dtbs
	4）.拷贝内核设备树文件到/tftpboot目录下
		cp arch/arm/boot/dts/exynos4412-fs4412.dtb /tftpboot
	5）.设置启动参数(加载内核到内存的0x41000000 加载dtb文件到0x42000000)
		set bootcmd tftp 0x41000000 uImage \; tftp 0x42000000 exynos4412-fs4412.dtb \;bootm 0x41000000 - 0x42000000

四. dtb文件的使用过程
1.展开dtb文件(树状结构) allnodes => 子节点(也可以包含子节点)
2.内核会将dtb文件展开成结构体,从而产生一个硬件设备的拓扑图,在编程过程中可以直接通过系统提供的接口获取到设备树
中节点的属性和信息
3. 名词解释
	DT: Device Tree
	FDT : Flattened Device Tree
	OF : Open Firmware
	DTS : device Tree source
	DTSI : device tree source include
	DTB : device Tree blob
	DTC : device tree compiler
五. 语法
1. 实例
eg: 模版
/{  /* 根节点 */

	node1 { /*  子节点1 */
		a-string-property = "A tring"; /* 键-值对 属性*/
		child-node1 { /* 子子节点 */

		};
		child-node2 {

		};
	};

	node2 {
		a-cell-property = <1 2 3 4>; /* 32位整数数组 */
	};
}；

eg: arm/boot/dts/exynos4412-fs4412.dts

#incldue "exynos4412.dtsi"  /* device tree source include */

/{
	model = "xxxx";
	compiler = "xxxx";

	memory {
		reg = <0x40000000 0x40000000>;
	};

	firmware@0x03f000 {
		compatible = "samsung,secure-firmware";
		reg = <0x0203f000 0x1000>;
	};
}

2.节点node
	.每个节点必须有一个 "<名称>[@<设备地址>]" 形式的名字
	<名称> 是一个不超过31位的简单ascii字符串
	<设备地址> 用来访问该设备的主地址,并且该地址也在节点的reg属性中列出
	树中的每一个表示一个设备的节点都需要一个compatible属性
3.属性
	1）.文本字符串(无结束符)
		string-property = "a tring"
	2）.Cells是32位无符号整数,用尖括号表示
		cell-property = <0xbeef 123 0xabcd1234>
	3）.二进制数据用方括号限制
		binary-property = [01 23 45 67]
	4）.不同形式的数据可以使用逗号连在一起：
		mixed-property = "a tring",[01 23 45 67], <0xbeef 123 0xabcd1234>
	5）.逗号可用于创建字符串列表
		string-list = "red fish","blue fish"
六. 属性
1. compatible 属性
	实际在代码中可以用于匹配,它包含一个 "<制造商>,<型号>" 形式的字符串。
eg: 
	/{
			compatible = "acme,coyoted-revenge";
	};
2. #address-cells 和 #size-cells
	#address-cells = <2> 表示address字段的长度为1
	#size-cells = <1>； 表示length字段的长度为1
	
	eg:  ··· reg = <0 0 0x1000> ··· 地址占两个cells，长度占一个cells

3. reg = <address1 length1 [address2 length2] [address3 length3]>
	eg：
/{
	compatible = "acme,coyoted-revenge";
	#address-cells = <1>;
	#size-cells = <1>；
	serial@101f2000 {
		compatible = "arm,pl011";
		reg = <0x101f2000 0x1000>;
		interrupts = < 2 0 >;
	};
};
4. interrupts
	描述中断连接需要四个属性
	1）.interrupt-controller (设备节点属性) - 一个空的属性定义该节点作为一个接受中断信号的设备
	2）.#interrupt-cells(中断控制器节点属性) - 表明该中断控制器的中断指示符中cell的个数
	3）.interrupt-parent(设备节点属性) 包含一个指向该设备连接的中断控制器的phandle,那些没有 interrupt-parent
		的节点则从它的父节点继承该属性
	4）.interrupts(设备节点属性) 包含一个中断指示符的列表,对应于设备上的中断输出信号

eg:
/{
	compatible = "acme,coyoted-revenge";
	#address-cells = <1>;
	#size-cells = <1>；
	interrupt-parent = <&intc>;  

	serial@101f0000{  /* 继承于 intc  */
		compatible = "arm, pl011";
		reg = <0x101f0000 0x1000>;
		interrupts = < 1 0 >;
	};

	intc:interrupt-controller@10140000 { /* 其它设备可以引用整个节点 */
		compatible = "arm,pl190";
		reg = <0x10140000 0x1000>;
		interrupt-controller;  /* 表示该节点是中断控制器 */
		#interrupt-cells<2>;
	};

};




