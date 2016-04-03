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
                je      .createfile_fail

                invoke  GetFileSizeEx, [h], addr fileSize       ; Определяем размер файла
                or      eax, eax
                jz      .getfilesizeex_fail

                mov     eax, [fileSize.high]                    ; Проверяем максимальный размер
                or      eax, eax
                jnz     .filesize_toobig
                mov     eax, [fileSize.low]
                cmp     eax, [fileBufferSize]
                ja      .filesize_toobig

                invoke  ReadFile, [h], [fileBuffer], \          ; Читаем файл
                        [fileSize.low], addr fileReaded, NULL
                or      eax, eax
                jz      .readfile_fail

                mov     eax, [fileReaded]                       ; Проверяем, сколько прочитано
                cmp     eax, [fileSize.low]
                jne     .readfile_wrongread

                invoke  CloseHandle, [h]                        ; Закрываем файл
                or      eax, eax
                jz      .closehandle_fail

                clc                                             ; Всё в порядке, выходим
                popad
                ret

 .fail:         stc
                popad
                ret

 .createfile_fail:
                SYSTEMERROR     ActionOpenFile
                jmp     .fail
 .getfilesizeex_fail:
                SYSTEMERROR     ActionGetFileSize
                jmp     .fail
 .filesize_toobig:
                USERERROR       , ErrorResourceTooBig
                jmp     .fail
 .readfile_fail:
                SYSTEMERROR     ActionReadFile
                jmp     .fail
 .readfile_wrongread:
                USERERROR       , ErrorReadNotEqRequested
                jmp     .fail
 .closehandle_fail:
                SYSTEMERROR     ActionCloseHandle
                jmp     .fail
endp

proc            SaveFile        fileName:DWORD, fileBuffer:DWORD, fileSize:DWORD
                local   h:DWORD
                local   fileWritten:DWORD

                pushad
                invoke  CreateFile, [fileName], GENERIC_WRITE, \        ; Открываем файл для записи
                        0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL
                mov     [h], eax
                or      eax, eax
                jz      .createfile_fail

                invoke  WriteFile, [h], [fileBuffer], \                 ; Записываем в файл
                        [fileSize], addr fileWritten, NULL
                or      eax, eax
                jz      .writefile_fail
                
                mov     eax, [fileWritten]                              ; Проверяем, сколько байт записалось
                cmp     eax, [fileSize]
                jne     .writefile_wrongsize

                invoke  CloseHandle, [h]                                ; Закрываем файл
                or      eax, eax
                jz      .closehandle_fail

                clc
                popad
                ret

 .fail:         stc
                popad
                ret

 .createfile_fail:
                SYSTEMERROR     ActionCreateFile
                jmp     .fail
 .closehandle_fail:
                SYSTEMERROR     ActionCloseHandle
                jmp     .fail
 .writefile_fail:
                SYSTEMERROR     ActionWriteFile
                jmp     .fail
 .writefile_wrongsize:
                USERERROR       , ErrorWritedNotEqRequested
                jmp     .fail
endp
