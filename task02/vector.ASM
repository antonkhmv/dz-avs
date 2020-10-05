; Variant 6
; Разработать программу, которая вводит одномерный массив A[N], формирует из элементов массива A новый 
; массив B по правилам, указанным в таблице, и выводит его. Память под массивы может выделяться как 
; статически, так и динамически по выбору разработчика.

; Разбить решение задачи на функции следующим образом:

; Ввод и вывод массивов оформить как подпрограммы.
; Выполнение задания по варианту оформить как процедуру
; Организовать вывод как исходного, так и сформированного массивов
; Указанные процедуры могут использовать данные напрямую (имитация процедур без параметров). Имитация 
; работы с параметрами также допустима.

; Массив B из элементов A, значение которых кратно введённому числу x
;------------------------------------------------------------------------------------------------------

format PE console
entry start

include 'win32a.inc'

section '.data' data readable writable

        strArrSize   db 'Enter array size: ', 0
        strArrAMesg  db 'Array A:', 10, 13, 0
        strArrBMesg  db 'Array B:', 10, 13, 0
        strEnterX    db 'Enter X: ', 0
        strArrElemI  db 'Value at index [%d]: ', 0
        strScanInt   db '%d', 0
        strArrElemOut  db '[%d] = %d', 10, 0

        arr_A_size   dd 0
        arr_B_size   dd 1
        i            dd ?
        j            dd ?
        tmp          dd ?
        tmp2         dd 0
        tmpStack     dd ?
        x            dd 10
        r            dd ?
        r1           dd ?

        MAX_SIZE = 1000

        arr_A          rd MAX_SIZE
        arr_B          rd MAX_SIZE
        arr_addr       dd 0
        arr_size       dd 0

        resStr db '%d',10,13,0

;--------------------------------------------------------------------------
section '.code' code readable executable

start:
        ; Input the array A
        call InputArray

        ; Set arr_addr to arr_A and print the array.
        call PrintMesgA
        call PrintArr

        ; Read x
        push strEnterX
        call [printf]
        push x
        push strScanInt
        call [scanf]

        ; Build array B
        call BuildB

        ; Set arr_addr to arr_B and print the array.
        call PrintMesgB
        call PrintArr

        ; Finish
        call [getch]
        push 0
        call [ExitProcess]
;--------------------------------------------------------------------------
BuildB:
        mov [tmpStack], esp
        ; Set a pointer to the beginning of arr_A
        mov ebx, arr_A
        ; Set a pointer to the beginning of arr_B
        mov esi, arr_B

        xor ecx, ecx    ; ecx = 0
        xor edx, edx    ; edx = 0
                
buildBLoop:
        mov [tmp], ebx
        mov [tmp2], esi

        ; while ecx < arr_A_size
        cmp ecx, [arr_A_size]
        je endBuildB

        mov [i], ecx
        mov [j], edx

        ; eax = A[i]
        mov eax, dword[ebx]
        mov [r1], eax

        ; eax = abs(eax)
        cmp eax, 0
        jg __end
        mov eax, 0
        sub eax, [r1]
__end:
        mov edx, 0

        ; edx = eax % x
        div [x]

        mov [r], edx

        ; B[j] = A[i]
        mov eax, [r1]
        mov dword[esi], eax

        mov esi, [tmp2]
        mov ebx, [tmp]

        add ebx, 4

        mov ecx, [i]
        mov edx, [j]


        inc ecx
        ; A[i] % x == 0
        cmp [r], 0
        je cond
        jmp _end
cond:
        ; increase j only if A[i] % x == 0
        add esi, 4
        add edx, 1
_end:
        jmp buildBLoop
endBuildB:
        mov esp, [tmpStack]
        mov [arr_B_size], edx
        ret
;--------------------------------------------------------------------------
InputArray:
        push strArrSize
        call [printf]
        add esp, 4

        push arr_A_size
        push strScanInt
        call [scanf]
        add esp, 8

        mov eax, [arr_A_size]
        cmp eax, 0
        jle _exit
        cmp eax, MAX_SIZE
        jge _exit
        jmp getArray
_exit:
        push 0
        call [ExitProcess]

getArray:
        xor ecx, ecx            ; ecx = 0
        mov ebx, arr_A            ; ebx = &arr_A

getArrLoop:
        mov [tmp], ebx
        cmp ecx, [arr_A_size]
        jge endInputArray       ; to end of loop

        ; input element
        mov [i], ecx
        push ecx
        push strArrElemI
        call [printf]
        add esp, 8

        push ebx
        push strScanInt
        call [scanf]
        add esp, 8

        mov ecx, [i]
        inc ecx
        mov ebx, [tmp]
        add ebx, 4
        jmp getArrLoop

endInputArray:
        ret

;--------------------------------------------------------------------------
PrintMesgA:
        push strArrAMesg
        call [printf]
        add esp, 4

        mov ebx, arr_A
        mov [arr_addr], ebx
        mov edx, [arr_A_size]
        mov [arr_size], edx
        ret
;--------------------------------------------------------------------------
PrintMesgB:
        push strArrBMesg
        call [printf]
        add esp, 4

        mov ebx, arr_B
        mov [arr_addr], ebx
        mov ebx, [arr_B_size]
        mov [arr_size], ebx
        ret
;--------------------------------------------------------------------------
PrintArr:
        mov [tmpStack], esp
        mov ebx, [arr_addr]          ; ebx = &arr_A
        xor ecx, ecx            ; ecx = 0

putArrLoop:
        mov [tmp], ebx
        cmp ecx, [arr_size]
        jge endOutputArray
        mov [i], ecx

        ; output element
        push dword [ebx]
        push ecx
        push strArrElemOut
        call [printf]

        mov ecx, [i]
        inc ecx
        mov ebx, [tmp]
        add ebx, 4
        jmp putArrLoop
endOutputArray:
        mov esp, [tmpStack]
        ret
;--------------------------------------------------------------------------
                                                 
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