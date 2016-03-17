                .udata
                ; Структура для определения рабочего экрана 
                ; (аналог консоли, но свой, со своими символами)
                ; 
                ; Существует 3 экрана:
                ;   1. scr.screen      - Экран программы (текстовый)
                ;   2. scr.cachescreen - Экран программы, кешированный (текстовый)
                ;   3. scr.vmem        - Экран вывода (графический)
struct          Scr
enable          rd      1                               ; =0 - рисование запрещено; >0 - рисование разрешено
dc              rd      1                               ; Контекст для рисования
dib             rd      1                               ; Bitmap для рисования
vmem            rd      1                               ; Битовые данные Bitmap(dib)
pixels          POINT                                   ; Размеры экрана в пикселах
chars           POINT                                   ; Размеры экрана в символах
pitch           rd      1                               ; Размер скан-строки, в байтах
screen          rw      SCREEN_WIDTH*SCREEN_HEIGHT      ; Рабочий экран
cachescreen     rw      SCREEN_WIDTH*SCREEN_HEIGHT      ; Экран, с нарисованными в mem символами
translADDR      rd      SCREEN_WIDTH*SCREEN_HEIGHT      ; Таблица для вычисления адреса по смещению символа
translCH        rd      256                             ; Таблица для вычисления адреса маски символа
palitre         rd      16                              ; Палитра
bih             BITMAPINFOHEADER                        ; Информация о картинке (vmem - адрес битовых данных картинки)
fontdata2       db      256*CHAR_HEIGHT dup(?)          ; "Распакованный" шрифт
ends
scr             Scr

MainWindowClass rd      1
MainWindow      rd      1

                ; System constants and structures
DIB_RGB_COLORS          =       0
;ERROR_FILE_NOT_FOUND    =       2

                .const
                ; Сообщения об ошибках
ErrorCreateMainClass    du      'Can''t register window class', 0
ErrorCreateMainWindow   du      'Can''t create window', 0
ErrorCreateImageDC      du      'Can''t create image (DC)', 0
ErrorCreateImageDIB     du      'Can''t create image (DIB)', 0
                ; Проводимые действия
ActionRegisterClass     du      'Register class', 0
ActionCreateWindow      du      'Create window', 0
ActionCreateCompDC      du      'Create compatible DC', 0
ActionCreateDIBSect     du      'Create DIB section', 0

                .code
