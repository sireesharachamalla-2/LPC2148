                PRESERVE8
                THUMB

; Area Definition and Entry Point
; Startup Code must be linked first at Address at which it expects to run.

                AREA    RESET, CODE, READONLY
                ARM

; Exception Vectors
; Mapped to Address 0.
; Absolute addressing mode must be used.
; Dummy Handlers are implemented as infinite loops which can be modified.

Vectors         LDR     PC, Reset_Addr         
                LDR     PC, Undef_Addr
                LDR     PC, SWI_Addr
                LDR     PC, PAbt_Addr
                LDR     PC, DAbt_Addr
                NOP                            ; Reserved Vector 
                LDR     PC, [PC, #-0x0FF0]     ; Vector from VICVectAddr
                LDR     PC, FIQ_Addr

; Vector Table Entries
Reset_Addr      DCD     Reset_Handler
Undef_Addr      DCD     Undef_Handler
SWI_Addr        DCD     SWI_Handler
PAbt_Addr       DCD     PAbt_Handler
DAbt_Addr       DCD     DAbt_Handler
                DCD     0                      ; Reserved Address 
IRQ_Addr        DCD     IRQ_Handler
FIQ_Addr        DCD     FIQ_Handler

; ---------------------------------------------------------------
; Exception Handlers
; Each handler loops forever unless replaced by real handler code
; ---------------------------------------------------------------

Undef_Handler   B       Undef_Handler
SWI_Handler     B       SWI_Handler
PAbt_Handler    B       PAbt_Handler
DAbt_Handler    B       DAbt_Handler
IRQ_Handler     B       IRQ_Handler
FIQ_Handler     B       FIQ_Handler

; ---------------------------------------------------------------
; Reset Handler Declaration
; This will be linked from main FreeRTOS application or crt0.s
; ---------------------------------------------------------------
                IMPORT  SystemInit
                IMPORT  main

Reset_Handler
                BL      SystemInit              ; Initialize PLL, VPB, etc.
                LDR     R0, =main               ; Call main()
                               MOV     LR, PC
                BX      R0

LoopForever
                B       LoopForever

                END
