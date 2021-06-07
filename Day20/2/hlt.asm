[BITS 32]
  MOV   AL,'A'  ;文字を代入
  CALL  0xfb4   ;asm_cons_putcharのアドレス
fin:
  HLT
  JMP   fin


