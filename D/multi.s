section .data
    newline db 10, 0
    hex_format db "%02hhx", 0
    error_msg db "Invalid argument", 10, 0
    len db 0
    buf_size equ 600 
    STATE dd 0x001E
    MASK dd 0xffff
    x_struct db 5
    x_num db 0xaa, 1, 2, 0x44, 0x4f
    y_struct db 6
    y_num db 0xaa, 1, 2, 3, 0x44, 0x4f


section .bss
    buf resb buf_size
    

segment .text
    extern printf
    global add_multi
    extern malloc
    global PRmulti
    global get_multi
    extern puts
    extern fgets
    global randomNumber
    extern stdin
    global getMaxMin
    global main
    extern strlen
    global print_multi

 main:
    call argv_print_multi_loop
    push ebx
    call print_multi
    push eax
    call print_multi

    call add_multi
    add esp,8
    push eax
    call print_multi
    mov eax, 1     
    xor ebx, ebx   
    int 0x80       



        



PRmulti:
    push    ebp
    mov     ebp, esp
    sub     esp, 4
    pushad

random_len:
    call    randomNumber
    cmp     eax,0
    jle     random_len
    mov     ebx,eax
    push    ebx
    call    malloc
    pop ebx
    mov esi,eax
    mov byte [esi], bl
    mov ecx, 0

rand_multi:
    call randomNumber
    mov     byte [esi+ecx+1], al
    inc     ecx
    cmp     ecx,ebx
    jl      rand_multi
    mov     eax, esi
    mov     [ebp-4],eax
    popad
    mov     eax, [ebp-4]
    mov     esp, ebp
    pop     ebp
    ret




argv_print_multi_loop:
    push ebp     
    mov ebp, esp 
    
    mov eax, [ebp+12]  
    cmp eax,1
    je def_stru

    mov ebx, [ebp+16] 
    add ebx, 4
    mov edx, [ebx]    

    push edx       
    mov al, byte [edx] 
    cmp al, '-'       
    jne end_error  
    mov al, byte [edx+1] 
    cmp al, 'R'       
    je rand_cse  
    cmp al, 'I'       
    je in_cse

    call puts
    jmp invArg

    end_error:
        call puts      
        jmp invArg
    
    rand_cse:
        call PRmulti
        mov  ebx, eax
        call PRmulti
        jmp argv_print_multi_loop_end
    
    in_cse:
        call get_multi
        mov  ebx, eax
        call get_multi
        jmp argv_print_multi_loop_end

    def_stru:
        mov eax , x_struct
        mov ebx, y_struct
        jmp argv_print_multi_loop_end
    
    argv_print_multi_loop_end:
        mov esp, ebp  
        pop ebp       
        ret
    
    
   
    invArg:
        mov eax, 4
        mov ebx, 2
        mov ecx, error_msg
        mov edx, 17
        int 0x80
        mov eax, 1     
        xor ebx, ebx   
        int 0x80   



print_multi:
    jmp init

    end_print_multi:
        push newline
        call printf
        add esp, 4
        popad
        mov esp, ebp
        pop ebp
        ret
        
    init:
        push ebp
        mov ebp, esp
        pushad
        mov ebx,[ebp+8]
        mov al, byte [ebx]
        movzx ecx, al
        mov edx, 0

    jmp print_multi_loop

   

    print_multi_loop:
        cmp edx,ecx  
        jge end_print_multi
        mov esi,ecx
        sub esi,edx
        movzx eax, byte [ebx+esi] 
        pushad
        push eax
        push hex_format
        call printf
        add esp, 8
        popad
        add edx , 1 
        jmp print_multi_loop
    
        

