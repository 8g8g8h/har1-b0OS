GLOBAL  api_putchar
GLOBAL  api_putstr0
GLOBAL  api_end
[SECTION .text]
  
api_putchar:	;void api_putchar(int c)
    MOV   EDX,1
    MOV   AL,[ESP+4]  ;int c
    INT   0x40
    RET
api_putstr0:  ;void api_putstr0(char *s);
    PUSH  EBX
    MOV   EDX,2
    MOV   EBX,[ESP+8]
    INT   0x40
    POP   EBX
    RET

api_end:    ;void api_end(void)
    MOV   EDX,4
    INT   0x40

