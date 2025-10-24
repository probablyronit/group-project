; pw_check.asm -- BIOS hidden-password verifier
; Assembles with MASM (small model). Uses INT 16h for keyboard handling (no echo).
.MODEL SMALL
.STACK 100h

.DATA
prompt      db 0Dh,0Ah,'Enter password: $'
ok_msg      db 0Dh,0Ah,'Access Granted.$'
wrong_msg   db 0Dh,0Ah,'Incorrect password. Try again.$'
locked_msg  db 0Dh,0Ah,'LOCKED.$'

; Hardcoded password (ASCII)
pwd         db 'password'    ; 8 bytes, case-sensitive
pwd_len     equ 8

; input buffer
MAXLEN      equ 12
inbuf       db MAXLEN dup(0) ; storage for typed chars (no terminator)
inlen       db 0              ; actual length entered

.CODE
; ---------- helper: print $-terminated string (DOS INT 21 AH=09) ----------
print_str PROC
    push dx
    mov ah, 09h
    int 21h
    pop dx
    ret
print_str ENDP

; ---------- read hidden input via BIOS INT 16h AH=00 (no echo) ----------
; Returns: inlen = length (byte), buffer filled at inbuf[0..len-1]
read_hidden PROC
    push ax
    push bx
    push cx
    push dx
    push si

    mov si, offset inbuf
    mov byte ptr inlen, 0     ; clear length

read_loop:
    mov ah, 00h
    int 16h                   ; BIOS keyboard read (no echo)
    cmp al, 0Dh               ; Enter?
    je done_read
    cmp al, 08h               ; Backspace?
    je do_backspace
    ; If printable and not exceed max
    cmp al, 20h
    jb ignore_key
    mov bl, [inlen]
    cmp bl, MAXLEN
    jae ignore_key            ; if length >= MAXLEN ignore extra chars
    ; store char
    mov [si+bx], al
    inc byte ptr inlen
    jmp read_loop

do_backspace:
    mov bl, [inlen]
    cmp bl, 0
    je read_loop
    dec bl
    mov [inlen], bl
    ; optional: physically don't erase on screen because we never echoed
    jmp read_loop

ignore_key:
    jmp read_loop

done_read:
    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    ret
read_hidden ENDP

; ---------- compare buffer with password ----------
; Returns ZF=1 if equal, ZF=0 if not (we'll set AL=1/0 for clarity)
cmp_pwd PROC
    push si
    push di
    push cx
    mov al, [inlen]
    cmp al, pwd_len
    jne .not_equal
    mov cx, pwd_len
    mov si, offset inbuf
    mov di, offset pwd
cmp_loop:
    mov al, [si]
    mov bl, [di]
    cmp al, bl
    jne .not_equal
    inc si
    inc di
    loop cmp_loop
    ; equal
    mov al, 1
    jmp .done
.not_equal:
    mov al, 0
.done:
    pop cx
    pop di
    pop si
    ret
cmp_pwd ENDP

; ---------- main ----------
MAIN PROC
    mov ax,@data
    mov ds,ax

    mov cx, 3          ; up to 3 tries

try_again:
    ; prompt
    lea dx, prompt
    call print_str

    ; read hidden input
    call read_hidden

    ; compare
    call cmp_pwd
    cmp al, 1
    je access_granted

    ; incorrect
    lea dx, wrong_msg
    call print_str

    dec cx
    cmp cx, 0
    jne try_again

    ; locked
    lea dx, locked_msg
    call print_str
    jmp exit              ; halt (infinite loop)

access_granted:
    lea dx, ok_msg
    call print_str
exit:
    mov ah, 4Ch
    int 21h

MAIN ENDP
END MAIN
