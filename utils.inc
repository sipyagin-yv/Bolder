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
