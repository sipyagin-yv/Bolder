                .const
SKA_SwitchTable:; ОСНОВНЫЕ КОДЫ
                dd      0               ; 00 =Пустота                        - НЕТ ОБРАБОТКИ
                dd      SKA1.SM1        ; 01 =Камень - падает, скатывается
                dd      SKA1.SM2        ; 02 =Алмаз - падает, скатывается
                dd      0               ; 03 =Земля                          - НЕТ ОБРАБОТКИ
                dd      0               ; 04 =Стены кирпичные, пробиваемые   - НЕТ ОБРАБОТКИ
                dd      SKA1.UPR        ; 05 =Человечек - передвигается
                dd      SKA1.SM6        ; 06 ="Чудище"
                dd      0               ; 07 =Стены бетонные, непробиваемые  - НЕТ ОБРАБОТКИ
                ; ОБРАБОТКА ПРОМЕЖУТОЧНЫХ ЭЛЕМЕНТОВ
                dd      SKA1.SM8        ; 08 - ??? - спрайт - алмаз, но нигде не используется
                dd      0               ; 09 - ??? - спрайт - пустота, но нигде не используется
                dd      0               ; 10 - нижняя часть алмаза, падающего вниз
                dd      SKA1.SM11       ; 11 - верхняя часть алмаза, падающего вниз
                dd      0               ; 12 - ???
                dd      0               ; 13 - ???
                dd      0               ; 14 - ???
                dd      0               ; 15 - нижняя часть камня, падающего вниз
                dd      SKA1.SM16       ; 16 - верхняя часть камня, падающего вниз
                dd      SKA1.UPR        ; 17 - человечек бежит ВВЕРХ, закончил движение
                dd      0               ; 18 - человечек бежит ВВЕРХ, откуда / бежит ВНИЗ, куда
                dd      SKA1.SM19       ; 19 - человечек бежит ВНИЗ, откуда
                dd      SKA1.SM20       ; 20 - человечек бежит ВВЕРХ, куда      ;(код 21 меняется на 20, из-за обработки
                dd      SKA1.SM21       ; 21 - человечек бежит ВВЕРХ, куда      ; лабиринта снизу вверх)
                dd      0               ; 22 - человечек бежит ВПРАВО, откуда
                dd      SKA1.SM23       ; 23 - человечек бежит ВПРАВО, куда
                dd      SKA1.UPR        ; 24 - человечек бежит ВПРАВО, закончил движение
                dd      0               ; 25 - человечек бежит ВЛЕВО, куда
                dd      SKA1.SM26       ; 26 - человечек бежит ВЛЕВО, откуда
                dd      SKA1.UPR        ; 27 - человечек бежит ВЛЕВО, закончил движение
                dd      SKA1.SM28       ; 28
                dd      SKA1.SM29       ; 29
                dd      SKA1.SM30       ; 30
                dd      0               ; 31
                dd      0               ; 32
                dd      SKA1.SM33       ; 33
                dd      SKA1.SM34       ; 34
                dd      0               ; 35
                dd      SKA1.SM36       ; 36
                dd      SKA1.SM37       ; 37
                dd      SKA1.SM38       ; 38
                dd      SKA1.SM39       ; 39
                dd      SKA1.SM40       ; 40
                dd      SKA1.SM41       ; 41

                .code
                ; ОБРАБОТКА ЛАБИРИНТА!!!
