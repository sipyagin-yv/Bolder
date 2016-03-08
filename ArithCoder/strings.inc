macro           movsz
{               local   .L1
 .L1:           lodsw
                stosw
                or      ax, ax
                jnz     .L1
                dec     edi
                dec     edi
}
