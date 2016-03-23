;==============================================================================;
;  work with resources                                                         ;
;==============================================================================;
RESOURCE_AREA_SIZE =    100000          ; Максимальный размер ресурсного файла

struct          ResourceMainHeader
magic           rd      1                       ; must be "BLDR" or other id
count           rd      1                       ; count of resources
size            rd      1                       ; size of resources
crc             rd      1                       ; crc32 of resources (exclude crc32)
reserv1         rd      1               ; reserv, must be 0
reserv2         rd      1               ; reserv, must be 0
reserv3         rd      1               ; reserv, must be 0
crcheader       rd      1                       ; crc32 of header
ends

struct          ResourceDataHeader
size            rw      1
ends

                .udata
_Resource       RB      RESOURCE_AREA_SIZE
resaddr         RD      1               ; Адрес ресурсов
reschanged      RB      1               ; Флаг измененного ресурса

                .code
                ;
                ; Init resource manager
                ;
                ; In:  EDI=address of resource area
                ;      EAX=count of resource entries
                ;
proc            InitResource
                pushad
                ; Инициализируем заголовок
                push    eax
                mov     esi, edi                        ; Для вычисления размера
                mov     ecx, sizeof.ResourceMainHeader  ; Для начала очищаем все поля заголовка
                xor     eax, eax
                rep     stosb
                pop     eax
                mov     edi, esi
                mov     [edi + ResourceMainHeader.magic], 'BLDR'
                mov     [edi + ResourceMainHeader.count], eax
                ; Инициализируем блоки данных
                add     edi, sizeof.ResourceMainHeader
 .L1:           mov     [edi + ResourceDataHeader.size], 0
                add     edi, sizeof.ResourceDataHeader
                dec     eax
                jnz     .L1
                sub     edi, esi
                mov     [esi + ResourceMainHeader.size], edi
                mov     [resaddr], esi
                mov     [reschanged], TRUE
                call    CalculateResourceCRC
                popad
                ret                
endp

                ; Get ptr to resource
                ;
                ; IN    edi - resource index
                ;
                ; OUT   esi - resource ptr
                ;       eax - size of resource
                ; 
                ; REMARK
                ;       eax = 0, if resource ptr is out of resource count
                ;
proc            GetResourcePtr uses edi
                xor     eax, eax
                mov     esi, [resaddr]
                cmp     edi, [esi + ResourceMainHeader.count]
                jae     .L1
                add     esi, sizeof.ResourceMainHeader
                ;
 .L0:           mov     ax, [esi + ResourceDataHeader.size]
                add     esi, sizeof.ResourceDataHeader
                dec     edi
                js      .L1
                add     esi, eax
                jmp     .L0
 .L1:           ret
endp

                ; put array [ESI] of length ECX into resource number EDI
                ; IN    esi = array to write to resource
                ;       ecx = length of array (in bytes)
                ;       edi = index of resource (from 0)
                ; OUT   -
proc            ReplaceResource
                pushad
                mov     ebx, [resaddr]                          ; ebx = адрес области данных с ресурсами
                cmp     edi, [ebx + ResourceMainHeader.count]   ; Проверка на максимальный номер ресурса
                jae     .Done
                push    esi ecx                                 ; Сохраним на время адрес и размер новых данных
                call    GetResourcePtr
                mov     edx, esi                                ; Сохраним адрес заменяемого ресурса
                add     esi, eax                                ; esi = Адрес ресурса, следующего за изменяемым
                ; Корректируем размеры 
                sub     eax, ecx                                ; eax = разница между старым и новым размером
                jz      .L1                                     ; Новый размер=старый размер, просто заменяем ресурс
                mov     ecx, [ebx + ResourceMainHeader.size]    ; ecx = сохраняем размер всего ресурса
                sub     [ebx + ResourceMainHeader.size], eax    ; корректируем размер всего ресурса
                mov     edi, esi
                sub     edi, eax                                ; edi = Куда перемещать ресурс, следующего за изменяемым
                ; Итак, двигаем [esi] -> [edi] by ecx bytes
                ; Вычисляем, сколько байт нужно сдвинуть
                ; ecx = ebx + (ecx - esi) 
                ; => ecx = адрес_начала_ресурсов + размер_ресурса - адрес_перемещаемого_ресурса
                add     ecx, ebx
                sub     ecx, esi
                jz      .L1                                     ; Переход, если ничего не нужно двигать
                ; Мы готовы, переместить esi -> edi by ecx
                cld                                             ; Перемещение от больших адресов к меньшим (edi <--- esi)
                cmp     esi, edi
                ja      .Move
                std                                             ; Перемещение от меньших адресов к большим (esi ---> edi)
                lea     esi, [esi+ecx-1]
                lea     edi, [edi+ecx-1]
 .Move :        rep     movsb                                   ; Двигаем блок
                cld                                             ; Восстановить флаг DF
 .L1:           pop     ecx esi                                 ; А теперь перемещаем новые данные
                mov     edi, edx
                mov     [edi - sizeof.ResourceDataHeader \      ; Новый размер
                             + ResourceDataHeader.size], cx     
                rep     movsb                                   ; Новые данные
                call    CalculateResourceCRC                    ; Пересчёт CRC32
                mov     [reschanged], TRUE                      ; Устанавливаем флаг измененных ресурсов
 .Done:         popad
                ret
endp

                ; Вычисляем контрольную сумму ресурсов
proc            CalculateResourceCRC
                pushad
                mov     ebx, [resaddr]
                mov     edx, sizeof.ResourceMainHeader
                
                ; Сначала вычисляем crc32 самих ресурсов
                lea     esi, [ebx + edx]
                mov     ecx, [ebx + ResourceMainHeader.size]
                sub     ecx, edx
                stdcall crc32calc
                mov     [ebx + ResourceMainHeader.crc], eax

                ; Затем вычисляем crc32 заголовка
                mov     esi, ebx
                xor     eax, eax
                mov     [ebx + ResourceMainHeader.crcheader], eax       ; crcheader не учитывает сам crcheader
                mov     ecx, edx
                stdcall crc32calc
                mov     [esi + ResourceMainHeader.crcheader], eax
                popad
                ret
endp

proc            CheckResourceCRC
                pushad
                mov     ebx, [resaddr]
                mov     edx, sizeof.ResourceMainHeader
        
                ; Сначала вычисляем crc32 заголовка
                mov     esi, ebx
                xor     edi, edi
                xchg    edi, [ebx + ResourceMainHeader.crcheader]
                mov     ecx, edx
                stdcall crc32calc
                cmp     eax, edi
                xchg    edi, [ebx + ResourceMainHeader.crcheader]
                jne     .error
                
                ; А потом вычисляем crc32 самих ресурсов
                lea     esi, [ebx + edx]
                mov     ecx, [ebx + ResourceMainHeader.size]
                sub     ecx, edx
                stdcall crc32calc
                cmp     eax, [ebx + ResourceMainHeader.crc]
                jne     .error
                clc
                popad
                ret

 .error:        stc
                popad
                ret
endp
