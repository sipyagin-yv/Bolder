;-------------------------------------;
; ������� ��� ����������� ������ ���� ;
; � ����������� �������� ������ � ����;
;-------------------------------------;
; copyright YSoft lab. (2016)         ;
;-------------------------------------;
; rev_init 30/01/2016                 ;
;-------------------------------------;

; ��������� ������:
; 1 = code
; 2 = data
; 3 = udata
; 4 = const

macro           .code
{
                end if                  ; ��������� ���������� ������
                if current_section = 1  ; �������� ����� ������ .code
}

macro           .data
{
                end if                  ; ��������� ���������� ������
                if current_section = 2  ; �������� ����� ������ .data
}

macro           .udata
{
                end if                  ; ��������� ���������� ������
                if current_section = 3  ; �������� ����� ������ .udata
}

macro           .const
{
                end if                  ; ��������� ���������� ������
                if current_section = 4  ; �������� ����� ������ .const
}

macro           BEGIN
{
                repeat  4

                ; ��������� ������� ������
current_section =       %

                ; ����������, ����� ������ ���������
                     if current_section = 1
                        section '.code' readable executable
                                ;db      'SECTION CODE----'
                else if current_section = 2
                        section '.data' readable writeable
                                ;db      'SECTION DATA----'
                else if current_section = 3
                        section '.udata' readable writeable
                                ;db      'SECTION UDATA---'
                else if current_section = 4
                        section '.const' readable
                                ;db      'SECTION CONST---'
                end if

                ; ������ ����������� ������
                if 1=1
}

macro           END
{
                ; ���������� ����������� ������
                end if

                ; ���������� ����� �� �������
                end repeat

                ; ���������� ������ ��������� � ����������� ������������� ��������
                .end    start
}

                BEGIN
