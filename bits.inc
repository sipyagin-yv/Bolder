struct          TBits
data            rd      1
buff            rd      1
count           rb      1
ends

                .udata

bits            TBits

                .code
                ; IN    eax = addr of bits data
proc            BitsInit
                mov     [bits.data], eax
                mov     [bits.buff], 0
                mov     [bits.count], 0
                ret
endp

proc            BitsFlush
                
endp

                ; IN    dl = bits count to put
                ;       eax = bits
proc            BitsPut uses eax ebx ecx
                lea     ebx, [bits]
                mov     cl, [ebx + TBits.count]
                shl     eax, cl
                or      [ebx + TBits.buff], eax
                add     cl, dl
                ;
 .L0:           cmp     cl, 8
                jb      .L1
                mov     eax, [ebx + TBits.data]
                mov     ch, byte [ebx + TBits.buff]
                mov     [eax], ch
                inc     eax
                mov     [ebx + TBits.data]
                shr     [ebx + TBits.buff], 8
                sub     cl, 8
                jmp     .L0
 .L1:           mov     [ebx + TBits.count]
                ret
endp
                
                ; IN    dl = bits count to get
                ; OUT   eax = readed bits
proc            BitsGet uses ebx ecx
                lea     ebx, [bits]
                ; Набираем биты, если их недостаточно
 .L0:           cmp     dl, [ebx + TBits.count]
                jbe     .L1
                ; Читаем байт из входного потока
                mov     eax, [ebx + TBits.data]
                inc     eax
                mov     [ebx + TBits.data], eax
                movzx   eax, byte [eax-1]
                ;
                mov     cl, [ebx + TBits.count]
                shl     eax, cl
                or      [ebx + TBits.buff], eax
                add     [ebx + TBits.count], 8
                jmp     .L0
                ; Готовим "маску" для чтения бит
 .L1:           mov     cl, dl
                mov     eax, 1
                shl     eax, cl
                dec     eax
                and     eax, [ebx + TBits.buff]
                ; Обновляем данные
                shr     [ebx + TBits.buff], cl
                sub     [ebx + TBits.count], cl
                ret
endp
