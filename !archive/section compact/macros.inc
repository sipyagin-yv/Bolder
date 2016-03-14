;-------------------------------------;
; Макросы для определение секций кода ;
; и объединения подобных секций в одну;
;-------------------------------------;
; copyright YSoft lab. (2016)         ;
;-------------------------------------;
; rev_init 30/01/2016                 ;
;-------------------------------------;

; Кодировка секций:
; 1 = code
; 2 = data
; 3 = udata
; 4 = const

macro           .code
{
                end if                  ; Закрываем предыдущую секцию
                if current_section = 1  ; Начинаем новую секцию .code
}

macro           .data
{
                end if                  ; Закрываем предыдущую секцию
                if current_section = 2  ; Начинаем новую секцию .data
}

macro           .udata
{
                end if                  ; Закрываем предыдущую секцию
                if current_section = 3  ; Начинаем новую секцию .udata
}

macro           .const
{
                end if                  ; Закрываем предыдущую секцию
                if current_section = 4  ; Начинаем новую секцию .const
}

macro           BEGIN
{
                repeat  4

                ; Установка текущей секции
current_section =       %

                ; Определяем, какую секцию объявлять
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

                ; Начало определений секций
                if 1=1
}

macro           END
{
                ; Завершение определений секций
                end if

                ; Завершение цикла по секциям
                end repeat

                ; Завершение текста программы и определение импортируемых процедур
                .end    start
}

                BEGIN