proc            CreateScreen    wchar:DWORD, hchar:DWORD
                invoke  RtlZeroMemory, scr, sizeof.Scr
                ;
                xor     eax, eax
                mov     [scr.dc], eax
                mov     [scr.dib], eax
                ;
                mov     eax, [wchar]
                mov     [scr.chars.x], eax
                imul    eax, CHAR_WIDTH         ; Ширина символа
                mov     [scr.pixels.x], eax
                ;
                mov     eax, [hchar]
                mov     [scr.chars.y], eax
                imul    eax, CHAR_HEIGHT        ; Высота символа
                mov     [scr.pixels.y], eax
                ;
                mov     eax, [scr.pixels.x]
                shl     eax, 2                  ; "размер" одного пиксела - 4 байта: "0 R G B"
                mov     [scr.pitch], eax
                ;
                ; Создать DC
                ;
                invoke  CreateCompatibleDC, 0
                mov     [scr.dc], eax
                or      eax, eax
                jnz     .L1

                SYSTEMERROR     ActionCreateCompDC
                stdcall ShowError, ErrorCreateImageDC, 0, 0

 .L1:           ;
                ; Создать bitmap, для рисования и вывода в окно
                ;
                invoke  RtlZeroMemory, scr.bih, sizeof.BITMAPINFOHEADER
                lea     edx, [scr.bih]
                mov     [edx + BITMAPINFOHEADER.biSize], sizeof.BITMAPINFOHEADER
                ;
                mov     eax, [scr.pixels.x]
                mov     [edx + BITMAPINFOHEADER.biWidth], eax
                ;
                mov     eax, [scr.pixels.y]
                neg     eax                             ; top-down, т.е. начало блока памяти - левый-верхний пиксел
                mov     [edx + BITMAPINFOHEADER.biHeight], eax
                ;
                mov     [edx + BITMAPINFOHEADER.biPlanes], 1
                mov     [edx + BITMAPINFOHEADER.biBitCount], 32
                mov     [edx + BITMAPINFOHEADER.biCompression], BI_RGB
                ;;mov     [edx + BITMAPINFOHEADER.biSizeImage], 0
                ;;mov     [edx + BITMAPINFOHEADER.biXPelsPerMeter], 0
                ;;mov     [edx + BITMAPINFOHEADER.biYPelsPerMeter], 0
                ;;mov     [edx + BITMAPINFOHEADER.biClrUsed], 0
                ;;mov     [edx + BITMAPINFOHEADER.biClrImportant], 0
                ;
                invoke  CreateDIBSection, \
                        [scr.dc], \
                        edx, \
                        DIB_RGB_COLORS, \
                        scr.vmem, \
                        NULL, 0
                mov     [scr.dib], eax
                or      eax, eax
                jnz     .L2

                SYSTEMERROR   ActionCreateDIBSect
                stdcall ShowError, ErrorCreateImageDIB, 0, 0

 .L2:           ;
                ; Установить размеры клиентской части окна 
                ; (вначале размер клиентской части равен размеру экрана)
                ;
                stdcall SetClientSize, [MainWindow], [scr.pixels.x], [scr.pixels.y]
                ;
                ; Распаковка шрифта
                ;
                call    PrepareFont
                ;
                ; Создать перекодировочную таблицу для вычисления адреса экрана
                ; addr = y * CHAR_HEIGHT * scr.pixels.x + x * CHAR_WIDTH
                ; offset = pixel * 4 + scr.mem
                ;
                lea     edi, [scr.translADDR]
                mov     ebx, 0                  ; for(y=0; y<scr.chars.y; y++)
                ;
.LoopY:         mov     ecx, 0                  ; for(x=0; x<scr.chars.x; x++)
                imul    eax, ebx, CHAR_HEIGHT   ; вынос подвыражения (y * CHAR_HEIGHT * scr.pixels.x) за пределы цикла
                imul    eax, [scr.pixels.x]
                ;
.LoopX:         imul    edx, ecx, CHAR_WIDTH
                add     edx, eax
                shl     edx, 2
                add     edx, [scr.vmem]
                mov     [edi], edx
                add     edi, 4
                ;
                loopb   ecx, [scr.chars.x], .LoopX
                loopb   ebx, [scr.chars.y], .LoopY
                ;
                ; Создать перекодировочную таблицу для вычисления адреса символа
                ;
                xor     ecx, ecx                ; for(ecx=0; ecx<256; ecx++)
 .tr2:          imul    eax, ecx, CHAR_HEIGHT
                add     eax, scr.fontdata2
                mov     [scr.translCH+ecx*4], eax
                inc     cl
                jnz     .tr2
                ;
                ; Инициализация экранов
                ;
                lea     edi, [scr.screen]
                mov     ecx, SCREEN_WIDTH*SCREEN_HEIGHT
                mov     ax, 0F20h
                rep     stosw
                ;
                lea     edi, [scr.cachescreen]
                mov     ecx, SCREEN_WIDTH*SCREEN_HEIGHT
                not     ax                      ; Чтобы в первый раз перерисовать в scr.vmem весь экран
                rep     stosw
                ;
                ; Подготовка палитры по умолчанию
                ;
                lea     esi, [defaultpalitre]
                lea     edi, [scr.palitre]
                mov     ecx, 16
                rep     movsd
                ;
                ; Разрешаем рисование
                ;
                mov     [scr.enable], -1
                ;
                ; Показать то, что скрыто
                ;
                invoke  ShowWindow, [MainWindow], SW_SHOWNORMAL
                ret
endp

