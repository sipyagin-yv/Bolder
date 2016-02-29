                .udata
crc32table      rd      256             ; для вычисления crc32

                .code
                ;
                ; initialize table for calculating crc32
                ;
proc            crc32InitTable uses eax edx
                xor     eax, eax
 .L1:           push    eax
                mov     dl, 8
                ; cycle by bits
 .L3:           shr     eax, 1
                jnc     .L2
                xor     eax, 0EDB88320h
 .L2:           dec     dl
                jnz     .L3
                ;
                mov     edx, eax
                pop     eax
                mov     [crc32table+eax*4], edx
                inc     al
                jnz     .L1
                ret
endp

                ;
                ; Calculate crc32 of block
                ;
                ; IN    esi - address of block
                ;       ecx - length of block
                ; OUT   eax - calculated crc32
                ;
proc            crc32calc uses esi ebx ecx
                mov     eax, -1
 .L1:           xor     al, [esi]
                inc     esi
                movzx   ebx, al
                shr     eax, 8
                xor     eax, [crc32table + ebx*4]
                loop    .L1
                not     eax
                ret
endp
