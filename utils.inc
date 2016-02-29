;=== Циклы ===
                ; Аналог loop label, но с другими регистрами
macro           loopr   op1, label
{
        if op1 in <ecx>
                loop    label
        else
                dec     op1
                jnz     label
        end if
}

macro           loopb   regs1, regs2, label
{
                inc     regs1
                cmp     regs1, regs2
                jb      label
}

;=== Переход по диапазону ===
                ; if( (op1 >= low) && (op1 <= high) ) goto label
macro           br      op1, low, high, label
{
                local   .L1
                cmp     op1, low
                jb      .L1
                cmp     op1, high
                jbe     label
 .L1:
}

;=== Сравнение + условный переход ===
macro           ba      op1, op2, label
{
                cmp     op1, op2
                ja      label
}

macro           bb      op1, op2, label
{
                cmp     op1, op2
                jb      label
}

macro           be      op1, op2, label
{
                cmp     op1, op2
                je      label
}

macro           bne     op1, op2, label
{
                cmp     op1, op2
                jne     label
}

macro           bz      op1, label
{
                or      op1, op1
                jz      label
}

macro           bnz     op1, label
{
                or      op1, op1
                jnz      label
}

macro           bs      op1, label
{
                or      op1, op1
                js      label
}

macro           bns     op1, label 
{
                or      op1, op1
                jns     label 
}

;=== Проверки флагов ===
macro           fset?   op1, op2, label
{
                test    op1, op2
                jnz     label
}

macro           fclear? op1, op2, label
{
                test    op1, op2
                jz      label
}

;=== Возврат значений из функций ===
macro           return  value
{
                mov     eax, value
                ret
}

;=== Загрузка значений ===
                ; if (reg == 0) then reg = value
macro           default reg, value
{               local   .L1
                bnz     reg, .L1
                mov     reg, value
 .L1:
}

;=== Переключатели ===
macro           switch  reg, [code, label]
{
        forward
                cmp     reg, code
                je      label
}

;=== Строковые операции ===
macro           BLKOP   opr, count
{               local   count1, count2
count1  = count/4
count2  = count mod 4
                if count1 <> 0
                        mov     ecx, count1
                        rep     opr#sd  ; dword
                end if
                if count2 <> 0
                        if count1 <> 0
                                mov     cl, count2
                        else
                                mov     ecx, count2
                        end if
                        rep     opr#sb  ; byte
                end if
}

macro           MOVEBLK source, destination, count
{
                mov     esi, source
                mov     edi, destination
                BLKOP   mov, count
}

macro           CMPBLK  source, destination, count
{
                mov     esi, source
                mov     edi, destination
                BLKOP   cmp, count
}

;=== Работа с символами Unicode
                ; SKBCHS - SKip Back CHarS
macro           trimu   regBase, regIndex, char
{               local   .L1, .L2
 .L1:           or      regIndex, regIndex
                jz      .L2
                dec     regIndex
                cmp     word [regBase + regIndex*2], char
                je      .L1
                inc     regIndex
 .L2:
}

                ; STCHS - STore CHarS
macro           stosu   [chrs]
{
                mov     ax, chrs
                stosw
}

                ; MVCHS - MoVe CHarS
macro           movsu   source
{
                local   .L1
        if ~ source eq
                mov     esi, source
        end if
 .L1:           lodsw
                stosw
                or      ax,ax
                jnz     .L1
                dec     edi
                dec     edi
}