get_multi:
    jmp init_get_multi1 

    get_multi_loop:
        cmp edx ,0
        jle get_multi_end
        push edx
        movzx eax, byte [ebx]
        call convert_char_to_hexa_get_multi
        mov ecx, eax
        shl ecx,4
        inc ebx

        jmp not_even

        converted:
        ret

        not_even:
        movzx eax, byte [ebx]
        call convert_char_to_hexa_get_multi
        add eax,ecx
        mov byte[esi+edx], al
        pop edx
        dec edx
        inc ebx
        jmp get_multi_loop

    init_get_multi1:
        push ebp
        mov ebp, esp
        sub esp, 4
        pushad
        jmp get_from_user

    


    use_malloc:
        dec  eax
        mov [len],eax   
        inc eax 
        push eax
        call malloc
        add esp,4
        mov esi, eax        
        mov ebx,buf
        mov edx, [len]
        shr edx, 1
        mov byte[esi], dl
    


    mov eax,[len]     
    mov ecx,0
    and eax,1
    cmp eax,0
    je get_multi_loop
    add edx, 1
    mov byte[esi], dl
    push edx
    jmp not_even


    
    get_from_user:
        mov eax, [stdin]                    
        push eax
        push dword buf_size
        push dword buf
        call fgets
        add esp, 12
        mov eax, buf
        push eax
        call strlen
        add esp, 4
        jmp use_malloc
    

    convert_char_to_hexa_get_multi:
        cmp eax, '9'
        jle numbers
        cmp eax, 'F'
        jle big_letters
        cmp eax, 'f'
        jle small_letters
    
    big_letters:
        sub eax, 'A'
        add eax, 10
        jmp converted

    small_letters:
        sub eax, 'a'
        add eax, 10
        jmp converted

    numbers:
        sub eax, '0'
        jmp converted
    
    get_multi_end:
        mov [ebp-4],esi
        popad
        mov eax, [ebp-4]
        mov esp, ebp
        pop ebp
        ret


getMaxMin:
    jmp init_getMaxMin
    init_getMaxMin:
        push    ebp                 
        mov     ebp, esp  
        movzx ecx, byte [eax] 
        movzx edx, byte [ebx]
        cmp ecx, edx
        jg end_getMaxMin 

    mov ecx, ebx
    mov ebx, eax
    mov eax, ecx

    end_getMaxMin:
        mov esp, ebp
        pop ebp
        ret  





add_multi:
    init_add_multi:
        push esp
        mov eax,[esp+12]         
        mov ebx,[esp+8]          
        call getMaxMin

    movzx edx,byte [eax]             
    inc edx                 
    push eax
    push ebx
    push edx
    call malloc
    pop edx
    mov esi,eax             
    mov byte[esi], dl
    pop ebx
    pop eax              
    mov edi, 0              
    movzx edx, byte[ebx]    
    
    mov ecx,0
    jmp minL 

    maxL:
        movzx  edx, byte[eax]
        push edx
        cmp edi,edx
        jge lastCry
        movzx edx, ch
        movzx ecx, byte[eax + edi +1]
        add  ecx,edx
        mov byte[esi + edi +1], cl
        pop edx
        inc edi
        jmp maxL 
           
    minL:
        push edx
        cmp edi,edx
        jge maxL
        movzx edx, ch
        movzx ecx, byte[eax + edi +1]
        add ecx, edx
        movzx edx, byte[ebx + edi +1]
        add ecx,edx
        mov byte[esi + edi +1], cl
        pop edx
        inc edi
        jmp minL
        
   

    lastCry:
        movzx ebx, byte[esi]
        mov byte[esi+ebx],ch
        mov ecx,[esi+ebx]
        cmp ecx,0
        je noC

    add_multi_end:
        mov eax,esi
        add esp,8
        pop esp
        ret

    noC:
        dec ebx
        mov byte[esi],bl
        jmp add_multi_end



randomNumber:
    init_randomNumber:
        push ebp
        mov ebp, esp
        mov ax, [STATE]
        mov bX, [MASK]

    xor bx, ax
    jp even

    STC 
    RCR ax,1
    jmp PRNG_end

    even:
        shr ax,1

    PRNG_end:   
        mov [STATE] ,ax
        mov  eax, [STATE]
        mov     esp, ebp
        pop     ebp
        ret