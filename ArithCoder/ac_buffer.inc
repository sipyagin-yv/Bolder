                ; Арифметический кодировщик. Кодирует/декодирует указанное количество байт за раз
                ; Например, можно кодировать/декодировать 1000 байт, 
                ; в каждом байте декодировать биты с 0 по ENC_MAXBIT.
                ; Размер кодера+декодера ~ 324 байта

MODEL_BITS      =       13
MODEL_COUNT     =       (1 SHL MODEL_BITS)
MODEL_MASK      =       MODEL_COUNT-1
ENC_MAXBIT      =       2                       ; Кодируем биты с 0 по BITS_ENC_MAX
ENC_BITMASK     =       (1 SHL ENC_MAXBIT)
ENC_COUNT       =       (1 SHL ENC_MAXBIT)-1

macro           DEC_AC  src, dst
{               mov     esi, src
                mov     edi, dst
                call    DecodeAC
}

macro           ENC_AC  src, dst, len
{               mov     esi, src
                mov     edi, dst
                mov     ecx, len
                call    EncodeAC
}

                .udata
model_freq      rb      sizeof.model_freq_item * MODEL_COUNT

struct          model_freq_item
bit0            rd      1
bit1            rd      1
ends

struct          AC_Header
pk_len          rd      1
src_len         rd      1
ends

                .code
AC_BEGIN:
proc            AC_Predict
                ; Mid = Min + (Max-Min-1) * model_freq[bits].bit0 / 
                ;       (model_freq[bits].bit0 + model_freq[bits].bit1)
                push    ebx edx
                mov     eax, edi
                sub     eax, esi
                dec     eax
                mov     ebx, dword [model_freq + ebp*8 + model_freq_item.bit0]
                mul     ebx
                add     ebx, dword [model_freq + ebp*8 + model_freq_item.bit1]
                div     ebx
                add     eax, esi
                pop     edx ebx
                ; if (Mid == Min) Mid++
                cmp     eax, esi
                jne     .L1
                inc     eax
 .L1:           ; if (Mid == (Max-1)) Mid--
                dec     edi
                cmp     eax, edi
                jne     .L2
                dec     eax
 .L2:           inc     edi
                ret
endp

proc            AC_CoderInit
                ; Инициализируем таблицу частот
                mov     edi, model_freq
                mov     ecx, MODEL_COUNT*2
                xor     eax, eax
                inc     al
                rep     stosd
                ; Начальные значения bits, min, max
                xor     ebp, ebp
                xor     esi, esi
 .LoadConstEDI: mov     edi, 1000000h
                ret
endp

proc            AC_UpdateMinMax
                mov     eax, 0ffff00h
                ; Min = (Min SHL 8) AND 0xffff00;
                shl     esi, 8
                and     esi, eax
                ; Max = (Max SHL 8) AND 0xffff00;
                shl     edi, 8
                and     edi, eax
                ; if(Min >= Max) Max = 0x1000000;
                cmp     esi, edi
                jae     AC_CoderInit.LoadConstEDI
                ret
endp

                ; AL = смещение к model_freq_item.bit0/model_freq_item.bit0
proc            AC_Update
                movzx   eax, al
                inc     dword [model_freq + ebp*8 + eax]
                shr     al, 2
                shl     ebp, 1
                or      ebp, eax
                and     ebp, MODEL_MASK
                ret
endp

                ; AL = кодируемый бит
proc            AC_Update2
                and     eax, 1
                shl     eax, 2
                inc     dword [model_freq + ebp*8 + eax]
                shr     eax, 2
                shl     ebp, 1
                or      ebp, eax
                and     ebp, MODEL_MASK
                ret
endp

proc            AC_CheckWhile
                mov     eax, edi
                sub     eax, esi
                cmp     eax, 256
                jae     .L1
                ; if(value==1) Max--;
                ;==> Max -= value;
                mov     eax, ebp
                and     eax, 1
                sub     edi, eax
                stc
 .L1:           ret
endp

                ; Encode data (compress)
                ; IN    esi = source data
                ;       ecx = size of source data
                ;       edi = buffer to store encoded data
                ; OUT   -
