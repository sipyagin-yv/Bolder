struct          TBits
data            rd      1
origin          rd      1
buff            rd      1
count           rb      1
ends

                .udata
bits            TBits

                .code

;============================================================================================================;
; Работа с битами                                                                                            ;
; ---------------                                                                                            ;
; BitsPut:                                                                                                   ;
;       1. Добавляем новые биты в буфер бит слева                                                            ;
;       2. Отправить накопившиеся биты в буфере бит в выходной поток (если есть)                             ;
;          2.1 Корректируем битовый буфер                                                                    ;
; BitsGet:                                                                                                   ;
;       1. Заполняем битовый буфер, чтобы было достаточное количество бит для чтения                         ;
;       2. Извлекаем из битового буфера биты слева                                                           ;
;       3. Корректируем битовый буфер                                                                        ;
;============================================================================================================;
                
                ; IN    addr = addr of bits data
proc            BitsInit uses eax, addr:DWORD
                mov     eax, [addr]
                mov     [bits.data], eax
                mov     [bits.origin], eax
                xor     eax, eax
                mov     [bits.buff], eax
                mov     [bits.count], al
                ret
endp

                ; Записать бит 0 в выходной поток
proc            BitsPut0
                stdcall BitsPut, 0, 1
                ret
endp

                ; Записать бит 1 в выходной поток
proc            BitsPut1
                stdcall BitsPut, 1, 1
                ret
endp


proc            BitsGet1
                stdcall BitsGet, 1
                ret
endp

                ; Вывести count бит value в выходной поток
proc            BitsPut uses eax ebx ecx, value:DWORD, count:BYTE
                ; Загрузить переменные
                mov     ebx, [bits.buff]
                mov     ch, [bits.count]
                ; Сохранить биты в буфере бит
                mov     cl, ch
                shl     eax, cl
                or      ebx, eax
                add     ch, [count]
                cmp     ch, 8
                jb      .L1
                ; Сохранить байты в выходном потоке
                mov     eax, [bits.data]
                ; Цикл сохранения буфера бит
 .L2:           mov     [eax], bl
                inc     eax
                shr     ebx, 8
                sub     ch, 8
                cmp     ch, 8
                jae     .L2
                mov     [bits.data], eax
                ; Сохранить переменные
 .L1:           mov     [bits.count], ch
                mov     [bits.buff], ebx
                ret
endp

                ; Прочитать count бит из входного потока
proc            BitsGet uses ebx ecx edx, count:BYTE
                ; Загрузить переменные
                mov     ebx, [bits.buff]
                mov     ch, [bits.count]
                mov     cl, [count]
                ; Заполнить буфер, добавляя прочитанные биты слева
                cmp     cl, ch  ; Запрашиваем_бит <= имеется_бит ?
                jbe     .L1
                mov     edx, [bits.data]
                ; Цикл заполнения буфера бит
 .L2:           movzx   eax, byte [edx]
                inc     edx
                xchg    ch, cl
                shl     eax, cl
                or      ebx, eax
                xchg    ch, cl
                add     ch, 8
                cmp     cl, ch  ; Запрашиваем_бит > имеется_бит ?
                ja      .L2
                mov     [bits.data], edx
 .L1:           ; Готовим маску для извлечения бит
                mov     eax, 1
                shl     eax, cl
                dec     eax
                ; Извлекаем биты из битового буфера
                and     eax, ebx
                ; Корректировать битовый буфер
                shr     ebx, cl
                sub     ch, cl
                ; Сохранить переменные
                mov     [bits.count], ch
                mov     [bits.buff], ebx
                ret
endp

                ; Вывести в выходной поток оставшиеся биты из буфера
proc            BitsFlush
                movzx   eax, [bits.count]
                or      eax, eax
                jz      .done
                neg     al
                add     al, 8                   ; eax = 8 - eax
                stdcall BitsPut, 0, eax
 .done:         ; Вычислить размер сжатых данных
                mov     eax, [bits.data]
                sub     eax, [bits.origin]                
                ret
endp
