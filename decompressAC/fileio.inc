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
                xor     eax, eax
 .return:       popad
                ret

 .fail:         mov     eax, -1
                jmp     .return
endp
