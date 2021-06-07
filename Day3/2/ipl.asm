;har1-b0OS-ipl
; TAB=4
;��������OS�{�̂�ǂݍ���(���̃R�[�h�̓u�[�g�Z�N�^�{��)
CYLS		EQU		10
		ORG		0x7c00		;���̃v���O�������ǂ��ɓǂݍ��܂��̂�

;�ȉ��͕W���I��FAT12�t�H�[�}�b�g�t���b�s�[�f�B�X�N�̂��߂̋L�q

		JMP		entry
		DB		0x90
		DB		"HAR1-B0ipl"		;�u�[�g�Z�N�^�̖��O
		DW		512				;1�Z�N�^�̑傫��
		DB		1				;FAT�J�n�ꏊ
		DB		2				;FAT�̌�
		DW		224				;���[�g�f�B���N�g���̈�̑傫��
		DW		2880			;�h���C�u�̑傫��
		DB		0xf0			;���f�B�A�^�C�v
		DW		9				;FAT�̈�̒���
		DW		18				;1�g���b�N�ɂ���Z�N�^�̐�
		DW		2				;�w�b�h��
		DD		0				;�p�[�e�B�V����
		DB		0,0,0x29		;���̒l�ɂ���
		DD		0xffffffff		;�{�����[���V���A���ԍ�
		DB		"HAR1-B0OS"		;�f�B�X�N�̖��O
		DB		"FAT12	"		;�t�H�[�}�b�g�̖��O
		RESB	18				;���ɂ����Ƃ�

;�v���O�����{��

entry:
		MOV		AX,0			;���W�X�^������
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX

;�f�B�X�N��ǂ�

		MOV		AX,0x0820
		MOV		ES,AX
		MOV		CH,0			;�V�����_0
		MOV		DH,0			;�w�b�h0
		MOV		CL,2			;�Z�N�^2

readloop:
		MOV		SI,0			;���s�񐔂𐔂��郌�W�X�^

retry:
		MOV		AH,0x02			;AH=0x02 :�f�B�X�N�ǂݍ���
		MOV		AL,1			;1�Z�N�^
		MOV		BX,0			
		MOV		DL,0			;A�h���C�u
		INT		0x13			;�f�B�X�NBIOS�Ăяo��
		JNC		next				;�G���[�ɂȂ�Ȃ�������fin��
		ADD		SI,1			;SI��1�𑫂�
		CMP		SI,5			;SI��5���r
		JAE		error			;SI>=5��������error��
		MOV		AH,0x00			;(�V�X�e�����Z�b�g�ɕK�v�ȑ���)
		MOV		DL,0x00			;A�h���C�u(�V�X�e�����Z�b�g�ɕK�v�ȑ���)
		INT		0x13			;�h���C�u�̃��Z�b�g(�V�X�e�����Z�b�g�ɕK�v�ȑ���)
		JMP		retry

next:
		MOV		AX,ES			;�J�n�ʒu(0x820)�܂ňړ�����
		ADD		AX,0x0020		;�A�h���X��0x200�i�߂�
		MOV		ES,AX			;ADD ES,0x20�Ƃ������߂��Ȃ��̂ł��̂悤�ɕ\�L
		ADD		CL,1			;CL��1�𑫂�
		CMP		CL,18			;CL��18���r
		JBE		readloop		;CL<=18��������readloop��
		MOV		CL,1
		ADD		DH,1
		CMP		DH,2
		JB		readloop		;DH<2��������readloop��
		MOV		DH,0
		ADD		CH,1
		CMP		CH,CYLS
		JB		readloop		;CH<CYLS��������readloop

;�ǂݏI�������̏���

fin:
		HLT						;��������܂�CPU���~������
		JMP		fin				;�������[�v

error:
		MOV		SI,msg
putloop:
		MOV		AL,[SI]
		ADD		SI,1			;SI��1�𑫂�
		CMP		AL,0
		JE		fin
		MOV		AH,0x0e			;�ꕶ���\���t�@���N�V����
		MOV		BX,15			;�J���[�R�[�h
		INT		0x10			;�r�f�IBIOS�Ăяo��
		JMP		putloop
msg:
		DB		0x0a,0x0a		;���s��2��
		DB		"load error"
		DB		0x0a			;���s
		DB		0

		RESB	0x7dfe-0x7c00-($-$$)		;0x1ef�܂ł�0x00�Ŗ��߂閽��

		DB		0x55,0xaa		;BS BootSign
