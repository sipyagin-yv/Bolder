;-------------------------------------------------------------------------------------;
; Арифметический кодировщик. Кодирует несколько бит за раз.                           ;
; Размер кодера/декодера ~ 400 байт                                                   ;
;-------------------------------------------------------------------------------------;
; ИСПОЛЬЗОВАНИЕ                                                                       ;
; =============                                                                       ;
; 1. ДЛЯ КОДИРОВАНИЯ                                                                  ;
;    a) AC_EncodeStart (eax = адрес для записи сжатых данных)                         ;
;    b) AC_EncodeBits (eax = кодируемые биты, bl = сколько бит кодировать)            ;
;    c) AC_EncodeEnd (OUT eax = размер закодированных данных)                         ;
; 2. ДЛЯ ДЕКОДИРОВАНИЯ                                                                ;
;    a) AC_DecodeStart (eax = адрес сжатых данных)                                    ;
;    b) AC_DecodeBits (bl = сколько бит декодировать, OUT eax = декодированные биты)  ;
;-------------------------------------------------------------------------------------;

AC_MODEL_BITS   =       13              ; Количество бит жестко задано
AC_MODEL_COUNT  =       (1 SHL AC_MODEL_BITS)
AC_MODEL_MASK   =       AC_MODEL_COUNT-1

struct          AC_Freq_Item
bit0            rd      1               ; Количество битов 0 после последовательности бит bits
bit1            rd      1               ; Количество битов 1 после последовательности бит bits
ends

struct          AC_State
pk_ptr          rd      1               ; Текущий указатель на сжатые данные
pk_ptr_orig     rd      1               ; Начальное значение pk_ptr
min             rd      1               ; Min, нижняя граница, для кодирования/декодирования
max             rd      1               ; Max, верхняя граница, для кодирования/декодирования
val             rd      1               ; Val, для декодирования
bits            rd      1               ; Текущие биты, для кодирования/декодирования
freq            rd      AC_MODEL_COUNT*2; freq[AC_MODEL_COUNT]: array of AC_Freq_Item
ends

struct          AC_Data_Header
src_len         rd      1
pk_len          rd      1
ends

                .udata
ac_state        AC_State

                .code
AC2_BEGIN:
;=== HELPERS ==================================================================

                ; Внутренняя процедура
                ; Делаем общие действия для кодирования/декодирования
                ; ecx = AC_State
proc            AC_Helper_Predict uses ebx edx
                mov     eax, edi                        ; Mid := Min + (Max-Min-1) * freq[bits].bit0 /
                sub     eax, esi                        ;        (freq[bits].bit0 + freq[bits].bit1)
                dec     eax
                mov     ebx, [ac_state.freq + ebp*sizeof.AC_Freq_Item + AC_Freq_Item.bit0]     ; freq[bits].bit0
                mul     ebx
                add     ebx, [ac_state.freq + ebp*sizeof.AC_Freq_Item + AC_Freq_Item.bit1]     ; freq[bits].bit1
                div     ebx
                add     eax, esi
                cmp     eax, esi                        ; if (Mid == Min) Mid++;
                jne     .L1
                inc     eax
 .L1:           dec     edi                             ; if (Mid == (Max-1)) Mid--;
                cmp     eax, edi
                jne     .L2
                dec     eax
 .L2:           inc     edi
                ret
endp

                ; Инициализация переменных
                ; IN    eax = адрес данных (для сжатия/для декодирования)                
                ; OUT   eax = указатель на структуру ac_state
proc            AC_Helper_Init
                push    eax
                lea     edi, [ac_state.freq]            ; Инициализация таблицы частот
                mov     ecx, AC_MODEL_COUNT * 2
                xor     eax, eax
                inc     eax
                rep     stosd
                pop     ebx                             ; Инициализация состояния кодера/декодера
                xor     esi, esi
                mov     edi, 1000000h
                xor     ebp, ebp
                call    AC_Helper_StoreState
                mov     [eax + AC_State.pk_ptr_orig], ebx
                ret
endp

                ; Внутренняя процедуры, обновляем значения min, max
proc            AC_Helper_UpdateMinMax
                mov     eax, 0ffff00h                   ; load constant
                shl     esi, 8                          ;    Min := (Min SHL 8) AND $FFFF00;
                and     esi, eax
                shl     edi, 8                          ;    Max := (Max SHL 8) AND $FFFF00;
                and     edi, eax
                cmp     esi, edi                        ;    if Min >= Max then Max := $1000000;
                jb      .L1
                mov     edi, 1000000h
 .L1:           ret
endp

proc            AC_Helper_LoadState
                mov     eax, ac_state
                mov     esi, [eax + AC_State.min]
                mov     edi, [eax + AC_State.max]
                mov     ebp, [eax + AC_State.bits]
                mov     ebx, [eax + AC_State.pk_ptr]
                ret
endp

proc            AC_Helper_StoreState
                mov     eax, ac_state
                mov     [eax + AC_State.min], esi
                mov     [eax + AC_State.max], edi
                mov     [eax + AC_State.bits], ebp
                mov     [eax + AC_State.pk_ptr], ebx
                ret
endp

                ; Обновить счётчики
                ; IN    al = value (значение кодируемого бита, 0 или 1)
proc            AC_Helper_Update
                and     eax, 1
                shl     eax, 2                          ; *4, eax = смещение к счётчику бита 0 или 1 (eax = 0, or eax = 4)
                inc     [ac_state.freq + ebp*sizeof.AC_Freq_Item + eax]
                shr     eax, 2
                lea     ebp, [ebp*2+eax]
                and     ebp, AC_MODEL_MASK
                ret
endp

proc            AC_Helper_WhileCheck
                mov     eax, edi                        ;  while (Max-Min) < 256 do     
                sub     eax, esi                        ;  {
                cmp     eax, 256                        
                jae     .return
                mov     eax, ebp
                and     eax, 1
                sub     edi, eax
                stc
 .return:       ret
endp

;=== ENCODER ==================================================================

                ; Начинаем сжатие
                ; IN    eax = адрес для сжатых данных
                ; OUT   -
proc            AC_EncodeStart
                pushad
                call    AC_Helper_Init
                popad
                ret
endp

                ; Закодировать биты
                ; IN    bl = количество кодируемых бит. Кодируются биты с bl-1 по 0.
                ;       eax = значение бит
                ; OUT   -
proc            AC_EncodeBits
                pushad
                mov     cl, 32
                sub     cl, bl
                shl     eax, cl
                mov     edx, eax                        ; edx = bits to encode; byte [esp] = counter
                movzx   ecx, bl                         ; ecx = counter
                call    AC_Helper_LoadState
                ; registers plan:
                ;       eax = 
                ;       ebx = pk_ptr
                ;       ecx = counter
                ;       edx = bits to encode
                ;       ebp = bits
                ;       esi = min
                ;       edi = max
 .LoopBits:     call    AC_Helper_Predict               ; Load values and calculate Mid
                or      edx, edx                        ; if (value == 1) Min=Mid else Max=Mid
                cmovs   esi, eax
                cmovns  edi, eax
                sets    al                              ; al = value
                call    AC_Helper_Update
 .LoopWhile:    call    AC_Helper_WhileCheck
                jae     .L2
                mov     eax, esi                        ;    buffer[bufferIndex] := Min SHR 16;
                shr     eax, 16                         ;    inc(bufferIndex);                 
                mov     [ebx], al
                inc     ebx
                call    AC_Helper_UpdateMinMax
                jmp     .LoopWhile                      ;  } // while
 .L2:           shl     edx, 1
                loop    .LoopBits
                call    AC_Helper_StoreState
                popad
                ret
endp

                ; Завершаем сжатие
                ; IN    -
                ; OUT   eax = размер сжатых данных
proc            AC_EncodeEnd uses ebx edi
                mov     ebx, ac_state
                mov     edi, [ebx + AC_State.pk_ptr]
                mov     eax, [ebx + AC_State.min]
                bswap   eax                             ; OutputByte( Min SHR 16)
                shr     eax, 8                          ; OutputByte( Min SHR 8) 
                stosd                                   ; OutputByte( Min )      
                dec     edi
                mov     [ebx + AC_State.pk_ptr], edi    ; Для дальнейшего кодирования с теми же частотами!
                sub     edi, [ebx + AC_State.pk_ptr_orig]
                mov     eax, edi
                ret
endp

;=== DECODER ==================================================================

                ; Инициализация декодера
                ; IN    eax = адрес сжатых данных
proc            AC_DecodeStart
                pushad
                mov     ebx, [eax]                      ; Инициализация val
                bswap   ebx                             ; загружаем три байта из сжатых данных: 
                shr     ebx, 8                          ; eax = 00 | [eax+0] | [eax+1] | [eax+2]
                add     eax, 3
                push    ebx
                call    AC_Helper_Init
                pop     [eax + AC_State.val]
                popad
                ret
endp

                ; Декодировать биты из потока
                ; IN    bl = количество читаемых бит
                ; OUT   eax = прочитанный бит
proc            AC_DecodeBits
                pushad
                push    ebx                             ; Счётчик бит
                xor     edx, edx                        ; Здесь будем набирать биты, вдвигаться они будут в младший бит
                call    AC_Helper_LoadState
                mov     ecx, [eax + AC_State.val]
                ; registers plan:
                ;       eax = 
                ;       ebx = pk_ptr
                ;       ecx = val
                ;       edx = bits to decode
                ;       ebp = bits
                ;       esi = min
                ;       edi = max
 .LoopBits:     call    AC_Helper_Predict               ; Load and Calculate mid
                cmp     ecx, eax                        ;  if( Val >= Mid ) { result := 1; Min := Mid; }
                cmovae  esi, eax                        ;              else { result := 0; Max := Mid; }
                cmovb   edi, eax
                setae   al
                shl     edx, 1
                or      dl, al
                call    AC_Helper_Update
 .LoopWhile:    call    AC_Helper_WhileCheck
                jae     .L2
                movzx   ecx, cx                         ; Val = ((Val SHL 8) AND $FFFF00) OR InputByte(); 
                shl     ecx, 8
                mov     cl, [ebx]
                inc     ebx
                call    AC_Helper_UpdateMinMax
                jmp     .LoopWhile                      ;  } // while
 .L2:           dec     byte [esp]
                jnz     .LoopBits
                pop     eax                             ; restore counter
                mov     [esp + pushad_data.r_eax], edx  ; store result
                call    AC_Helper_StoreState
                mov     [eax + AC_State.val], ecx       ; store val
                popad
                ret
endp

                ; Процедура не требуется на самом деле, т.к. нечего выполнять в конце декодирования
proc            AC_DecodeEnd
                ret
endp

AC2_END:
