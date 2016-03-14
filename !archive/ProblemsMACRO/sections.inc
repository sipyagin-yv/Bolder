SECT_DEFINES    =       1
SECT_CODE       =       2
SECT_DATA       =       3
SECT_UDATA      =       4
SECT_CONST      =       5

macro           .defines
{
                end if
                if current_section = SECT_DEFINES
                display 'continue defines...',13,10
}

macro           .code
{
                end if
                if current_section = SECT_CODE
                display 'continue code...',13,10
}

macro           .data
{
                end if
                if current_section = SECT_DATA
                display 'continue data...',13,10
}

macro           .udata
{
                end if
                if current_section = SECT_UDATA
                display 'continue udata...',13,10
}

macro           .const
{
                end if
                if current_section = SECT_CONST
                display 'continue const...',13,10
}

macro           BEGIN
{
                repeat  5

                ; Установка текущей секции
current_section =       %

                     if current_section = SECT_CODE
                        section '.code' code readable executable
                        db      16 dup(0cch)
                        display 'START .code',13,10

                else if current_section = SECT_DATA
                        section '.data' data readable writeable
                        db      16 dup(0cch)
                        display 'START .data',13,10
                         
                else if current_section = SECT_UDATA
                        section '.udata' data readable writeable
                        db      16 dup(?)
                        display 'START .udata',13,10

                else if current_section = SECT_CONST
                        section '.const' data readable
                        db      16 dup(0cch)
                        display 'START .const',13,10

                end if

                ; Начало определений секций
                ; "беспризорный" код отправляем в секцию .macro
                if current_section = SECT_DEFINES;patch 20/02/2016
                display 'START .defines',13,10

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
