.extern a,b,c,d,e,f
.global x,y,z
.section s1
x:
ld $a, %r1
ld $b, %r2
.section s2
y:
ld $c, %r4
.section s3
z:
ld $d, %r6
ld $e, %r7
ld $f, %r8
.section s4
ld $2, %r10
ld $2, %r11
ld $2, %r12