proc            DeleteScreen
                invoke  DeleteObject, [scr.dib]
                invoke  DeleteDC, [scr.dc]
                ret
endp

proc            CreateMainWindow
                local   wc:WNDCLASS
                ;
                xor     eax, eax
                mov     [MainWindow], eax
                mov     [MainWindowClass], eax
                ;
                ; Регистрация класса
                ;
                invoke  RtlZeroMemory, addr wc, sizeof.WNDCLASS
                ;
                ;;mov     [wc.style], 0
                mov     [wc.lpfnWndProc], MainWindowProc
                ;;mov     [wc.cbClsExtra], 0              ; no extra class data
                ;;mov     [wc.cbWndExtra], 0              ; no extra window data

                invoke  GetModuleHandle, 0
                mov     [wc.hInstance], eax

                ;;mov     [wc.hIcon], NULL                ; NULL, no icon

                invoke  LoadCursor, 0, IDC_ARROW        ; Загружаем курсор по умолчанию
                mov     [wc.hCursor], eax

                ;;mov     [wc.hbrBackground], 0           ; =0, значит приложение само рисует клиентскую часть
                ;;mov     [wc.lpszMenuName], NULL         ; NULL, no menu
                mov     [wc.lpszClassName], MainWindowClassName

                invoke  RegisterClassW, addr wc
                mov     [MainWindowClass], eax
                test    eax, eax
                jnz     .L1
                
                SYSTEMERROR     ActionRegisterClass
                stdcall ShowError, ErrorCreateMainClass, 1
                
 .L1:           ;
                ; Создание главного окна (но пока не показываем его)
                ;
                invoke  CreateWindowEx, \
                        WS_EX_OVERLAPPEDWINDOW, \
                        MainWindowClassName, \
                        MainWindowTitle, \
                        WS_OVERLAPPEDWINDOW, \
                        CW_USEDEFAULT, 0, \
                        200, 200, \
                        0, 0, [wc.hInstance], 0
                mov     [MainWindow], eax
                test    eax, eax
                jnz     .L2

                SYSTEMERROR     ActionCreateWindow
                stdcall ShowError, ErrorCreateMainWindow, 0, 0

 .L2:           ret
endp

proc            DeleteMainWindow
                invoke  DestroyWindow, [MainWindow]
                invoke  UnregisterClass, MainWindowClassName, 0
                ret
endp

proc            SetClientSize   hWnd:DWORD, clientWidth:DWORD, clientHeight:DWORD
                local   clientRect:RECT
                local   windowRect:RECT
                ; Определяем размеры клиентской части окна
                invoke  GetClientRect, [MainWindow], addr clientRect
                ; Определяем размеры окна
                invoke  GetWindowRect, [MainWindow], addr windowRect
                ; Т.к. функции установки размеров клиентской части окна нет, то делаем следующее:
                ; нужно вычислить размер бордюров и прочих элементов, для этого нужно 
                ; от текущих размеров окна вычесть размер клиентской части, 
                ; w = windowRect.Right-windowRect.Left-clientRect.right
                ; h = windowRect.Bottom-windowRect.Top-clientRect.bottom
                ; и прибавить к w новый размер клиентской части
                ; w += clientWidth
                ; h += clientHeigh
                ; Полученные значения w, h есть размеры окна, у которого размер клиентской части
                ; будет clientWidth, clientHeight
                mov     eax, [windowRect.right]
                sub     eax, [windowRect.left]
                sub     eax, [clientRect.right]
                add     eax, [clientWidth]
                ;
                mov     ebx, [windowRect.bottom]
                sub     ebx, [windowRect.top]
                sub     ebx, [clientRect.bottom]
                add     ebx, [clientHeight]
                ;
                invoke  SetWindowPos, \
                        [MainWindow], \
                        0, \                            ; not used
                        0, 0, \                         ; not used
                        eax, ebx, \                     ; new width and height
                        SWP_NOMOVE + SWP_NOZORDER       ; меняем только размеры окна!
                ret
endp
