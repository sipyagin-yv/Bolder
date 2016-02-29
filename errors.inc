                ; Структура для описания ошибки
struct          EInfo
lasterror       rd      1               ; Системный код ошибки (полученный с помощью GetLastError)
action          rd      1               ; Описание действия, приведшего к ошибке lasterror
user            rd      1               ; Описание пользовательского действия, приведшего к ошибке
ends

                .udata
error           EInfo

                ; Макросы для установки подробной информации об ошибке
macro           CLEARERROR
{
                xor     eax, eax
                mov     [error.lasterror], eax
                mov     [error.action], eax
                mov     [error.user], eax
}

macro           SYSTEMERROR     ActionMsg*
{
                invoke  GetLastError
                mov     [error.lasterror], eax
                mov     [error.action], ActionMsg
                mov     [error.user], 0
}

macro           USERERROR       ActionMsg=0, UserMsg*
{
                mov     [error.lasterror], 0
                mov     [error.action], ActionMsg
                mov     [error.user], UserMsg
}

                .const
CauseExceptMsg  du      'cause exception ', 0
CauseUnkExcMsg  du      'cause unknown exception', 0

                .code
                ; Показать сообщение об ошибке в следующем формате:
                ;       BOLDER: <main_msg>
                ;       [<action_msg> cause exception <exception_msg>]
                ;
                ; In:   main_msg          - сообщение о том, что не получилось, выводится в <main_msg>
                ;       [error.lasterror] - код системной ошибки, выводится в <exception_msg> (=0, чтобы не выводить)
                ;       [error.action]    - указатель на сообщение <action_msg> (=0, чтобы не выводить)
                ;       [error.user]      - указатель на сообщение <exception_msg> (=0, чтобы не выводить)
                ;
                ;       Если указан error.user, то выводится это сообщение (error.lasterror при этом игнорируется). 
                ;       Иначе error.lasterror.
                ;
                ;       user_question   - адрес с текстом-вопросом, что делать далее (null, если вопроса нет)
                ;
                ;       user_buttons    - маска с кнопками-ответами на вопрос (если user_question=null, то игнорируются)
                ;
                ; Out:  function exit to OS.
                ;
proc            ShowError       main_msg:DWORD, user_question:DWORD, user_buttons:DWORD
                local   system_msg[256]:WORD            ; Временный буфер для формирования системного сообщения об ошибке
                local   out_msg[512]:WORD               ; Буфер для формирования сообщения
                local   exception_msg:DWORD             ; Указатель на сообщение <exception_msg>
                ;
                mov     eax, [error.user]
                or      eax, eax
                jnz     .L1
                mov     eax, [error.lasterror]
                or      eax, eax
                jz      .L1
                ;
                invoke  FormatMessage, FORMAT_MESSAGE_FROM_SYSTEM + FORMAT_MESSAGE_MAX_WIDTH_MASK, \
                        0, eax, 0, addr system_msg, 256, NULL
                or      eax, eax
                jz      .L1
                ; Удаляем конечные пробелы и точку
                lea     esi, [system_msg]
                trimu   esi, eax, ' '
                trimu   esi, eax, '.'
                mov     word [esi + eax*2], 0
                ;
                lea     eax, [system_msg]
 .L1:           mov     [exception_msg], eax                    ; Сохраняем адрес <exception_msg>

                lea     edi, [out_msg]
                ; 1-я строка сообщения
                movsu   ProgramName                             ; "BOLDER"
                stosu   ':', ' '
                movsu   [main_msg]                              ; <главное_сообщение>
                stosu   '.'

                ; 2-я строка сообщения, если указаны
                ; [error.action] или [exception_msg]
                xor     eax, eax
                cmp     [error.action], eax
                jne     .Line2
                cmp     [exception_msg], eax
                jne     .Line2
                jmp     .CloseMessage

                ; defined error.action, defined exception_msg:
                ; [ <error.action> cause exception "<exception_msg>" ]
                ;
                ; defined error.action:
                ; [ <error.action> cause unknown exception ]
                ;
                ; defined exception.msg:
                ; [cause exception "<exception_msg"]
.Line2:         stosu   13, 10, '['
                mov     esi, [error.action]                     ; output action
                or      esi, esi
                jz      .L2
                movsu
                stosu   ' '
.L2:            mov     eax, [exception_msg]                    ; output exception
                or      eax, eax
                jz      .L3
                movsu   CauseExceptMsg
                stosu   '"'
                movsu   [exception_msg]
                stosu   '"'
                jmp     .L4
.L3:            movsu   CauseUnkExcMsg
.L4:            stosu   ']', '.'

.CloseMessage:  ; Далее разбираемся с пользовательским вопросом
                mov     esi, [user_question]
                or      esi, esi
                jnz     .user_q
                stosu   0
                invoke  MessageBox, 0, addr out_msg, 0, MB_OK + MB_ICONERROR
                invoke  ExitProcess, eax
                ret

 .user_q:       stosu   13, 10, 13, 10
                movsu  
                stosu   0
                mov     eax, [user_buttons]
                or      eax, eax
                jnz     .L5
                mov     eax, MB_OK
 .L5:           or      eax, MB_ICONERROR
                invoke  MessageBox, 0, addr out_msg, 0, eax
                ret                
endp
