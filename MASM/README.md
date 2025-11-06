# pwsd.asm — README

**Short description**

`pwsd.asm` is a small DOS/MASM program that implements a hidden-password prompt using BIOS keyboard input (INT 16h) and DOS string output (INT 21h AH=09). It runs in 16-bit real mode and is written for the **MASM** (Microsoft Macro Assembler) / compatible assemblers (TASM compatible MASM-syntax). The program demonstrates low-level keyboard reading (no echo), simple input buffering with backspace handling, fixed password comparison, and a 3-try lockout.

---

## Table of contents

1. Overview
3. Requirements / environment
4. Build & run (step-by-step)
5. Program behavior (what to expect)
6. Code walkthrough — section-by-section explanation

   * Data section
   * Helper: `print_str`
   * Input: `read_hidden`
   * Comparison: `cmp_pwd`
   * Main program flow: `MAIN`
7. Limitations, bugs & edge cases
8. Suggested improvements / exercises
9. Troubleshooting

---

## 1) Overview

The program prints a prompt `Enter password:` and reads the typed characters *without echoing them to the screen* by using BIOS keyboard service `INT 16h` (AH=00). The typed characters are stored into a fixed-length buffer. Backspace is handled by decreasing the recorded length (there is no visible backspace since input is not echoed). When the user presses Enter (ASCII 0Dh), the program compares the typed bytes against the hard-coded password `pwd` (8 bytes: `password`). If the password matches, `Access Granted.` is printed; if not, the user sees `Incorrect password. Try again.` and may retry up to 3 times. After three failed attempts the program prints `LOCKED.` and exits.

This program is meant as a lab/homework exercise to practice BIOS/DOS interrupts, MASM syntax, and low-level input handling.

---

## 2) Files

* `pwsd.asm` — the source assembly file (single-file program).
* `pwsd.obj`, `pwsd.exe` — generated after assemble & link steps.

(You may add this README as `README.md` to your project folder.)

---

## 3) Requirements / environment

This program is 16-bit DOS real-mode code and expects to be run under DOS or a DOS emulator (DOSBox) or on an environment that supports 16-bit real-mode DOS interrupts. Recommended setup options:

**A — DOSBox (recommended for modern machines)**

