                ; Арифметический кодировщик. Позволяет закодировать за раз проивзольное количество
                ; бит. Например, сначала 8 бит, потом 3, затем массив из 1024 байт и т.д.
                ; Размер кодера/декодера ~ 341 байт

AC_MODEL_BITS   =       13
AC_MODEL_COUNT  =       (1 SHL AC_MODEL_BITS)
AC_MODEL_MASK   =       (1 SHL AC_MODEL_BITS)-1

struct          AC_Freq_Item
bit0            rd      1
bit1            rd      1
ends

struct          AC_Status
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
ac_status       AC_Status

                .code
AC2_BEGIN:
;=== HELPERS ==================================================================

                ; registers plan:
                ;       esi = min
                ;       edi = max
                ;       ebp = bits
                ;       eax = mid, temp
                ;       ebx = pk_ptr
                ;       edx = encoded/decoded bit
                ;       ecx = ac_status
                
                ; Внутренняя процедура
                ; Делаем общие действия для кодирования/декодирования
                ; ecx = AC_Status
proc            AC_Helper_LoadAndPredict
                mov     esi, [ecx + AC_Status.min]      ; load values
                mov     edi, [ecx + AC_Status.max]
                mov     ebp, [ecx + AC_Status.bits]
                mov     eax, edi                        ; Mid := Min + (Max-Min-1) * freq[bits].bit0 /
                sub     eax, esi                        ;        (freq[bits].bit0 + freq[bits].bit1)
                dec     eax
                mov     ebx, [ecx + AC_Status.freq + ebp*sizeof.AC_Freq_Item + AC_Freq_Item.bit0]
                mul     ebx
                add     ebx, [ecx + AC_Status.freq + ebp*sizeof.AC_Freq_Item + AC_Freq_Item.bit1]
                div     ebx
                add     eax, esi
                cmp     eax, esi                        ; if Mid = Min then Inc(Mid);
                jne     .L1
                inc     eax
 .L1:           dec     edi                             ; if Mid = (Max-1) then Dec(Mid);
                cmp     eax, edi
                jne     .L2
                dec     eax
 .L2:           inc     edi
                mov     ebx, [ecx + AC_Status.pk_ptr]   ; load remaining values
                ret
endp

                ; Внутренняя процедура, обновить счётчики и битовую маску
                ; IN    edx = кодируемый бит
proc            AC_Helper_Update
                                                        ; in eax calculate offset to freq[bits]
                lea     eax, [ecx + AC_Status.freq + ebp*sizeof.AC_Freq_Item]
                inc     dword [eax + edx * 4]           ; inc bit0 or bit1
                shl     ebp, 1
                or      ebp, edx
                and     ebp, AC_MODEL_MASK                
                ret
endp
                
proc            AC_Helper_Init
                mov     [ecx + AC_Status.pk_ptr], eax   ; init vars
                mov     [ecx + AC_Status.pk_ptr_orig], eax
                ;
                push    ecx                             ; init freq0 & freq1
                lea     edi, [ecx + AC_Status.freq]     
                mov     ecx, AC_MODEL_COUNT * 2
                xor     eax, eax
                inc     eax
                rep     stosd
                pop     ecx
                ;
                xor     esi, esi
                xor     ebp, ebp
                mov     edi, 1000000h
                jmp     AC_Helper_Store.storeMMB
endp

proc            AC_Helper_Store
                mov     [ecx + AC_Status.pk_ptr], ebx
 .storeMMB:     mov     [ecx + AC_Status.min], esi
                mov     [ecx + AC_Status.max], edi
                mov     [ecx + AC_Status.bits], ebp
                ret
endp

                ; Удалена, т.к. не приводит к уменьшению кода
                ; (сказывается команда stc и повтор условия jae после вызова процедуры)
;                ; Внутренняя процедура, выполняем начальную проверку цикла
;proc            AC_Helper_WhileCheck
;                mov     eax, edi                        ;  while (Max-Min) < 256 do     
;                sub     eax, esi                        ;  begin                        
;                cmp     eax, 256                        
;                jae     .L1
;                sub     edi, edx                        ;    if value = 1 then Dec(Max);
;                stc
; .L1:           ret
;endp

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

;=== ENCODER ==================================================================

                ; Начинаем сжатие
                ; IN    eax = адрес для сжатых данных
                ; OUT   -
