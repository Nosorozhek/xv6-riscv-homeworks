.data

OUTPUT_FMT: .asciz "%d\n"
#define MSG_FMT_LEN 2
MSG_PARSE_ERROR: .asciz "addrv: Incorrect input format.\nThere must be two numbers on a single line, separated by a single whitespace, with no other characters such as trailing whitespaces.\n"
#define MSG_PARSE_ERROR_LEN 164
MSG_READ_ERROR: .asciz "addrv: failed to read input\n"
#define MSG_READ_ERROR_LEN 28
MSG_WRITE_ERROR: .asciz "addrv: failed to write the result\n"
#define MSG_WRITE_ERROR_LEN 34

buf:
    .skip 512                           # Buffer (filled with zeros)

.text                                   # Text (code) section
.global main                            # Exportable object (label)

main:
    la      a0, buf                 # a0 = buf
    li      a1, 512                 # a1 = 512
    call    gets
    # check for return code
    beq     a0, zero, read_error    # if a0 == NULL then read_error
    mv      t0, a0                  # t0 = a0 (t0 stores second_ptr, a0 stores first_ptr)

    # parse the sign of the first number
    lb      t1, 0(t0)               # t1 = *second_ptr 
    li      t2, '-'                 # t2 = '-'
    li      t5, 1                   # t5 = 1 (t5 stores sign of the first number)
    bne     t1, t2, parse1          # if t1 != '-' then parse1
    li      t5, -1                  # t5 = -1 (t5 stores sign of the first number)
    addi    a0, a0, 1               # a0 = a0 + 1 (++first_ptr)
    addi    t0, t0, 1               # t0 = t0 + 1 (++second_ptr)

    # parse the first number
parse1:
    call    atoi                    # a0 = abs(first)
    mul     t5, a0, t5              # t5 = abs(first) * sign(first)

    # skip the first number
    li      t2, '0'                 # t2 = '0'
    li      t3, '9'                 # t3 = '9'
    li      t4, ' '                 # t4 = ' '
    lb      t1, 0(t0)               # t1 = *second_ptr 
loop1:
    blt     t1, t2, parse_error     # if t1 < '0' then parse_error
    bgt     t1, t3, parse_error     # if t1 > '9' then parse_error
    addi    t0, t0, 1               # t0 = t0 + 1 (++second_ptr)
    lb      t1, 0(t0)               # t1 = *second_ptr 
    beq     t1, t4, end1            # if t1 == ' ' then end1
    j       loop1
end1:
    
    # skip whitespace
    lb      t1, 0(t0)               # t1 = *second_ptr 
    li      t2, ' '                 # t2 = ' '
    bne     t1, t2, parse_error     # if t1 != t2 then parse_error
    addi    t0, t0, 1;              # t0 = t0 + 1 (++second_ptr)

    # parse the sign of the second number
    lb      t1, 0(t0)               # t1 = *second_ptr 
    li      t2, '-'                 # t2 = '-'
    li      t6, 1                   # t6 = 1 (t6 stores sign of the second number)
    bne     t1, t2, parse2          # if t1 != '-' then parse2
    li      t6, -1                  # t6 = -1 (t6 stores sign of the second number)
    addi    t0, t0, 1               # t0 = t0 + 1 (++second_ptr)

    # parse the second number
parse2:
    mv      a0, t0                  # a0 = t0
    call    atoi                    # a0 = abs(second)
    mul     t6, a0, t6              # t6 = abs(second) * sign(second)
                                
                                    # t5 = first
                                    # t6 = second
    
    # skip the second number
    li      t2, '0'                 # t2 = '0'
    li      t3, '9'                 # t3 = '9'
    li      t4, '\0'                # t4 = '\0'
    li      a0, '\r'                # a0 = '\r'
    li      a1, '\n'                # a1 = '\n'
    lb      t1, 0(t0)               # t1 = *ptr 
loop2:
    blt     t1, t2, parse_error     # if t1 < '0' then parse_error
    bgt     t1, t3, parse_error     # if t1 > '9' then parse_error
    addi    t0, t0, 1               # t0 = t0 + 1 (++ptr)
    lb      t1, 0(t0)               # t1 = *ptr 
    beq     t1, t4, end2            # if t1 == '\0' then end2
    beq     t1, a0, end2            # if t1 == '\r' then end2
    beq     t1, a1, end2            # if t1 == '\n' then end2
    j       loop2
end2:

    # compute answer
    add     a1, t5, t6;             # a1 = t5 + t6 = first + second
    la      a0, OUTPUT_FMT          # a0 = "%d\n"
    call    printf                  # call printf (external symbol)

    # check for return code
    blt     a0, zero, write_error   # if a0 < 0 then write_error

exit:     
    li a7, 2                        # exit system call
    li a0, 0                        # exit status (success)
    ecall

parse_error:
    li a7, 16                       # Write syscall number
    li a0, 2                        # stderr fd (2)
    la a1, MSG_PARSE_ERROR          # Error message string address 
    li a2, MSG_PARSE_ERROR_LEN      # Error message string length
    ecall                           # Syscall

    li a7, 2                        # Exit system call
    li a0, 1                        # Exit status (parse_error)
    ecall                           # Syscall

read_error:
    li a7, 16                       # Write syscall number
    li a0, 2                        # stderr fd (2)
    la a1, MSG_READ_ERROR          # Error message string address 
    li a2, MSG_READ_ERROR_LEN      # Error message string length
    ecall                           # Syscall

    li a7, 2                        # Exit system call
    li a0, 1                        # Exit status (read_error)
    ecall    

write_error:
    li a7, 16                       # Write syscall number
    li a0, 2                        # stderr fd (2)
    la a1, MSG_WRITE_ERROR          # Error message string address 
    li a2, MSG_WRITE_ERROR_LEN      # Error message string length
    ecall                           # Syscall

    li a7, 2                        # Exit system call
    li a0, 1                        # Exit status (write_error)
    ecall                           # Syscall
