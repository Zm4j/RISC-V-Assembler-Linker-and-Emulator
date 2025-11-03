.section textt
ld msg_begin, %r1
.equ a, msg_end - msg_begin
.equ b, hndl+1
call b
.equ cc, hndl2+5
call cc

.section data
msg_begin:
.ascii "Hello"
msg_end:

hndl:
.skip 1
ld $2, %r2
ret
hndl2:
.skip 5
ld $3, %r3
ret