proc            AC_Encoder_Start
                pushad
                mov     ecx, ac_status
                call    AC_Helper_Init
                popad
                ret
endp

                ; Закодировать 1 бит
                ; IN    edx = значение бита (берётся только 0-й бит, остальные игнорируются)
                ; OUT   -
proc            AC_Encoder_EncodeBit
                pushad
                mov     ecx, ac_status
                and     edx, 1                          ; isolate most low bit
                push    edx                             ; store bit, because AC_HelperLo... modify edx
                call    AC_Helper_LoadAndPredict        ; Load values and calculate Mid
                pop     edx
                or      edx, edx                        ;  if value = 1 then Min := Mid 
                jz      .L0                             ;               else Max := Mid;
                mov     esi, eax
                jmp     .L1
 .L0:           mov     edi, eax
 .L1:           call    AC_Helper_Update
 .LoopWhile:    mov     eax, edi                        ;  while (Max-Min) < 256 do     
                sub     eax, esi                        ;  {
                cmp     eax, 256                        
                jae     .L2
                sub     edi, edx                        ;    if value = 1 then Dec(Max);
                mov     eax, esi                        ;    buffer[bufferIndex] := Min SHR 16;
                shr     eax, 16                         ;    inc(bufferIndex);                 
                mov     [ebx], al
                inc     ebx
                call    AC_Helper_UpdateMinMax
                jmp     .LoopWhile                      ;  } // while
 .L2:           call    AC_Helper_Store                 ; store values
                popad
                ret
endp

                ; Завершаем сжатие
                ; IN    -
                ; OUT   eax = размер сжатых данных
proc            AC_Encoder_End uses ecx edi
                mov     ecx, ac_status
                mov     edi, [ecx + AC_Status.pk_ptr]
                mov     eax, [ecx + AC_Status.min]
                bswap   eax                             ; OutputByte( Min SHR 16)
                shr     eax, 8                          ; OutputByte( Min SHR 8) 
                stosd                                   ; OutputByte( Min )      
                dec     edi
                sub     edi, [ecx + AC_Status.pk_ptr_orig]
                xchg    eax, edi
                ret
endp

;=== DECODER ==================================================================

                ; Инициализация декодера
                ; IN    eax = адрес сжатых данных
proc            AC_Decoder_Start
                pushad
                mov     ebx, [eax]                      ; Инициализация val
                bswap   ebx                             ; загружаем три байта из сжатых данных: 
                shr     ebx, 8                          ; eax = 00 | [eax+0] | [eax+1] | [eax+2]
                add     eax, 3
                ;
                mov     ecx, ac_status
                mov     [ecx + AC_Status.val], ebx
                call    AC_Helper_Init
                popad
                ret
endp

                ; Декодировать 1 бит из потока
                ; IN    -
                ; OUT   eax = прочитанный бит
proc            AC_Decoder_DecodeBit
                pushad
                mov     ecx, ac_status
                call    AC_Helper_LoadAndPredict        ; Load and Calculate mid
                xor     edx, edx
                cmp     [ecx + AC_Status.val], eax      ;  if( Val >= Mid ) {  result := 1;
                jb      .L0                             ;                      Min := Mid;
                mov     esi, eax                        ;                   } else {        
                inc     edx                             ;                      result := 0;
                jmp     .L1                             ;                      Max := Mid; }
 .L0:           mov     edi, eax
 .L1:           call    AC_Helper_Update                ;  Model.update(result);
 .LoopWhile:    mov     eax, edi                        ;  while (Max-Min) < 256 do     
                sub     eax, esi                        ;  {
                cmp     eax, 256                        
                jae     .L2
                sub     edi, edx                        ;    if value = 1 then Dec(Max);
                movzx   eax, word [ecx + AC_Status.val] ; Val = ((Val SHL 8) AND $FFFF00) OR InputByte();
                shl     eax, 8
                or      al, [ebx]
                inc     ebx
                mov     [ecx + AC_Status.val], eax
                call    AC_Helper_UpdateMinMax
                jmp     .LoopWhile                      ;  } // while
 .L2:           call    AC_Helper_Store
                mov     [esp + pushad_data.r_eax], edx  ; store result
                popad
                ret
endp

                ; Процедура не требуется на самом деле, т.к. нечего выполнять в конце декодирования
proc            AC_Decoder_End
                ret
endp

AC2_END:
