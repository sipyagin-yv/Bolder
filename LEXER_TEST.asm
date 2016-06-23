; Это проба лексера

                ;
                ; test_proc (created 03.04.2016 20:52:34)
                ; Proc for testing purposes
                ;
                ; IN        -
                ;
                ; OUT        -
                ;
proc            test_proc       param1:dword, param2:dword
                        

                call    test_proc
                
                call    .testproc

                xor     eax, eax
                jmp     .L1
                
LabelTTT:                
                
L0:                
                
 L1:
 
 .Local1:
                 
                ret        
endp


;note