* Install DOSBox ([https://www.dosbox.com](https://www.dosbox.com)).
* Copy `pwsd.asm` into a folder and mount that folder inside DOSBox.
* Use a DOS assembler and linker inside DOSBox such as MASM/TASM and LINK/TLINK. Pre-built MASM/TASM binaries for DOS are commonly used in educational settings; copy them into your DOSBox environment.

**B — Real DOS / Virtual Machine**

* Run inside a virtual machine (DOS or FreeDOS) with MASM/TASM installed.

**C — Native Windows (older toolchains)**

* If you have an older Microsoft MASM toolchain (16-bit) you may assemble/link natively, but on modern 64-bit Windows this is usually not possible without an emulator.

> Note: This is 16-bit real-mode assembly — it cannot be run directly on modern 64-bit OSes without a DOS emulator or VM.

---

## 4) Build & run (step-by-step)

Below are example steps using **MASM + LINK** inside DOSBox. Equivalent steps with TASM / TLINK work similarly (replace `masm` with `tasm` and `link` with `tlink`).

1. Start DOSBox and mount the host directory. Example (from DOSBox prompt):

```
mount c c:\projects\pwsd
c:
```

2. Ensure MASM and LINK are present in the DOSBox environment (place `masm.exe`, `link.exe` or `ml.exe` and `link.exe` in the mounted `c:` or in `c:\dosutils` and add to PATH). Then assemble and link:

```
masm pwsd.asm;   ; produces pwsd.obj (or use ml /C /coff on modern toolchains if available)
link pwsd.obj;   ; produces pwsd.exe (or .com depending on options)
```

If using TASM/TLINK:

```
tasm pwsd.asm
tlink pwsd.obj
```

3. Run the program:

```
pwsd
```

**Expected interactive sequence**

* Program prints spacing and the prompt `Enter password:`.
* Type the password (characters are not echoed). Press Enter.
* If correct: displays `Access Granted.` and exits.
* If incorrect: displays `Incorrect password. Try again.` and prompts again until attempts exhausted.

---

## 5) Program behavior and sample transcript

```
(visual spacing printed)
        Enter password:  <-- user types: password <Enter>
        Access Granted.

(OR)
        Enter password:  <-- user types: wrongpass <Enter>
        Incorrect password. Try again.
        Enter password:  <-- second try...
        LOCKED.            <-- after 3 failed tries
```

Important: typed characters are not shown because BIOS INT 16h AH=00 was used (this function returns the pressed key in AL but does not echo to screen). The `inbuf` contains exactly the characters you typed (no CR/LF terminators) and `inlen` holds the length.

---

## 6) Code walkthrough — detailed explanation

Below is a detailed, line-by-line logical explanation of *each major block / function*.

### a) `DATA` segment

```
half_screen db 0Dh,0Dh,0Dh,0Dh,0Dh,0Dh,0Ah,0Ah,0Ah,0Ah,0Ah,0Ah,0Ah,0Ah,0Ah,0Ah,0Ah,0Ah,'$'
middle_adjustment db 0Dh,20h,20h,...,'$'
prompt      db 'Enter password: $'
ok_msg      db 'Access Granted.$'
wrong_msg   db 'Incorrect password. Try again.$'
locked_msg  db 'LOCKED.$'

pwd         db 'password'
pwd_len     equ 8

MAXLEN      equ 12
inbuf       db MAXLEN dup(0)
inlen       db 0
```

* `half_screen` and `middle_adjustment` are just sequences of CR (0Dh) and LF (0Ah) and spaces (20h) intended to add vertical spacing and indentation before certain messages. The trailing `$` is the DOS string terminator for INT 21h AH=09.
* All messages intended for INT 21h AH=09 must be terminated with `$`.
* `pwd` is the hardcoded ASCII password (`password`) and `pwd_len` is set to 8.
* `MAXLEN` is the maximum typed characters we accept (12). `inbuf` is a fixed-size buffer of `MAXLEN` bytes and `inlen` stores how many bytes the user actually typed.

**Notes:**

* `MAXLEN` must be >= `pwd_len` to let a correct password be entered.
* There is no guard against input longer than `MAXLEN` beyond ignoring extra keystrokes (the code checks and ignores if length >= MAXLEN).

---

### b) `print_str` PROC

```
print_str PROC
    push dx
    mov ah, 09h
    int 21h
    pop dx
    ret
print_str ENDP
```

* Purpose: prints a `$`-terminated string pointed to by `DX` using DOS interrupt `INT 21h` with `AH=09`.
* The routine `push dx` / `pop dx` preserves the DX register of the caller, because DOS `INT 21h` leaves DX unchanged for AH=09 but we still preserve the caller state as a good practice.
* Usage: callers set `DX` to `OFFSET some_string` then `call print_str`.

---

### c) `read_hidden` PROC (read keyboard without echo)

```
read_hidden PROC
    push ax
    push bx
    push cx
    push dx
    push si

    mov si, offset inbuf
    mov byte ptr inlen, 0

read_loop:
    mov ah, 00h
    int 16h
    cmp al, 0Dh
    je done_read
    cmp al, 08h
    je do_backspace
    cmp al, 20h
    jb ignore_key
    mov bl, [inlen]
    cmp bl, MAXLEN
    jae ignore_key
    mov [si+bx], al
    inc byte ptr inlen
    jmp read_loop

do_backspace:
    mov bl, [inlen]
    cmp bl, 0
    je read_loop
    dec byte ptr inlen
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
```

* Saves registers used (AX, BX, CX, DX, SI) at entry and restores them before return.
* `mov ah,00h / int 16h` calls BIOS keyboard service which waits for a keystroke and returns the ASCII code (if available) in `AL` and the scan code in `AH`. It **does not** echo the character.
* When `AL` is `0Dh` (carriage return / Enter), the input loop ends.
* If `AL` equals `08h` (Backspace), then the code decrements `inlen` (if >0). *No screen manipulation occurs* because we never echoed characters; the only effect is the internal buffer length decreases.
* If key code is < `20h` (ASCII < space) and not Backspace/Enter, it is ignored (this avoids control keys like Ctrl sequences).
* If printable (>= `20h`) and `inlen` < `MAXLEN`, the character is stored at `inbuf[inlen]` and `inlen` is incremented.
* When Enter is pressed, the procedure returns with `inlen` set and `inbuf` containing the typed characters.

**Edge cases / notes:**

* Extended keys (arrow keys, function keys) return `0` in AL and extended code in AH (or produce an initial 0). This code treats AL=0 as control and ignores it. If the user presses such keys, they are ignored. This is acceptable for simple password input.
* There is no terminal echo or masking (like `*`). If you want to show `*` per character you can call INT 21h AH=02 to write a character or write directly to screen; but this code intentionally hides input.

---

### d) `cmp_pwd` PROC (compare typed buffer with stored password)

```
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
```

* Compares `inlen` with `pwd_len`. If lengths differ, password is wrong.
* If lengths equal, compare each byte in `inbuf` vs `pwd` (case-sensitive). If any byte differs, set return value to 0 in AL. If equal, AL=1.
* The routine returns the result in `AL` (1 = equal, 0 = not equal). This avoids directly using flags and makes the caller clearer.

**Notes:**

* Comparison is case-sensitive because bytes are compared directly.
* The routine uses `loop` with `cx = pwd_len`.

---

### e) `MAIN` program flow

```
MAIN PROC
    mov ax,@data
    mov ds,ax

    mov cx, 3          ; up to 3 tries
    lea dx,half_screen
    call print_str
try_again:
    lea dx, half_screen
    call print_str
    lea dx, middle_adjustment
    call print_str
    lea dx, prompt
    call print_str

    call read_hidden

    call cmp_pwd
    cmp al, 1
    je access_granted

    lea dx, middle_adjustment
    call print_str
    lea dx, wrong_msg
    call print_str
    lea dx,half_screen
    call print_str

    dec cx
    cmp cx, 0
    jne try_again

    lea dx, middle_adjustment
    call print_str
    lea dx, locked_msg
    call print_str
    jmp exit

access_granted:
    lea dx,middle_adjustment
    call print_str
    lea dx, ok_msg
    call print_str
exit:
    mov ah, 4Ch
    int 21h

MAIN ENDP
END MAIN
```

* `mov ax,@data / mov ds,ax` initializes the `DS` data segment so that `OFFSET` addresses in the code reference the correct data.
* `CX = 3` sets a counter for the maximum allowed attempts.
* The program prints spacing then enters a `try_again` loop:

  1. Prints spacing and `Enter password:` prompt.
  2. Calls `read_hidden` to capture input.
  3. Calls `cmp_pwd` to compare typed input with stored password.
  4. If matched, jumps to `access_granted` and prints message.
  5. If not matched, prints `Incorrect password. Try again.` and decrements attempt counter `CX`. If `CX` is still >0 it repeats; otherwise prints `LOCKED.` and exits.
* `mov ah,4Ch / int 21h` exits back to DOS with return code in `AL` (unused here).

---

## 7) Limitations, bugs & edge cases

* **Hardcoded password**: `pwd` is stored in plaintext inside the binary; this is insecure but acceptable for a lab exercise.
* **No masking / no feedback**: Characters typed are invisible — this is by design. If you prefer to show `*` for each typed character, you must write a small routine to print `*` (with INT 21h AH=02) whenever a printable character is accepted.
* **No delete/line editing**: Backspace only decrements `inlen` — it does not move a cursor on-screen because no characters are echoed. Multi-key editing is not supported.
* **No CR/LF storage**: `Enter` ends the read and is not stored into `inbuf`.
* **Extended keys**: Some special keys may push extra codes; the code ignores anything < 20h (except backspace and enter).
* **Fixed buffer**: `MAXLEN` is a compile-time constant. If you increase `pwd_len`, remember to increase `MAXLEN` as well.
* **No stack/interrupt safety for other ISRs**: The code saves and restores only the registers it uses inside procedures; it assumes DOS/BIOS interrupts behave normally.

---

## 8) Suggested improvements / exercises (for learning)

* **Echo masked characters**: Show `*` as the user types (call INT 21h AH=02 to write the character). This requires printing and handling backspace visibly (move cursor back and erase `*`).
* **Salted/hashed password**: Implement a simple hash (XOR-based) in assembly to avoid storing the cleartext password. (For learning only — this would still be insecure.)
* **Case-insensitive compare**: Convert both typed chars and `pwd` to uppercase/lowercase before compare.
* **Configurable password**: Allow the program to read a password from a configuration area in disk or from command-line (more advanced — requires DOS file I/O / param parsing).
* **Protect against buffer overflow**: The current code already checks `inlen` vs `MAXLEN` and ignores extra characters, but you could provide an indicator when buffer is full.
* **Show remaining tries**: Print how many tries are left after each incorrect attempt.
* **Add timing / timeout**: Abort input if user does not press keys for N seconds (requires BIOS time services or polling the system tick).

---

