macro           SKIPBCK regBase, regIndex, ch
{
                local   .L1, .L2
 .L1:           or      regIndex, regIndex
                jz      .L2
                dec     regIndex
                cmp     word [regBase + regIndex*2], ch
                je      .L1
                inc     regIndex
 .L2:
}

macro           sz_st   [chrs]
{
                mov     ax, chrs
                stosw
}

macro           sz_movs source
{
                local   .L1
        if ~ source eq
                mov     esi, source
        end if
 .L1:           lodsw
                stosw
                or      ax,ax
                jnz     .L1
                sub     edi, 2
}
