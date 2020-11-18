format PE console
entry Main

include 'win32a.inc'

section '.data' data readable writable

        MaxI = 65536

        ; Main data
        N               dd ?
        strScanInt      db '%u', 0
        strInput        db 'Enter N: ',0
        strOutput       db 'The amount of Mersenne numbers up to N is: %u', 10, 13, 0

        ; CountMersenneNumber data
        ans             dd ?
        j               dd ?
        n               dd ?
        tmp2            dd ?

        ; CheckPrime data
        i               dd ?
        num             dd ?
        tmp             dd ?


;--------------------------------------------------------------------------
section '.code' code readable executable

;------------------------- main function 'Main' --------------------------------------------
Main:
        ; Print a message asking the user to input the number
        invoke printf, strInput

        ; Input N
        invoke scanf, strScanInt, N

        ; Count the amount of Mersenne numbers
        push [N]
        call CountMersenneNumbers

        ; Output the answer
        invoke printf, strOutput, edx

        call [getch]
        invoke ExitProcess, 0

;----------------------------- function 'CountMersenneNumbers' ------------------------------
; Parameters: num, dd
; Returns: (to edx) dd, the amount of Mersenne primes from 1 to num.
CountMersenneNumbers:
        mov [tmp2], esp
        ; int CountMersenneNumbers(int num) {
        ;     int ans = 0;
        ;     for (int j = 1; ...
        mov ebx, dword[esp+4]
        mov [n], ebx
        mov [j], 1
        mov [ans], 0

CountMersenneNumbers_Loop:
        mov ecx, [j]
        ; ... j <= n;
        cmp ecx, [n]
        ja CountMersenneNumbers_Return

        ; ans += CheckPrime(j);
        push [j]
        call CheckPrime
        add [ans], edx

        ; ... j = j*2 + 1) {
        mov ecx, [j]
        add [j], ecx
        add [j], 1

        ; if [j] = 2**32-1 => [j]*2 + 1 = 2**32-1 (integer overflow)
        cmp ecx, [j]
        je CountMersenneNumbers_Return

        jmp CountMersenneNumbers_Loop

CountMersenneNumbers_Return:
        mov esp, [tmp2]
        mov edx, [ans]
        ret

;----------------------------- function 'CheckPrime' ----------------------------------------
; Parameters: num, dd
; Returns: (to edx) bool, ture if num is prime and false otherwise.
CheckPrime:
        ; bool CheckPrime(int num) {
        ;     for (int i = 2; ...
        mov [tmp], esp
        mov ebx, dword[esp+4]
        mov [num], ebx

        ; if (num == 1) return false;
        cmp ebx, 1
        je CheckPrime_ReturnFalse

        mov [i], 2

CheckPrime_Loop:
        ; eax = i
        mov eax, [i]

        ; protection from overflow
        cmp eax, MaxI
        je CheckPrime_ReturnTrue

        ; for (... i * i <= num; ...) {
        ; ...
        ; }
        ; return true;
        mul [i]
        cmp eax, [num]
        ja CheckPrime_ReturnTrue

        ; if (num % d == 0)
        mov eax, [num]
        div [i]
        cmp edx, 0
        ; return false;
        je CheckPrime_ReturnFalse

        ;... ++i) {
        add [i], 1
        jmp CheckPrime_Loop

CheckPrime_ReturnTrue:
        mov esp, [tmp]
        mov edx, 1
        ret

CheckPrime_ReturnFalse:
        mov esp, [tmp]
        mov edx, 0
        ret
;--------------------------------------------------------------------------------------

section '.idata' import data readable
    library kernel, 'kernel32.dll',\
            msvcrt, 'msvcrt.dll',\
            user32,'USER32.DLL'

include 'api\user32.inc'
include 'api\kernel32.inc'
    import kernel,\
           ExitProcess, 'ExitProcess',\
           HeapCreate,'HeapCreate',\
           HeapAlloc,'HeapAlloc'

include 'api\kernel32.inc'
    import msvcrt,\
           printf, 'printf',\
           scanf, 'scanf',\
           getch, '_getch'