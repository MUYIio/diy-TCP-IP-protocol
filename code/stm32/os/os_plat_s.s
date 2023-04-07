                AREA    SWITCH, CODE, READONLY
PendSV_Handler  PROC
                EXPORT  PendSV_Handler
				IMPORT os_switch_ctx
				; 保存其余寄存器
                CPSID   I                                                   ; Prevent interruption during context switch
				MRS     R0, PSP
    			CBZ     R0, PendSV_Handler_Nosave		                    ; Skip register save the first time
				STMDB   R0!, {R4-R11}

PendSV_Handler_Nosave
				; 调用函数：参数通过R0传递，返回值也通过R0传递 
				BL      os_switch_ctx

				; 恢复下一任务寄存器
				LDMIA   R0!, {R4-R11} 
				MSR     PSP, R0    
				
				; 切换到普通用户模式
				MOV     LR, #0xFFFFFFFD             
                CPSIE   I
				BX      LR
                ENDP
					
					
				END
