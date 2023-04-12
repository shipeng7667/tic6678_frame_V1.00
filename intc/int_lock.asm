*********************************************************************************************
;By L.Q.Y
;函数作用:关闭中断
;返回:中断掩码
**********************************************************************************************

		.asg	B15,	SP							;栈指针
		.asg	B3,		return						;函数返回地址
		.asg	A4,		ret_val_32					;函数返回值

		.global int_lock

int_lock:	.asmfunc
			B		.S2		return
			DINT
			MVC		.S2		TSR, B16
			SHRU	.S2		B16, 1, B16
			AND		.L1X	B16, 1, A4
			NOP		1



			.endasmfunc
