[BITS 32]
		MOV   ECX,msg
    MOV   EDX,1
putloop:
    MOV   AL,[CS:ECX] ;このCSはこの機械語コードがどのセグメントに展開されているかを示している
    CMP   AL,0
    JE    fin
    INT   0x40
    ADD   ECX,1
    JMP   putloop
fin:
    MOV   EDX,4     ;ここでアプリが終了するモードに移行するためにEDXに4を入れて終了モードに移行するようにしている。
    INT   0X40     
msg:
    DB    "hello",0
