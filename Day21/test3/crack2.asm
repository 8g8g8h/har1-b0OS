[BITS 32]
    MOV   EAX,1*8           ;OS用のセグメント
    MOV   DS,AX             ;これをDSに入れる
    MOV   BYTE [0x102600],0 ;ここで[0x102600]の計算はDSのアドレスにある0x102600という意味になる
    RETF