proc            SKA1
                mov     [PSK],0                 ; Флаг движения
                mov     [HeroDie],FALSE
                mov     [SoundsCount],0         ; Кол-во звуковых эффектов
                mov     esi,Maze+191            ; +191 перемещаемся в правую самую нижнюю ячейку
                mov     bl,MAZE_HEIGHT
 .LoopY:        mov     bh,MAZE_WIDTH
 .LoopX:        movzx   eax, byte [esi]
                cmp     al,41
                ja      .NextCell
                mov     eax, [SKA_SwitchTable + eax*4]
                or      eax, eax
                jz      .NextCell
                jmp     eax
 .MovedAndNextCell:
                mov     [PSK],1                 ; Хоть что-то двинулось...
 .NextCell:     dec     esi
                dec     bh
                jnz     .LoopX
                dec     bl
                jnz     .LoopY
                call    MakeSoundEffects
                stdcall DrawLevel, Maze
                ret

 .SM8:          mov     byte [esi],2            ; ?????????????
                jmp     .NextCell

                ;=== КАМЕНЬ ===

                ; Камень падает вниз
 .SM1:          bne     byte [esi+16], 0, .SR0  ; КАМЕНЬ: Можем упасть?
                mov     byte [esi],16           ; КАМЕНЬ: Падаем вниз...
                mov     byte [esi+16],15
                jmp     .MovedAndNextCell

                ; Камень скатывается вправо
 .SR0:          be      bh, 16, .SR1            ; КАМЕНЬ: Есть куда скатиться вправо?         
                mov     al,[esi +1]             ; Если крайняя правая ячейка, ничего не делаем
                or      al,[esi+16 +1]
                jnz     .SR1
                mov     byte [esi +1],1         ; КАМЕНЬ: Катимся вправо...
                mov     byte [esi],0
                jmp     .MovedAndNextCell

                ; Камень скатывается влево
 .SR1:          be      bh, 1, .NextCell        ; КАМЕНЬ: Есть куда скатиться влево?
                mov     al,[esi -1]
                or      al,[esi+16 -1]
                jnz     .NextCell
                mov     byte [esi -1],1         ; КАМЕНЬ: Катимся влево...
                mov     byte [esi],0
                dec     esi                     ; Сдвигаемся влево, пропуская камень, иначе камень
                dec     bh                      ; очень быстро будет "скатываться" вправо.
                jmp     .MovedAndNextCell
                
                ; Камень - завершает падение вниз
 .SM16:         mov     byte [esi],0
                mov     byte [esi+16],1
                call    .BSL
                jmp     .MovedAndNextCell

                ;=== АЛМАЗ ===

                ; Алмаз падает вниз
 .SM2:          bne     byte [esi+16], 0, .SR2  ; АЛМАЗ: Может упасть?
                mov     byte [esi],11           ; АЛМАЗ: Падает вниз...
                mov     byte [esi+16],10
                jmp     .MovedAndNextCell

                ; Алмаз скатывается вправо
 .SR2:          be      bh,16,.SR3              ; АЛМАЗ: Скатиться вправо?
                mov     al,[esi +1]
                or      al,[esi+16 +1]
                jnz     .SR3
                mov     byte [esi +1],2         ; АЛМАЗ: Катимся вправо...
                mov     byte [esi],0
                jmp     .MovedAndNextCell

                ; Алмаз скатывается влево
 .SR3:          be      bh, 1, .NextCell        ; АЛМАЗ: Скатиться влево?
                mov     al,[esi -1]
                or      al,[esi+16 -1]
                jnz     .NextCell
                mov     byte [esi -1],2         ; АЛМАЗ: Катимся влево...
                mov     byte [esi],0
                dec     esi                     ; Пропускаем левую ячейку, иначе алмаз будем очень
                dec     bh                      ; быстро двигаться влево
                jmp     .MovedAndNextCell

                ; Алмаз - завершает падение вниз
 .SM11:         mov     byte [esi],0            ; АЛМАЗ: Падение закончилось
                mov     byte [esi+16],2
                call    .BSL                    ; Проверка - кого прибили...
                jmp     .MovedAndNextCell

                ;=== ЧЕЛОВЕЧЕК ===

                ; Временный код 21 меняется на 20
 .SM21:         mov     byte [esi],20           ; меняем 21 на 20, так как
                jmp     .NextCell               ; обработка уровня идёт снизу
                                                ; вверх, и если сразу поставить
                                                ; 20 то человечек будет бежать
                                                ; вверх со скоростью света.

                ; Человечек завершает движения вниз
 .SM19:         mov     byte [esi],0
                mov     byte [esi+16],17
                jmp     .NextCell

                ; Человечек завершает движение вверх
 .SM20:         mov     byte [esi],17
                mov     byte [esi+16],0
                jmp     .NextCell

                ; Человечек завершает перемещение вправо
 .SM23:         mov     byte [esi],24
                mov     byte [esi -1],0
                jmp     .NextCell

                ; Человечек завершает движение влево
 .SM26:         mov     byte [esi],0
                mov     byte [esi -1],27
                dec     esi                     ; Пропускаем левую ячейку
                dec     bh
                jmp     .NextCell

                ; Начало движения человечка, в зависимости от нажатой клавиши
 .UPR:          mov     [HeroCoord],esi         ; Координаты человека
                push    esi
                mov     eax,[KOD]
                switch  eax, KBD_MOVE_LEFT,  .U1L, \
                             KBD_MOVE_RIGHT, .U1R, \
                             KBD_MOVE_UP,    .U1U, \
                             KBD_MOVE_DOWN,  .U1D, \
                             KBD_DESTRUCT,   .SelfDestruct
                pop     esi
                mov     byte [esi],5            ; Код человечка
                jmp     .NextCell

                ; Человечек начинает движение влево
 .U1L:          mov     edx,-1
                call    .DWR
                mov     byte [esi],26
                mov     byte [esi -1],25
                pop     esi
                dec     esi                     ; Пропускаем ячейки слева, иначе человечек будет бежать быстро
                dec     bh
                jmp     .NextCell

                ; Человечек начинает движение вправо
 .U1R:          mov     edx,1
                call    .DWR
                mov     byte [esi],22
                mov     byte [esi +1],23
                pop     esi
                jmp     .NextCell

                ; Человечек начинает движение вверх
 .U1U:          mov     edx,-16
                call    .DWR
                mov     byte [esi-16],21
                mov     byte [esi],18
                pop     esi
                jmp     .NextCell

                ; Человечек начинает движение вниз
 .U1D:          mov     edx,16
                call    .DWR
                mov     byte [esi],19
                mov     byte [esi+16],18
                pop     esi
                jmp     .NextCell

                ; Человечек самоуничтожается
 .SelfDestruct: push    ebx
                sub     esi,17
                dec     bl
                dec     bh
                call    .UBR1                   ; Расставляем алмазы на области 3*3
                pop     ebx
                pop     esi
                jmp     .NextCell

                ;

 .SM29:         mov     byte [esi],28
                mov     byte [esi -1],0
                jmp     .NextCell

 .SM30:         mov     byte [esi],0
                mov     byte [esi -1],28
                dec     esi
                dec     bh
                jmp     .NextCell

 .SM33:         mov     byte [esi],0
                mov     byte [esi+16],28
                jmp     .NextCell

 .SM34:         mov     byte [esi],28
                mov     byte [esi+16],0
                jmp     .NextCell

 .SM40:         mov     byte [esi],34
                jmp     .NextCell

 .SM36:         mov     byte [esi],6
                mov     byte [esi -1],0
                jmp     .NextCell

 .SM37:         mov     byte [esi],0
                mov     byte [esi -1],6
                dec     esi
                dec     bh
                jmp     .NextCell

 .SM38:         mov     byte [esi],0
                mov     byte [esi+16],6
                jmp     .NextCell

 .SM39:         mov     byte [esi],6
                mov     byte [esi+16],0
                jmp     .NextCell

 .SM41:         mov     byte [esi],39
                jmp     .NextCell



                ; DWR - проверка на движение камней человечком
 .DWR:          push    esi
                push    ebx
                call    .PROW
                add     esi,edx
                mov     al,[esi]
                be      al,2,.ZWK
                be      al,1,.SDW
                be      al,3,.UED
                be      al,0,.UED
                pop     ebx
                pop     esi
                pop     eax                      ; Адрес возврата
                pop     esi
                jmp     .NextCell

 .ZWK:          inc     [SoundsCount];call    SOO
                dec     [AlmazesCount]
                add     [OBK],SCORE_TREASURE
 .UED:          pop     ebx
                pop     esi
                ret

 .SDW:          call    .PROW           ; СДВИГ КАМНЕЙ!!!
                add     esi,edx
                mov     al,[esi]
                be      al,1,.SDW
                be      al,0,.UEDO
                pop     ebx
                pop     esi
                pop     eax             ; Адрес возврата
                pop     esi
                jmp     .NextCell

                ; Проверка на пределы уровня
 .PROW:         mov     eax,[KOD]
                switch  eax, KBD_MOVE_LEFT,  .R1L, \
                             KBD_MOVE_RIGHT, .R1R, \
                             KBD_MOVE_UP,    .R1U, \
                             KBD_MOVE_DOWN,  .R1D
                ret
 .R1L:          dec     bh
                jz      .EP2
                ret
 .R1R:          inc     bh
                be      bh,17,.EP2
                ret
 .R1U:          dec     bl
                jz      .EP2
                ret
 .R1D:          inc     bl
                be      bl,13,.EP2
                ret
 .EP2:          pop     eax             ; Адрес возврата
                pop     ebx
                pop     esi
                pop     eax             ; Адрес возврата
                pop     esi
                jmp     .NextCell

 .UEDO:         mov     byte [esi],1
                pop     ebx
                pop     esi
                ret

                ; Проверка: грохнул ли кого камень?
 .BSL:          mov     al,[esi+16+16]                   ; Смотрим на 2 строчки ниже
                be      al,5,.UB2
                be      al,17,.UB2
                be      al,24,.UB2
                be      al,27,.UB2
                be      al,6,.UB2
                be      al,28,.UB2
                bne     al,0,.L1
                ret
 .L1:           cmp     al,5
                jc      .ZW
                ret
 .ZW:           inc     [SoundsCount];call    SOO
                ret

 .UB2:          push    ebx
                push    esi
                add     esi,15
                dec     bh
                inc     bl
                call    .UBR1
                pop     esi
                pop     ebx
                ret

                ; ВЗРЫВ!!!
 .UBR1:         inc     [SoundsCount];call    SOO
 .TRM:          call    .PKR
                inc     esi
                inc     bh
                call    .PKR
                inc     esi
                inc     bh
                call    .PKR
                add     esi,16
                inc     bl
                call    .PKR
                dec     esi
                dec     bh
                call    .PKR
                dec     esi
                dec     bh
                call    .PKR
                add     esi,16
                inc     bl
                call    .PKR
                inc     esi
                inc     bh
                call    .PKR
                inc     esi
                inc     bh
                call    .PKR
                inc     [SoundsCount];call    SOO
                ret

 .PKR:          be      bh,17,.Ex
                be      bh,0,.Ex
                be      bl,13,.Ex
                be      bl,0,.Ex
                mov     al,[esi]
                be      al,7,.Ex                ; бетон непробиваем
                cmp     al,29
                jnc     .Ex
                be      al,28,.ZP
                be      al,2,.Ex                ; уже алмаз
                be      al,5,.PP                ; человечек - взрывается
                cmp     al,17
                jnc     .PP
                cmp     al,7
                jnc     .Ex
                jmp     .ZP
 .Ex:           ret
 .ZP:           mov     byte [esi],2
                inc     [AlmazesCount]
                ret
 .PP:           mov     [HeroDie],TRUE          ; ЧЕЛОВЕЧЕК Прихлопнут :(
                jmp     .ZP


                ;=== ЧУДИЩЕ ===

                ;
 .SM6:          push    ebx
                push    esi
                cmp     esi,[HeroCoord]
                jc      .MM0_
 .SE1:          dec     esi
                dec     bh
                be      bh,0,.SE2
                mov     al,[esi]
                cmp     al,28
                jnc     .SE2
                be      al,5,.UBR2
                cmp     al,17
                jnc     .UBR2
                bne     al,0,.SE2
                mov     byte [esi],31
                inc     esi
                mov     byte [esi],37
                pop     esi
                dec     esi
                pop     ebx
                dec     bh
                jmp     .NextCell
 .SE2:          sub     esi,15
                inc     bh
                dec     bl
                be      bl,0,.E5_
                mov     al,[esi]
                cmp     al,28
                jnc     .E5_
                be      al,5,.UBR2
                cmp     al,17
                jnc     .UBR2
                bne     al,0,.E5_
                mov     byte [esi],41
                pop     esi
                mov     byte [esi],32
                pop     ebx
                jmp     .NextCell
 .E5_:          pop     esi
                pop     ebx
                mov     byte [esi],28
                jmp     .NextCell
 .MM0_:         inc     esi
                inc     bh
                be      bh,17,.SQ0
                mov     al,[esi]
                cmp     al,28
                jnc     .SQ0
                be      al,5,.UBR2
                cmp     al,17
                jnc     .UBR2
                bne     al,0,.SQ0
                mov     byte [esi],36
                pop     esi
                mov     byte [esi],31
                pop     ebx
                jmp     .NextCell
 .SQ0:          add     esi,15
                dec     bh
                inc     bl
                be      bl,13,.E6_
                mov     al,[esi]
                cmp     al,28
                jnc     .E6_
                be      al,5,.UBR2
                cmp     al,17
                jnc     .UBR2
                bne     al,0,.E6_
                mov     byte [esi],32
                pop     esi
                mov     byte [esi],38
                pop     ebx
                jmp     .NextCell
 .E6_:          pop     esi
                pop     ebx
                mov     byte [esi],28
                jmp     .NextCell

 .SM28:         push    ebx
                push    esi
                cmp     esi,[HeroCoord]
                jnc     .MM1_
 .SE3:          dec     esi
                dec     bh
                be      bh,0,.SE4
                mov     al,[esi]
                cmp     al,28
                jnc     .SE4
                be      al,5,.UBR2
                cmp     al,17
                jnc     .UBR2
                bne     al,0,.SE4
                mov     byte [esi],31
                pop     esi
                mov     byte [esi],30
                dec     esi
                pop     ebx
                dec     bh
                jmp     .NextCell
 .SE4:          sub     esi,15
                inc     bh
                dec     bl
                be      bl,0,.E7_
                mov     al,[esi]
                cmp     al,28
                jnc     .E7_
                be      al,5,.UBR2
                cmp     al,17
                jnc     .UBR2
                bne     al,0,.E7_
                mov     byte [esi],40
                pop     esi
                mov     byte [esi],32
                pop     ebx
                jmp     .NextCell
 .E7_:          pop     esi
                pop     ebx
                mov     byte [esi],6
                jmp     .NextCell
 .MM1_:         inc     esi
                inc     bh
                be      bh,17,.SQ1
                mov     al,[esi]
                cmp     al,28
                jnc     .SQ1
                be      al,5,.UBR2
                cmp     al,17
                jnc     .UBR2
                bne     al,0,.SQ1
                mov     byte [esi],29
                pop     esi
                mov     byte [esi],31
                pop     ebx
                jmp     .NextCell
 .SQ1:          add     esi,15
                dec     bh
                inc     bl
                be      bl,13,.E8_
                mov     al,[esi]
                cmp     al,28
                jnc     .E8_
                be      al,5,.UBR2
                cmp     al,17
                jnc     .UBR2
                bne     al,0,.E8_
                mov     byte [esi],32
                pop     esi
                mov     byte [esi],33
                pop     ebx
                jmp     .NextCell
 .E8_:          pop     esi
                pop     ebx
                mov     byte [esi],6
                jmp     .NextCell

 .UBR2:         sub     esi,17
                dec     bh
                dec     bl
                call    .TRM
                pop     esi
                pop     ebx
                jmp     .NextCell
endp ; SKA1