proc            EncodeAC
                pushad
                mov     [edi + AC_Header.src_len], ecx
                add     edi, sizeof.AC_Header
                push    edi                     ; store begin dest address
                push    ecx                     ; store counter, in [esp]
                ;
                push    esi                     ; push source
                push    edi                     ; push dest
                ; Начало упаковки
                call    AC_CoderInit
                pop     ecx                     ; pop ecx=dest_ptr
                pop     ebx                     ; pop ebx=source_ptr
                ; Использование регистров
                ; eax = mid, temp
                ; ebx = source_ptr
                ; ecx = dest_ptr
                ; edx = dh=кодируемый байт, dl=маска кодируемого бита в кодируемом байте
                ; ebp = bits
                ; esi = min
                ; edi = max
                ; Читаем очередной байт
 .LoopBytes:    mov     dh, [ebx]
                inc     ebx
                mov     dl, ENC_BITMASK         ; Маска для очередного бита
                ; Цикл кодирования бит
 .LoopBits:     call    AC_Predict
                ; if (value == 1) Min = Mid
                ;            else Max = Mid
                test    dh, dl
                cmovnz  esi, eax
                cmovz   edi, eax
                setnz   al
                call    AC_Update2

;                push    eax
;                test    dh, dl
;                jz      .L3
;                pop     esi
;                mov     al, model_freq_item.bit1; Бит 1
;                jmp     .L4
; .L3:           pop     edi
;                mov     al, model_freq_item.bit0; Бит 0
; .L4:           call    AC_Update

                ; Проверка, не пора ли вывести байт в выходной поток
 .LoopWhile:    call    AC_CheckWhile
                jae     .L5
                ; OutputByte( Min SHR 16 );
                mov     eax, esi
                bswap   eax
                mov     [ecx], ah
                inc     ecx
                call    AC_UpdateMinMax
                jmp     .LoopWhile
 .L5:           ; Цикл по битам
                shr     dl, 1
                jnz     .LoopBits
                dec     dword [esp]
                jnz     .LoopBytes
                pop     eax                     ; restore counter, now it equal 0
                ; OutputByte( Min SHR 16)
                ; OutputByte( Min SHR 8)
                ; OutputByte( Min )
                bswap   esi
                shr     esi, 8
                mov     [ecx], esi
                add     ecx, 3
                ;
                pop     eax                     ; restore begin dest address
                sub     ecx, eax                ; ecx = ecx + 3 - eax
                mov     [eax-sizeof.AC_Header+AC_Header.pk_len], ecx
                popad
                ret
endp

proc            DecodeAC
                pushad
                push    [esi+AC_Header.src_len] ; store counter in [esp]
                add     esi, sizeof.AC_Header
                push    edi                     ; push dest_ptr
                ; Начало распаковки
                lodsd                           ; загружаем три байта из сжатых данных:
                bswap   eax                     ; eax = 00 | [esi+0] | [esi+1] | [esi+2]
                shr     eax, 8
                dec     esi
                push    esi                     ; push source_ptr
                push    eax                     ; push val
                call    AC_CoderInit
                pop     ecx                     ; pop val
                pop     ebx                     ; pop source_ptr
                ; Использование регистров
                ; eax = mid, temporary
                ; ebx = source_ptr
                ; ecx = val
                ; edx = dl=счётчик бит, dh=декодируемый байт
                ; ebp = bits
                ; esi = min
                ; edi = max
 .LoopBytes:    mov     edx, ENC_COUNT          ; dl = количество записываемых бит в байт; dh = формируемый выходной байт
                ; Вычисляем очередной бит
 .LoopBits:     call    AC_Predict
                cmp     ecx, eax
                cmovae  esi, eax
                cmovb   edi, eax
                setae   al
                call    AC_Update2
                shl     dh, 1
                or      dh, al

                
;                shl     dh, 1                   ; Заранее готовим место под новый бит
;                push    eax                     ; if (Val >= Mid) { bit=1; min=mid; }
;                cmp     ecx, eax                ;           else  { bit=0; max=mid; }
;                jb      .bit0
;                pop     esi
;                mov     al, model_freq_item.bit1; Обновление для бита 1
;                inc     dh                      ; Новый бит = 1
;                jmp     .L4
; .bit0:         pop     edi
;                mov     al, model_freq_item.bit0; Обновление для бита 0
; .L4:           call    AC_Update

                ; Проверка, не пора ли прочитать очередной байт из входного потока
 .LoopWhile:    call    AC_CheckWhile
                jae     .L5
                ; Val = ((Val SHL 8) AND $FFFF00) OR InputByte();
                shl     ecx, 16                 ; ALT:    shl     ecx, 8       
                shr     ecx, 8                  ;         and     ecx, 0ffff00h
                mov     cl, [ebx]
                inc     ebx
                call    AC_UpdateMinMax
                jmp     .LoopWhile
 .L5:           dec     dl
                jnz     .LoopBits
                ; Сохранить байт в выходной поток
                pop     eax                     ; pop dest_ptr
                mov     [eax], dh
                inc     eax
                dec     dword [esp]             ; [counter]
                push    eax
                jnz     .LoopBytes
                pop     eax                     ; pop dest_ptr
                pop     eax                     ; pop [counter]
                popad
                ret
endp
AC_END:
