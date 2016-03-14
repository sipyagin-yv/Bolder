                .code
                ; Функции и процедуры для работы с файлами
                ;==========================================

                ; Load file into memory
                ;
                ; IN:   fileName - file of the loading file
                ;       fileBuffer - buffer, where the file is loading
                ;       fileBufferSize - size of the buffer
                ;
                ; OUT:  CF=1, if error
                ;       CF=0, otherwise
proc            LoadFile        fileName:DWORD, fileBuffer:DWORD, fileBufferSize:DWORD
                local   fileSize:LARGE_INTEGER
                local   h:DWORD
                local   fileReaded:DWORD

                pushad
                invoke  CreateFile, [fileName], GENERIC_READ, \ ; Открываем файл для чтения
                        0, NULL, OPEN_EXISTING, 0, 0
                mov     [h], eax
                cmp     eax, INVALID_HANDLE_VALUE
                jne     .L1
                SYSTEMERROR     ActionOpenFile
                jmp     .fail

 .L1:           invoke  GetFileSizeEx, [h], addr fileSize       ; Определяем размер файла
                or      eax, eax
                jnz     .L2
                SYSTEMERROR     ActionGetFileSize
                jmp     .fail

 .L2:           mov     eax, [fileSize.high]                    ; Проверяем максимальный размер
                or      eax, eax
                jnz     .L3
                mov     eax, [fileSize.low]
                cmp     eax, [fileBufferSize]
                jbe     .L4
 .L3:           USERERROR       , ErrorResourceTooBig
                jmp     .fail

 .L4:           invoke  ReadFile, [h], [fileBuffer], \          ; Читаем файл
                        [fileSize.low], addr fileReaded, NULL
                or      eax, eax
                jz      .L5
                mov     eax, [fileReaded]
                cmp     eax, [fileSize.low]
                je      .L6

                USERERROR       , ErrorReadNotEqRequested
                jmp     .fail

 .L5:           SYSTEMERROR     ActionReadFile
                jmp     .fail

 .L6:           invoke  CloseHandle, [h]                        ; Закрываем файл
                or      eax, eax
                clc
                jnz     .return

                SYSTEMERROR     ActionCloseHandle

 .fail:         stc
 .return:       popad
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
                jnz     .L1
                SYSTEMERROR     ActionCreateFile
                jmp     .fail

 .L1:           invoke  WriteFile, [h], [fileBuffer], \                 ; Записываем в файл
                        [fileSize], addr fileWritten, NULL
                or      eax, eax
                jz      .L2
                mov     eax, [fileWritten]
                cmp     eax, [fileSize]
                je      .L3
                USERERROR       , ErrorWritedNotEqRequested
                jmp     .fail

 .L2:           SYSTEMERROR     ActionWriteFile
                jmp     .fail

 .L3:           invoke  CloseHandle, [h]                                ; Закрываем файл
                or      eax, eax
                clc
                jnz     .return
                SYSTEMERROR     ActionCloseHandle

 .fail:         stc
 .return:       popad
                ret
endp
