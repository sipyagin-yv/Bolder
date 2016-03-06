macro           MSG     message
{               stdcall writeStr, message
}

macro           LOAD    fname, fbuffer, maxBufferSize, fail_addr
{               stdcall LoadFile, fname, fbuffer, maxBufferSize
                jc      fail_addr
}

macro           SAVE    fname, fbuffer, bufferSize, fail_addr
{               stdcall SaveFile, fname, fbuffer, bufferSize
                jc      fail_addr
}

struct          LARGE_INTEGER
low             rd      1
high            rd      1
ends

proc            LoadFile        fileName:DWORD, fileBuffer:DWORD, fileBufferSize:DWORD
                local   fileSize:LARGE_INTEGER
                local   h:DWORD
                local   fileReaded:DWORD

                pushad
                invoke  CreateFile, [fileName], GENERIC_READ, \ ; Открываем файл для чтения
                        0, NULL, OPEN_EXISTING, 0, 0
                mov     [h], eax
                cmp     eax, INVALID_HANDLE_VALUE
                je      .fail
                invoke  GetFileSizeEx, [h], addr fileSize       ; Определяем размер файла
                or      eax, eax
                jz      .fail
                mov     eax, [fileSize.high]                    ; Проверяем максимальный размер
                or      eax, eax
                jnz     .fail
                mov     eax, [fileSize.low]
                cmp     eax, [fileBufferSize]
                ja      .fail
                invoke  ReadFile, [h], [fileBuffer], \          ; Читаем файл
                        [fileSize.low], addr fileReaded, NULL
                or      eax, eax
                jz      .fail
                mov     eax, [fileReaded]
                cmp     eax, [fileSize.low]
                jne     .fail
                invoke  CloseHandle, [h]                        ; Закрываем файл
                or      eax, eax
                jz      .fail
                popad
                mov     eax, [fileReaded]
                clc
                ret

 .fail:         popad
                mov     eax, -1
                stc
                ret
endp

proc            SaveFile        fileName:DWORD, fileBuffer:DWORD, fileSize:DWORD
                local   h:DWORD
                local   fileWritten:DWORD

                pushad
                invoke  CreateFile, [fileName], GENERIC_WRITE, \        ; Открываем файл для записи
                        0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL
                mov     [h], eax
                or      eax, eax
                jz      .fail
                invoke  WriteFile, [h], [fileBuffer], \                 ; Записываем в файл
                        [fileSize], addr fileWritten, NULL
                or      eax, eax
                jz      .fail
                mov     eax, [fileWritten]
                cmp     eax, [fileSize]
                jne     .fail
                invoke  CloseHandle, [h]                                ; Закрываем файл
                or      eax, eax
                jz      .fail
                popad
                xor     eax, eax
                clc
                ret

 .fail:         popad
                mov     eax, -1
                stc
                ret
endp

proc            writeStr   msgaddr:DWORD
                local   written:DWORD
                pushad
                push    0                       ; lpReserved
                lea     eax, [written]
                push    eax                     ; lpNumberOfCharsWritten
                or      ecx, -1
                mov     edi, [msgaddr]
                xor     eax, eax
                repne   scasw
                not     ecx
                dec     ecx
                push    ecx                     ; nNumberOfCharsToWrite
                push    [msgaddr]               ; lpBuffer
                invoke  GetStdHandle, STD_OUTPUT_HANDLE
                push    eax                     ; hConsoleOutput
                invoke  WriteConsole
                popad
                ret
endp
