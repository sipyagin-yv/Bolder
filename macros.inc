;-------------------------------------;
; Макросы для определение секций кода ;
; и объединения подобных секций в одну;
;-------------------------------------;
; copyright YSoft lab. (2016)         ;
;-------------------------------------;
; rev_init 30/01/2016                 ;
; patch 20/02/2016 (if 1=1 заменен на ;
;   if current_section = 1, иначе     ;
;   структуры и прочие вещи           ;
;   определяются по несколько раз )   ;
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
                ; Примечание:
                ;       После объявления секций определяем несколько байт, иначе, при запуске увидим 
                ;       "невозможно запустить это приложение"
                ; Объяснение:
                ;       Если секция имеет нулевой размер, то следующая объявленная 
                ;       секция "накладывается" поверх, получается, что обе секции 
                ;       будут иметь один и тот же RVA, что приводит к ошибке запуска.
                     if current_section = 1
                        section '.code' code readable executable
                        db      16 dup(0cch)
                        ;;display 'start .code',13,10

                else if current_section = 2
                        section '.data' data readable writeable
                        db      16 dup(0cch)
                        ;;display 'start .data',13,10
                         
                else if current_section = 3
                        section '.udata' data readable writeable
                        db      16 dup(?)
                        ;;display 'start .udata',13,10

                else if current_section = 4
                        section '.const' data readable
                        db      16 dup(0cch)
                        ;;display 'start .const',13,10

                end if

                ; Начало определений секций
                ; "беспризорный" код отправляем в секцию .code
                if current_section = 1;patch 20/02/2016
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
