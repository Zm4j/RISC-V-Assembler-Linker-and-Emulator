.global a,b,c,d,e,f
.extern x,y,z
.section s1
a:
ld $1, %r1
ld $1, %r2
ld $1, %r3
.section s2
b:
ld $1, %r4
ld $1, %r5
.section s3
ld $1, %r6
c:
ld $1, %r7
ld $1, %r8
ld $1, %r9
.equ d, a+b+c+0xff00
.equ e, f+d
.equ f, 0x1111
.equ g, x+y+z+f
