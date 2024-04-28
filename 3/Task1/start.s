section .data
    newline db 0xA
    bufferSize equ 2048
    buffer times bufferSize db 0
    infile dd 0
    outfile dd 1

section .text
    global _start
    global system_call
    extern strlen
    global main
    global encode
_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv 
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call main   

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop
        
system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...        
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller


encode:
    pusha

    mov eax, 3
    mov ebx, [infile]
    mov ecx, buffer
    mov edx, bufferSize
    int 0x80

    cmp eax, 0 ;check if its EOF
    je end_encode

    mov edi, eax
    mov ebx, buffer
    mov ecx, edi
    

    loop:
        cmp ecx, 0
        je print_buffer

        cmp byte [ebx], 'A'
        jl put

        cmp byte [ebx], 'z'
        jg put

        inc byte [ebx]

        put:
            inc ebx
            dec ecx
            jmp loop
        
        print_buffer:
            mov eax, 4
            mov ebx, [outfile]
            mov ecx, buffer
            mov edx, edi
            int 0x80
            popa
            jmp encode
            
        
    end_encode:
        popa
        ret
            



main:
    ; Loop through the arguments
    cmp ecx, 1
    je end_prog

    push ebp
    mov ebp, esp
    pusha
    mov ecx, [ebp+8]
    mov esi, [ebp + 12] 
    mov edi, 1

    mov byte [infile], 0 ;stdin
    mov byte [outfile], 1 ;stdout
    
    next_arg:
        ;check if we are done with the pointers
        cmp edi, ecx
        jge end_prog

        ;not done, check if the next pointer points to null
        lea ebx, [esi + edi * 4]
        mov ebx, [ebx]
        cmp ebx, 0
        je end_prog

        ;not points to null, parse the next arg
        push ebx
        call strlen
        add esp, 4
        push ebx 

        ;write the next arg to stdout
        mov edx, eax              
        mov eax, 4
        mov ecx, ebx                
        mov ebx, 1              
        int 0x80

        ;write newline to stdout 
        mov eax, 4
        mov ebx, 1
        mov ecx, newline 
        mov edx, 1
        int 0x80

        pop ebx
        
        inc edi
        
        ;search for -o or -i: check if the 1st char of the string is '-'
        cmp byte [ebx], '-'
        jne next_arg
        ;1st char is '-', check if the second is 'i' or 'o'
        inc ebx
        cmp byte [ebx], 'o'
        je open_outfile
        cmp byte [ebx], 'i'
        je open_infile

        ;move to the next argument if its not 'i' or 'o'
        jmp next_arg

    open_outfile:
        mov eax, 5
        inc ebx
        mov ecx, 0x42 ;stads for O_CREAT | O_TRUNC | O_WRONLY
        mov edx, 0666o
        int 0x80
        mov dword [outfile], eax
        jmp next_arg
        
    open_infile:
        push ecx ;save ecx curr state
        mov eax, 5
        inc ebx
        mov ecx, 0
        int 0x80
        mov dword [infile], eax
        pop ecx ;return ecx to prev state 
        jmp next_arg




    end_prog:
        call encode 
        ;close outfile if its not stdout
        cmp dword [outfile], 1
        je close_infile
        ;not stdout, close it
        mov eax, 6
        mov ebx, [outfile]
        int 0x80

        close_infile: ;if its not stdin
            cmp dword [infile], 0
            je exit
            ;not stdin, close it
            mov eax, 6
            mov ebx, [infile]
            int 0x80


        ; Exit the program
        exit:
            popa
            mov esp, ebp
            pop ebp
            mov eax, 1                ; system call for exit
            xor ebx, ebx              ; return value (success)
            int 0x80                  ; invoke system cal
