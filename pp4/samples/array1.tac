	# standard Decaf preamble 
	.data
TRUE:
	.asciiz "true"
FALSE:
	.asciiz "false"
	
	.text
	.align 2
	.globl main
	.globl _PrintInt
	.globl _PrintString
	.globl _PrintBool
	.globl _Alloc
	.globl _StringEqual
	.globl _Halt
	.globl _ReadInteger
	.globl _ReadLine
	
_PrintInt:
	subu $sp, $sp, 8	# decrement so to make space to save ra, fp
	sw $fp, 8($sp)  	# save fp
	sw $ra, 4($sp)  	# save ra
	addiu $fp, $sp, 8	# set up new fp
	li $v0, 1       	# system call code for print_int
	lw $a0, 4($fp)
	syscall
	move $sp, $fp
	lw $ra, -4($fp)
	lw $fp, 0($fp)
	jr $ra
	
_PrintBool:
	subu $sp, $sp, 8
	sw $fp, 8($sp)
	sw $ra, 4($sp)
	addiu $fp, $sp, 8
	lw $t1, 4($fp)
	blez $t1, fbr
	li $v0, 4       	# system call for print_str
	la $a0, TRUE
	syscall
	b end
fbr:
	li $v0, 4       	# system call for print_str
	la $a0, FALSE
	syscall
end:
	move $sp, $fp
	lw $ra, -4($fp)
	lw $fp, 0($fp)
	jr $ra
	
_PrintString:
	subu $sp, $sp, 8
	sw $fp, 8($sp)
	sw $ra, 4($sp)
	addiu $fp, $sp, 8
	li $v0, 4       	# system call for print_str
	lw $a0, 4($fp)
	syscall
	move $sp, $fp
	lw $ra, -4($fp)
	lw $fp, 0($fp)
	jr $ra
	
_Alloc:
	subu $sp, $sp, 8
	sw $fp, 8($sp)
	sw $ra, 4($sp)
	addiu $fp, $sp, 8
	li $v0, 9       	# system call for sbrk
	lw $a0, 4($fp)
	syscall
	move $sp, $fp
	lw $ra, -4($fp)
	lw $fp, 0($fp)
	jr $ra
	
_StringEqual:
	subu $sp, $sp, 8
	sw $fp, 8($sp)
	sw $ra, 4($sp)
	addiu $fp, $sp, 8
	subu $sp, $sp, 4	# decrement sp to make space for return value
	li $v0, 0
	#Determine length string 1
	lw $t0, 4($fp)
	li $t3, 0
bloop1:
	lb $t5, ($t0)
	beqz $t5, eloop1
	addi $t0, 1
	addi $t3, 1
	b bloop1
eloop1:
	#Determine length string 2
	lw $t1, 8($fp)
	li $t4, 0
bloop2:
	lb $t5, ($t1)
	beqz $t5, eloop2
	addi $t1, 1
	addi $t4, 1
	b bloop2
eloop2:
	bne $t3, $t4, end1	# check if string lengths are the same
	lw $t0, 4($fp)
	lw $t1, 8($fp)
	li $t3, 0
bloop3:
	lb $t5, ($t0)
	lb $t6, ($t1)
	bne $t5, $t6, end1
	addi $t3, 1
	addi $t0, 1
	addi $t1, 1
	bne $t3, $t4, bloop3
eloop3:
	li $v0, 1
end1:
	move $sp, $fp
	lw $ra, -4($fp)
	lw $fp, 0($fp)
	jr $ra
	
_Halt:
	li $v0, 10
	syscall
	
_ReadInteger:
	subu $sp, $sp, 8
	sw $fp, 8($sp)
	sw $ra, 4($sp)
	addiu $fp, $sp, 8
	subu $sp, $sp, 4
	li $v0, 5
	syscall
	move $sp, $fp
	lw $ra, -4($fp)
	lw $fp, 0($fp)
	jr $ra
	
_ReadLine:
	subu $sp, $sp, 8
	sw $fp, 8($sp)
	sw $ra, 4($sp)
	addiu $fp, $sp, 8
	li $t0, 40
	subu $sp, $sp, 4
	sw $t0, 4($sp)
	jal _Alloc
	move $t0, $v0
	li $a1, 40
	move $a0, $t0
	li $v0, 8
	syscall
	move $t1, $t0
bloop4:
	lb $t5, ($t1)
	beqz $t5, eloop4
	addi $t1, 1
	b bloop4
eloop4:
	addi $t1, -1
	li $t6, 0
	sb $t6, ($t1)
	move $v0, $t0
	move $sp, $fp
	lw $ra, -4($fp)
	lw $fp, 0($fp)
	jr $ra
	
main:
	# BeginFunc 476
	subu $sp, $sp, 8	# decrement sp to make space to save ra, fp
	sw $fp, 8($sp)	# save fp
	sw $ra, 4($sp)	# save ra
	addiu $fp, $sp, 8	# set up new fp
	subu $sp, $sp, 476	# decrement sp to make space for locals/temps
	# _tmp0 = 10
	li $t0, 10		# load constant value 10 into $t0
	# _tmp1 = 0
	li $t1, 0		# load constant value 0 into $t1
	# _tmp2 = _tmp0 < _tmp1
	slt $t2, $t0, $t1	
	# _tmp3 = _tmp0 == _tmp1
	seq $t3, $t0, $t1	
	# _tmp4 = _tmp2 || _tmp3
	or $t4, $t2, $t3	
	# IfZ _tmp4 Goto _L0
	# (save modified registers before flow of control change)
	sw $t0, -24($fp)	# spill _tmp0 from $t0 to $fp-24
	sw $t1, -28($fp)	# spill _tmp1 from $t1 to $fp-28
	sw $t2, -32($fp)	# spill _tmp2 from $t2 to $fp-32
	sw $t3, -36($fp)	# spill _tmp3 from $t3 to $fp-36
	sw $t4, -40($fp)	# spill _tmp4 from $t4 to $fp-40
	beqz $t4, _L0	# branch if _tmp4 is zero 
	# _tmp5 = "Decaf runtime error: Array size is <= 0"
	.data			# create string constant marked with label
	_string1: .asciiz "Decaf runtime error: Array size is <= 0"
	.text
	la $t0, _string1	# load label
	# PushParam _tmp5
	subu $sp, $sp, 4	# decrement sp to make space for param
	sw $t0, 4($sp)	# copy param value to stack
	# LCall _PrintString
	# (save modified registers before flow of control change)
	sw $t0, -44($fp)	# spill _tmp5 from $t0 to $fp-44
	jal _PrintString   	# jump to function
	# PopParams 4
	add $sp, $sp, 4	# pop params off stack
	# LCall _Halt
	jal _Halt          	# jump to function
_L0:
	# _tmp6 = 4
	li $t0, 4		# load constant value 4 into $t0
	# _tmp7 = _tmp0 * _tmp6
	lw $t1, -24($fp)	# load _tmp0 from $fp-24 into $t1
	mul $t2, $t1, $t0	
	# _tmp8 = _tmp7 + _tmp6
	add $t3, $t2, $t0	
	# PushParam _tmp8
	subu $sp, $sp, 4	# decrement sp to make space for param
	sw $t3, 4($sp)	# copy param value to stack
	# _tmp9 = LCall _Alloc
	# (save modified registers before flow of control change)
	sw $t0, -48($fp)	# spill _tmp6 from $t0 to $fp-48
	sw $t2, -52($fp)	# spill _tmp7 from $t2 to $fp-52
	sw $t3, -56($fp)	# spill _tmp8 from $t3 to $fp-56
	jal _Alloc         	# jump to function
	move $t0, $v0		# copy function return value from $v0
	# PopParams 4
	add $sp, $sp, 4	# pop params off stack
	# *(_tmp9) = _tmp0
	lw $t1, -24($fp)	# load _tmp0 from $fp-24 into $t1
	sw $t1, 0($t0) 	# store with offset
	# b = _tmp9
	move $t2, $t0		# copy value
	# _tmp10 = 20
	li $t3, 20		# load constant value 20 into $t3
	# _tmp11 = 0
	li $t4, 0		# load constant value 0 into $t4
	# _tmp12 = _tmp10 < _tmp11
	slt $t5, $t3, $t4	
	# _tmp13 = _tmp10 == _tmp11
	seq $t6, $t3, $t4	
	# _tmp14 = _tmp12 || _tmp13
	or $t7, $t5, $t6	
	# IfZ _tmp14 Goto _L1
	# (save modified registers before flow of control change)
	sw $t0, -60($fp)	# spill _tmp9 from $t0 to $fp-60
	sw $t2, -8($fp)	# spill b from $t2 to $fp-8
	sw $t3, -64($fp)	# spill _tmp10 from $t3 to $fp-64
	sw $t4, -68($fp)	# spill _tmp11 from $t4 to $fp-68
	sw $t5, -72($fp)	# spill _tmp12 from $t5 to $fp-72
	sw $t6, -76($fp)	# spill _tmp13 from $t6 to $fp-76
	sw $t7, -80($fp)	# spill _tmp14 from $t7 to $fp-80
	beqz $t7, _L1	# branch if _tmp14 is zero 
	# _tmp15 = "Decaf runtime error: Array size is <= 0"
	.data			# create string constant marked with label
	_string2: .asciiz "Decaf runtime error: Array size is <= 0"
	.text
	la $t0, _string2	# load label
	# PushParam _tmp15
	subu $sp, $sp, 4	# decrement sp to make space for param
	sw $t0, 4($sp)	# copy param value to stack
	# LCall _PrintString
	# (save modified registers before flow of control change)
	sw $t0, -84($fp)	# spill _tmp15 from $t0 to $fp-84
	jal _PrintString   	# jump to function
	# PopParams 4
	add $sp, $sp, 4	# pop params off stack
	# LCall _Halt
	jal _Halt          	# jump to function
_L1:
	# _tmp16 = 4
	li $t0, 4		# load constant value 4 into $t0
	# _tmp17 = _tmp10 * _tmp16
	lw $t1, -64($fp)	# load _tmp10 from $fp-64 into $t1
	mul $t2, $t1, $t0	
	# _tmp18 = _tmp17 + _tmp16
	add $t3, $t2, $t0	
	# PushParam _tmp18
	subu $sp, $sp, 4	# decrement sp to make space for param
	sw $t3, 4($sp)	# copy param value to stack
	# _tmp19 = LCall _Alloc
	# (save modified registers before flow of control change)
	sw $t0, -88($fp)	# spill _tmp16 from $t0 to $fp-88
	sw $t2, -92($fp)	# spill _tmp17 from $t2 to $fp-92
	sw $t3, -96($fp)	# spill _tmp18 from $t3 to $fp-96
	jal _Alloc         	# jump to function
	move $t0, $v0		# copy function return value from $v0
	# PopParams 4
	add $sp, $sp, 4	# pop params off stack
	# *(_tmp19) = _tmp10
	lw $t1, -64($fp)	# load _tmp10 from $fp-64 into $t1
	sw $t1, 0($t0) 	# store with offset
	# c = _tmp19
	move $t2, $t0		# copy value
	# _tmp20 = 3
	li $t3, 3		# load constant value 3 into $t3
	# _tmp21 = 0
	li $t4, 0		# load constant value 0 into $t4
	# _tmp22 = _tmp20 < _tmp21
	slt $t5, $t3, $t4	
	# _tmp23 = _tmp20 == _tmp21
	seq $t6, $t3, $t4	
	# _tmp24 = _tmp22 || _tmp23
	or $t7, $t5, $t6	
	# IfZ _tmp24 Goto _L2
	# (save modified registers before flow of control change)
	sw $t0, -100($fp)	# spill _tmp19 from $t0 to $fp-100
	sw $t2, -12($fp)	# spill c from $t2 to $fp-12
	sw $t3, -104($fp)	# spill _tmp20 from $t3 to $fp-104
	sw $t4, -108($fp)	# spill _tmp21 from $t4 to $fp-108
	sw $t5, -112($fp)	# spill _tmp22 from $t5 to $fp-112
	sw $t6, -116($fp)	# spill _tmp23 from $t6 to $fp-116
	sw $t7, -120($fp)	# spill _tmp24 from $t7 to $fp-120
	beqz $t7, _L2	# branch if _tmp24 is zero 
	# _tmp25 = "Decaf runtime error: Array size is <= 0"
	.data			# create string constant marked with label
	_string3: .asciiz "Decaf runtime error: Array size is <= 0"
	.text
	la $t0, _string3	# load label
	# PushParam _tmp25
	subu $sp, $sp, 4	# decrement sp to make space for param
	sw $t0, 4($sp)	# copy param value to stack
	# LCall _PrintString
	# (save modified registers before flow of control change)
	sw $t0, -124($fp)	# spill _tmp25 from $t0 to $fp-124
	jal _PrintString   	# jump to function
	# PopParams 4
	add $sp, $sp, 4	# pop params off stack
	# LCall _Halt
	jal _Halt          	# jump to function
_L2:
	# _tmp26 = 4
	li $t0, 4		# load constant value 4 into $t0
	# _tmp27 = _tmp20 * _tmp26
	lw $t1, -104($fp)	# load _tmp20 from $fp-104 into $t1
	mul $t2, $t1, $t0	
	# _tmp28 = _tmp27 + _tmp26
	add $t3, $t2, $t0	
	# PushParam _tmp28
	subu $sp, $sp, 4	# decrement sp to make space for param
	sw $t3, 4($sp)	# copy param value to stack
	# _tmp29 = LCall _Alloc
	# (save modified registers before flow of control change)
	sw $t0, -128($fp)	# spill _tmp26 from $t0 to $fp-128
	sw $t2, -132($fp)	# spill _tmp27 from $t2 to $fp-132
	sw $t3, -136($fp)	# spill _tmp28 from $t3 to $fp-136
	jal _Alloc         	# jump to function
	move $t0, $v0		# copy function return value from $v0
	# PopParams 4
	add $sp, $sp, 4	# pop params off stack
	# *(_tmp29) = _tmp20
	lw $t1, -104($fp)	# load _tmp20 from $fp-104 into $t1
	sw $t1, 0($t0) 	# store with offset
	# s = _tmp29
	move $t2, $t0		# copy value
	# _tmp30 = 5
	li $t3, 5		# load constant value 5 into $t3
	# _tmp31 = *(b)
	lw $t4, -8($fp)	# load b from $fp-8 into $t4
	lw $t5, 0($t4) 	# load with offset
	# _tmp32 = 3
	li $t6, 3		# load constant value 3 into $t6
	# _tmp33 = _tmp32 < _tmp31
	slt $t7, $t6, $t5	
	# _tmp34 = -1
	li $s0, -1		# load constant value -1 into $s0
	# _tmp35 = _tmp34 < _tmp32
	slt $s1, $s0, $t6	
	# _tmp36 = _tmp35 && _tmp33
	and $s2, $s1, $t7	
	# IfZ _tmp36 Goto _L3
	# (save modified registers before flow of control change)
	sw $t0, -140($fp)	# spill _tmp29 from $t0 to $fp-140
	sw $t2, -20($fp)	# spill s from $t2 to $fp-20
	sw $t3, -144($fp)	# spill _tmp30 from $t3 to $fp-144
	sw $t5, -148($fp)	# spill _tmp31 from $t5 to $fp-148
	sw $t6, -156($fp)	# spill _tmp32 from $t6 to $fp-156
	sw $t7, -152($fp)	# spill _tmp33 from $t7 to $fp-152
	sw $s0, -160($fp)	# spill _tmp34 from $s0 to $fp-160
	sw $s1, -164($fp)	# spill _tmp35 from $s1 to $fp-164
	sw $s2, -168($fp)	# spill _tmp36 from $s2 to $fp-168
	beqz $s2, _L3	# branch if _tmp36 is zero 
	# _tmp37 = 4
	li $t0, 4		# load constant value 4 into $t0
	# _tmp38 = _tmp32 * _tmp37
	lw $t1, -156($fp)	# load _tmp32 from $fp-156 into $t1
	mul $t2, $t1, $t0	
	# _tmp39 = _tmp38 + _tmp37
	add $t3, $t2, $t0	
	# _tmp40 = b + _tmp39
	lw $t4, -8($fp)	# load b from $fp-8 into $t4
	add $t5, $t4, $t3	
	# Goto _L4
	# (save modified registers before flow of control change)
	sw $t0, -172($fp)	# spill _tmp37 from $t0 to $fp-172
	sw $t2, -176($fp)	# spill _tmp38 from $t2 to $fp-176
	sw $t3, -180($fp)	# spill _tmp39 from $t3 to $fp-180
	sw $t5, -180($fp)	# spill _tmp40 from $t5 to $fp-180
	b _L4		# unconditional branch
_L3:
	# _tmp41 = "Decaf runtime error: Array script out of bounds"
	.data			# create string constant marked with label
	_string4: .asciiz "Decaf runtime error: Array script out of bounds"
	.text
	la $t0, _string4	# load label
	# PushParam _tmp41
	subu $sp, $sp, 4	# decrement sp to make space for param
	sw $t0, 4($sp)	# copy param value to stack
	# LCall _PrintString
	# (save modified registers before flow of control change)
	sw $t0, -184($fp)	# spill _tmp41 from $t0 to $fp-184
	jal _PrintString   	# jump to function
	# PopParams 4
	add $sp, $sp, 4	# pop params off stack
	# LCall _Halt
	jal _Halt          	# jump to function
_L4:
	# *(_tmp40) = _tmp30
	lw $t0, -144($fp)	# load _tmp30 from $fp-144 into $t0
	lw $t1, -180($fp)	# load _tmp40 from $fp-180 into $t1
	sw $t0, 0($t1) 	# store with offset
	# _tmp42 = 1
	li $t2, 1		# load constant value 1 into $t2
	# _tmp43 = *(c)
	lw $t3, -12($fp)	# load c from $fp-12 into $t3
	lw $t4, 0($t3) 	# load with offset
	# _tmp44 = 6
	li $t5, 6		# load constant value 6 into $t5
	# _tmp45 = _tmp44 < _tmp43
	slt $t6, $t5, $t4	
	# _tmp46 = -1
	li $t7, -1		# load constant value -1 into $t7
	# _tmp47 = _tmp46 < _tmp44
	slt $s0, $t7, $t5	
	# _tmp48 = _tmp47 && _tmp45
	and $s1, $s0, $t6	
	# IfZ _tmp48 Goto _L5
	# (save modified registers before flow of control change)
	sw $t2, -188($fp)	# spill _tmp42 from $t2 to $fp-188
	sw $t4, -192($fp)	# spill _tmp43 from $t4 to $fp-192
	sw $t5, -200($fp)	# spill _tmp44 from $t5 to $fp-200
	sw $t6, -196($fp)	# spill _tmp45 from $t6 to $fp-196
	sw $t7, -204($fp)	# spill _tmp46 from $t7 to $fp-204
	sw $s0, -208($fp)	# spill _tmp47 from $s0 to $fp-208
	sw $s1, -212($fp)	# spill _tmp48 from $s1 to $fp-212
	beqz $s1, _L5	# branch if _tmp48 is zero 
	# _tmp49 = 4
	li $t0, 4		# load constant value 4 into $t0
	# _tmp50 = _tmp44 * _tmp49
	lw $t1, -200($fp)	# load _tmp44 from $fp-200 into $t1
	mul $t2, $t1, $t0	
	# _tmp51 = _tmp50 + _tmp49
	add $t3, $t2, $t0	
	# _tmp52 = c + _tmp51
	lw $t4, -12($fp)	# load c from $fp-12 into $t4
	add $t5, $t4, $t3	
	# Goto _L6
	# (save modified registers before flow of control change)
	sw $t0, -216($fp)	# spill _tmp49 from $t0 to $fp-216
	sw $t2, -220($fp)	# spill _tmp50 from $t2 to $fp-220
	sw $t3, -224($fp)	# spill _tmp51 from $t3 to $fp-224
	sw $t5, -224($fp)	# spill _tmp52 from $t5 to $fp-224
	b _L6		# unconditional branch
_L5:
	# _tmp53 = "Decaf runtime error: Array script out of bounds"
	.data			# create string constant marked with label
	_string5: .asciiz "Decaf runtime error: Array script out of bounds"
	.text
	la $t0, _string5	# load label
	# PushParam _tmp53
	subu $sp, $sp, 4	# decrement sp to make space for param
	sw $t0, 4($sp)	# copy param value to stack
	# LCall _PrintString
	# (save modified registers before flow of control change)
	sw $t0, -228($fp)	# spill _tmp53 from $t0 to $fp-228
	jal _PrintString   	# jump to function
	# PopParams 4
	add $sp, $sp, 4	# pop params off stack
	# LCall _Halt
	jal _Halt          	# jump to function
_L6:
	# *(_tmp52) = _tmp42
	lw $t0, -188($fp)	# load _tmp42 from $fp-188 into $t0
	lw $t1, -224($fp)	# load _tmp52 from $fp-224 into $t1
	sw $t0, 0($t1) 	# store with offset
	# d = b
	lw $t2, -8($fp)	# load b from $fp-8 into $t2
	move $t3, $t2		# copy value
	# _tmp54 = "hello"
	.data			# create string constant marked with label
	_string6: .asciiz "hello"
	.text
	la $t4, _string6	# load label
	# _tmp55 = *(s)
	lw $t5, -20($fp)	# load s from $fp-20 into $t5
	lw $t6, 0($t5) 	# load with offset
	# _tmp56 = 2
	li $t7, 2		# load constant value 2 into $t7
	# _tmp57 = _tmp56 < _tmp55
	slt $s0, $t7, $t6	
	# _tmp58 = -1
	li $s1, -1		# load constant value -1 into $s1
	# _tmp59 = _tmp58 < _tmp56
	slt $s2, $s1, $t7	
	# _tmp60 = _tmp59 && _tmp57
	and $s3, $s2, $s0	
	# IfZ _tmp60 Goto _L7
	# (save modified registers before flow of control change)
	sw $t3, -16($fp)	# spill d from $t3 to $fp-16
	sw $t4, -232($fp)	# spill _tmp54 from $t4 to $fp-232
	sw $t6, -236($fp)	# spill _tmp55 from $t6 to $fp-236
	sw $t7, -244($fp)	# spill _tmp56 from $t7 to $fp-244
	sw $s0, -240($fp)	# spill _tmp57 from $s0 to $fp-240
	sw $s1, -248($fp)	# spill _tmp58 from $s1 to $fp-248
	sw $s2, -252($fp)	# spill _tmp59 from $s2 to $fp-252
	sw $s3, -256($fp)	# spill _tmp60 from $s3 to $fp-256
	beqz $s3, _L7	# branch if _tmp60 is zero 
	# _tmp61 = 4
	li $t0, 4		# load constant value 4 into $t0
	# _tmp62 = _tmp56 * _tmp61
	lw $t1, -244($fp)	# load _tmp56 from $fp-244 into $t1
	mul $t2, $t1, $t0	
	# _tmp63 = _tmp62 + _tmp61
	add $t3, $t2, $t0	
	# _tmp64 = s + _tmp63
	lw $t4, -20($fp)	# load s from $fp-20 into $t4
	add $t5, $t4, $t3	
	# Goto _L8
	# (save modified registers before flow of control change)
	sw $t0, -260($fp)	# spill _tmp61 from $t0 to $fp-260
	sw $t2, -264($fp)	# spill _tmp62 from $t2 to $fp-264
	sw $t3, -268($fp)	# spill _tmp63 from $t3 to $fp-268
	sw $t5, -268($fp)	# spill _tmp64 from $t5 to $fp-268
	b _L8		# unconditional branch
_L7:
	# _tmp65 = "Decaf runtime error: Array script out of bounds"
	.data			# create string constant marked with label
	_string7: .asciiz "Decaf runtime error: Array script out of bounds"
	.text
	la $t0, _string7	# load label
	# PushParam _tmp65
	subu $sp, $sp, 4	# decrement sp to make space for param
	sw $t0, 4($sp)	# copy param value to stack
	# LCall _PrintString
	# (save modified registers before flow of control change)
	sw $t0, -272($fp)	# spill _tmp65 from $t0 to $fp-272
	jal _PrintString   	# jump to function
	# PopParams 4
	add $sp, $sp, 4	# pop params off stack
	# LCall _Halt
	jal _Halt          	# jump to function
_L8:
	# *(_tmp64) = _tmp54
	lw $t0, -232($fp)	# load _tmp54 from $fp-232 into $t0
	lw $t1, -268($fp)	# load _tmp64 from $fp-268 into $t1
	sw $t0, 0($t1) 	# store with offset
	# _tmp66 = *(b)
	lw $t2, -8($fp)	# load b from $fp-8 into $t2
	lw $t3, 0($t2) 	# load with offset
	# _tmp67 = 3
	li $t4, 3		# load constant value 3 into $t4
	# _tmp68 = _tmp67 < _tmp66
	slt $t5, $t4, $t3	
	# _tmp69 = -1
	li $t6, -1		# load constant value -1 into $t6
	# _tmp70 = _tmp69 < _tmp67
	slt $t7, $t6, $t4	
	# _tmp71 = _tmp70 && _tmp68
	and $s0, $t7, $t5	
	# IfZ _tmp71 Goto _L9
	# (save modified registers before flow of control change)
	sw $t3, -276($fp)	# spill _tmp66 from $t3 to $fp-276
	sw $t4, -284($fp)	# spill _tmp67 from $t4 to $fp-284
	sw $t5, -280($fp)	# spill _tmp68 from $t5 to $fp-280
	sw $t6, -288($fp)	# spill _tmp69 from $t6 to $fp-288
	sw $t7, -292($fp)	# spill _tmp70 from $t7 to $fp-292
	sw $s0, -296($fp)	# spill _tmp71 from $s0 to $fp-296
	beqz $s0, _L9	# branch if _tmp71 is zero 
	# _tmp72 = 4
	li $t0, 4		# load constant value 4 into $t0
	# _tmp73 = _tmp67 * _tmp72
	lw $t1, -284($fp)	# load _tmp67 from $fp-284 into $t1
	mul $t2, $t1, $t0	
	# _tmp74 = _tmp73 + _tmp72
	add $t3, $t2, $t0	
	# _tmp75 = b + _tmp74
	lw $t4, -8($fp)	# load b from $fp-8 into $t4
	add $t5, $t4, $t3	
	# Goto _L10
	# (save modified registers before flow of control change)
	sw $t0, -300($fp)	# spill _tmp72 from $t0 to $fp-300
	sw $t2, -304($fp)	# spill _tmp73 from $t2 to $fp-304
	sw $t3, -308($fp)	# spill _tmp74 from $t3 to $fp-308
	sw $t5, -308($fp)	# spill _tmp75 from $t5 to $fp-308
	b _L10		# unconditional branch
_L9:
	# _tmp76 = "Decaf runtime error: Array script out of bounds"
	.data			# create string constant marked with label
	_string8: .asciiz "Decaf runtime error: Array script out of bounds"
	.text
	la $t0, _string8	# load label
	# PushParam _tmp76
	subu $sp, $sp, 4	# decrement sp to make space for param
	sw $t0, 4($sp)	# copy param value to stack
	# LCall _PrintString
	# (save modified registers before flow of control change)
	sw $t0, -312($fp)	# spill _tmp76 from $t0 to $fp-312
	jal _PrintString   	# jump to function
	# PopParams 4
	add $sp, $sp, 4	# pop params off stack
	# LCall _Halt
	jal _Halt          	# jump to function
_L10:
	# _tmp77 = *(_tmp75)
	lw $t0, -308($fp)	# load _tmp75 from $fp-308 into $t0
	lw $t1, 0($t0) 	# load with offset
	# PushParam _tmp77
	subu $sp, $sp, 4	# decrement sp to make space for param
	sw $t1, 4($sp)	# copy param value to stack
	# LCall _PrintInt
	# (save modified registers before flow of control change)
	sw $t1, -316($fp)	# spill _tmp77 from $t1 to $fp-316
	jal _PrintInt      	# jump to function
	# PopParams 4
	add $sp, $sp, 4	# pop params off stack
	# _tmp78 = *(b)
	lw $t0, -8($fp)	# load b from $fp-8 into $t0
	lw $t1, 0($t0) 	# load with offset
	# PushParam _tmp78
	subu $sp, $sp, 4	# decrement sp to make space for param
	sw $t1, 4($sp)	# copy param value to stack
	# LCall _PrintInt
	# (save modified registers before flow of control change)
	sw $t1, -320($fp)	# spill _tmp78 from $t1 to $fp-320
	jal _PrintInt      	# jump to function
	# PopParams 4
	add $sp, $sp, 4	# pop params off stack
	# _tmp79 = "\n"
	.data			# create string constant marked with label
	_string9: .asciiz "\n"
	.text
	la $t0, _string9	# load label
	# PushParam _tmp79
	subu $sp, $sp, 4	# decrement sp to make space for param
	sw $t0, 4($sp)	# copy param value to stack
	# LCall _PrintString
	# (save modified registers before flow of control change)
	sw $t0, -324($fp)	# spill _tmp79 from $t0 to $fp-324
	jal _PrintString   	# jump to function
	# PopParams 4
	add $sp, $sp, 4	# pop params off stack
	# _tmp80 = *(c)
	lw $t0, -12($fp)	# load c from $fp-12 into $t0
	lw $t1, 0($t0) 	# load with offset
	# _tmp81 = 6
	li $t2, 6		# load constant value 6 into $t2
	# _tmp82 = _tmp81 < _tmp80
	slt $t3, $t2, $t1	
	# _tmp83 = -1
	li $t4, -1		# load constant value -1 into $t4
	# _tmp84 = _tmp83 < _tmp81
	slt $t5, $t4, $t2	
	# _tmp85 = _tmp84 && _tmp82
	and $t6, $t5, $t3	
	# IfZ _tmp85 Goto _L11
	# (save modified registers before flow of control change)
	sw $t1, -328($fp)	# spill _tmp80 from $t1 to $fp-328
	sw $t2, -336($fp)	# spill _tmp81 from $t2 to $fp-336
	sw $t3, -332($fp)	# spill _tmp82 from $t3 to $fp-332
	sw $t4, -340($fp)	# spill _tmp83 from $t4 to $fp-340
	sw $t5, -344($fp)	# spill _tmp84 from $t5 to $fp-344
	sw $t6, -348($fp)	# spill _tmp85 from $t6 to $fp-348
	beqz $t6, _L11	# branch if _tmp85 is zero 
	# _tmp86 = 4
	li $t0, 4		# load constant value 4 into $t0
	# _tmp87 = _tmp81 * _tmp86
	lw $t1, -336($fp)	# load _tmp81 from $fp-336 into $t1
	mul $t2, $t1, $t0	
	# _tmp88 = _tmp87 + _tmp86
	add $t3, $t2, $t0	
	# _tmp89 = c + _tmp88
	lw $t4, -12($fp)	# load c from $fp-12 into $t4
	add $t5, $t4, $t3	
	# Goto _L12
	# (save modified registers before flow of control change)
	sw $t0, -352($fp)	# spill _tmp86 from $t0 to $fp-352
	sw $t2, -356($fp)	# spill _tmp87 from $t2 to $fp-356
	sw $t3, -360($fp)	# spill _tmp88 from $t3 to $fp-360
	sw $t5, -360($fp)	# spill _tmp89 from $t5 to $fp-360
	b _L12		# unconditional branch
_L11:
	# _tmp90 = "Decaf runtime error: Array script out of bounds"
	.data			# create string constant marked with label
	_string10: .asciiz "Decaf runtime error: Array script out of bounds"
	.text
	la $t0, _string10	# load label
	# PushParam _tmp90
	subu $sp, $sp, 4	# decrement sp to make space for param
	sw $t0, 4($sp)	# copy param value to stack
	# LCall _PrintString
	# (save modified registers before flow of control change)
	sw $t0, -364($fp)	# spill _tmp90 from $t0 to $fp-364
	jal _PrintString   	# jump to function
	# PopParams 4
	add $sp, $sp, 4	# pop params off stack
	# LCall _Halt
	jal _Halt          	# jump to function
_L12:
	# _tmp91 = *(_tmp89)
	lw $t0, -360($fp)	# load _tmp89 from $fp-360 into $t0
	lw $t1, 0($t0) 	# load with offset
	# PushParam _tmp91
	subu $sp, $sp, 4	# decrement sp to make space for param
	sw $t1, 4($sp)	# copy param value to stack
	# LCall _PrintBool
	# (save modified registers before flow of control change)
	sw $t1, -368($fp)	# spill _tmp91 from $t1 to $fp-368
	jal _PrintBool     	# jump to function
	# PopParams 4
	add $sp, $sp, 4	# pop params off stack
	# _tmp92 = *(c)
	lw $t0, -12($fp)	# load c from $fp-12 into $t0
	lw $t1, 0($t0) 	# load with offset
	# PushParam _tmp92
	subu $sp, $sp, 4	# decrement sp to make space for param
	sw $t1, 4($sp)	# copy param value to stack
	# LCall _PrintInt
	# (save modified registers before flow of control change)
	sw $t1, -372($fp)	# spill _tmp92 from $t1 to $fp-372
	jal _PrintInt      	# jump to function
	# PopParams 4
	add $sp, $sp, 4	# pop params off stack
	# _tmp93 = "\n"
	.data			# create string constant marked with label
	_string11: .asciiz "\n"
	.text
	la $t0, _string11	# load label
	# PushParam _tmp93
	subu $sp, $sp, 4	# decrement sp to make space for param
	sw $t0, 4($sp)	# copy param value to stack
	# LCall _PrintString
	# (save modified registers before flow of control change)
	sw $t0, -376($fp)	# spill _tmp93 from $t0 to $fp-376
	jal _PrintString   	# jump to function
	# PopParams 4
	add $sp, $sp, 4	# pop params off stack
	# _tmp94 = *(d)
	lw $t0, -16($fp)	# load d from $fp-16 into $t0
	lw $t1, 0($t0) 	# load with offset
	# _tmp95 = 3
	li $t2, 3		# load constant value 3 into $t2
	# _tmp96 = _tmp95 < _tmp94
	slt $t3, $t2, $t1	
	# _tmp97 = -1
	li $t4, -1		# load constant value -1 into $t4
	# _tmp98 = _tmp97 < _tmp95
	slt $t5, $t4, $t2	
	# _tmp99 = _tmp98 && _tmp96
	and $t6, $t5, $t3	
	# IfZ _tmp99 Goto _L13
	# (save modified registers before flow of control change)
	sw $t1, -380($fp)	# spill _tmp94 from $t1 to $fp-380
	sw $t2, -388($fp)	# spill _tmp95 from $t2 to $fp-388
	sw $t3, -384($fp)	# spill _tmp96 from $t3 to $fp-384
	sw $t4, -392($fp)	# spill _tmp97 from $t4 to $fp-392
	sw $t5, -396($fp)	# spill _tmp98 from $t5 to $fp-396
	sw $t6, -400($fp)	# spill _tmp99 from $t6 to $fp-400
	beqz $t6, _L13	# branch if _tmp99 is zero 
	# _tmp100 = 4
	li $t0, 4		# load constant value 4 into $t0
	# _tmp101 = _tmp95 * _tmp100
	lw $t1, -388($fp)	# load _tmp95 from $fp-388 into $t1
	mul $t2, $t1, $t0	
	# _tmp102 = _tmp101 + _tmp100
	add $t3, $t2, $t0	
	# _tmp103 = d + _tmp102
	lw $t4, -16($fp)	# load d from $fp-16 into $t4
	add $t5, $t4, $t3	
	# Goto _L14
	# (save modified registers before flow of control change)
	sw $t0, -404($fp)	# spill _tmp100 from $t0 to $fp-404
	sw $t2, -408($fp)	# spill _tmp101 from $t2 to $fp-408
	sw $t3, -412($fp)	# spill _tmp102 from $t3 to $fp-412
	sw $t5, -412($fp)	# spill _tmp103 from $t5 to $fp-412
	b _L14		# unconditional branch
_L13:
	# _tmp104 = "Decaf runtime error: Array script out of bounds"
	.data			# create string constant marked with label
	_string12: .asciiz "Decaf runtime error: Array script out of bounds"
	.text
	la $t0, _string12	# load label
	# PushParam _tmp104
	subu $sp, $sp, 4	# decrement sp to make space for param
	sw $t0, 4($sp)	# copy param value to stack
	# LCall _PrintString
	# (save modified registers before flow of control change)
	sw $t0, -416($fp)	# spill _tmp104 from $t0 to $fp-416
	jal _PrintString   	# jump to function
	# PopParams 4
	add $sp, $sp, 4	# pop params off stack
	# LCall _Halt
	jal _Halt          	# jump to function
_L14:
	# _tmp105 = *(_tmp103)
	lw $t0, -412($fp)	# load _tmp103 from $fp-412 into $t0
	lw $t1, 0($t0) 	# load with offset
	# PushParam _tmp105
	subu $sp, $sp, 4	# decrement sp to make space for param
	sw $t1, 4($sp)	# copy param value to stack
	# LCall _PrintInt
	# (save modified registers before flow of control change)
	sw $t1, -420($fp)	# spill _tmp105 from $t1 to $fp-420
	jal _PrintInt      	# jump to function
	# PopParams 4
	add $sp, $sp, 4	# pop params off stack
	# _tmp106 = *(d)
	lw $t0, -16($fp)	# load d from $fp-16 into $t0
	lw $t1, 0($t0) 	# load with offset
	# PushParam _tmp106
	subu $sp, $sp, 4	# decrement sp to make space for param
	sw $t1, 4($sp)	# copy param value to stack
	# LCall _PrintInt
	# (save modified registers before flow of control change)
	sw $t1, -424($fp)	# spill _tmp106 from $t1 to $fp-424
	jal _PrintInt      	# jump to function
	# PopParams 4
	add $sp, $sp, 4	# pop params off stack
	# _tmp107 = "\n"
	.data			# create string constant marked with label
	_string13: .asciiz "\n"
	.text
	la $t0, _string13	# load label
	# PushParam _tmp107
	subu $sp, $sp, 4	# decrement sp to make space for param
	sw $t0, 4($sp)	# copy param value to stack
	# LCall _PrintString
	# (save modified registers before flow of control change)
	sw $t0, -428($fp)	# spill _tmp107 from $t0 to $fp-428
	jal _PrintString   	# jump to function
	# PopParams 4
	add $sp, $sp, 4	# pop params off stack
	# _tmp108 = *(s)
	lw $t0, -20($fp)	# load s from $fp-20 into $t0
	lw $t1, 0($t0) 	# load with offset
	# _tmp109 = 2
	li $t2, 2		# load constant value 2 into $t2
	# _tmp110 = _tmp109 < _tmp108
	slt $t3, $t2, $t1	
	# _tmp111 = -1
	li $t4, -1		# load constant value -1 into $t4
	# _tmp112 = _tmp111 < _tmp109
	slt $t5, $t4, $t2	
	# _tmp113 = _tmp112 && _tmp110
	and $t6, $t5, $t3	
	# IfZ _tmp113 Goto _L15
	# (save modified registers before flow of control change)
	sw $t1, -432($fp)	# spill _tmp108 from $t1 to $fp-432
	sw $t2, -440($fp)	# spill _tmp109 from $t2 to $fp-440
	sw $t3, -436($fp)	# spill _tmp110 from $t3 to $fp-436
	sw $t4, -444($fp)	# spill _tmp111 from $t4 to $fp-444
	sw $t5, -448($fp)	# spill _tmp112 from $t5 to $fp-448
	sw $t6, -452($fp)	# spill _tmp113 from $t6 to $fp-452
	beqz $t6, _L15	# branch if _tmp113 is zero 
	# _tmp114 = 4
	li $t0, 4		# load constant value 4 into $t0
	# _tmp115 = _tmp109 * _tmp114
	lw $t1, -440($fp)	# load _tmp109 from $fp-440 into $t1
	mul $t2, $t1, $t0	
	# _tmp116 = _tmp115 + _tmp114
	add $t3, $t2, $t0	
	# _tmp117 = s + _tmp116
	lw $t4, -20($fp)	# load s from $fp-20 into $t4
	add $t5, $t4, $t3	
	# Goto _L16
	# (save modified registers before flow of control change)
	sw $t0, -456($fp)	# spill _tmp114 from $t0 to $fp-456
	sw $t2, -460($fp)	# spill _tmp115 from $t2 to $fp-460
	sw $t3, -464($fp)	# spill _tmp116 from $t3 to $fp-464
	sw $t5, -464($fp)	# spill _tmp117 from $t5 to $fp-464
	b _L16		# unconditional branch
_L15:
	# _tmp118 = "Decaf runtime error: Array script out of bounds"
	.data			# create string constant marked with label
	_string14: .asciiz "Decaf runtime error: Array script out of bounds"
	.text
	la $t0, _string14	# load label
	# PushParam _tmp118
	subu $sp, $sp, 4	# decrement sp to make space for param
	sw $t0, 4($sp)	# copy param value to stack
	# LCall _PrintString
	# (save modified registers before flow of control change)
	sw $t0, -468($fp)	# spill _tmp118 from $t0 to $fp-468
	jal _PrintString   	# jump to function
	# PopParams 4
	add $sp, $sp, 4	# pop params off stack
	# LCall _Halt
	jal _Halt          	# jump to function
_L16:
	# _tmp119 = *(_tmp117)
	lw $t0, -464($fp)	# load _tmp117 from $fp-464 into $t0
	lw $t1, 0($t0) 	# load with offset
	# PushParam _tmp119
	subu $sp, $sp, 4	# decrement sp to make space for param
	sw $t1, 4($sp)	# copy param value to stack
	# LCall _PrintString
	# (save modified registers before flow of control change)
	sw $t1, -472($fp)	# spill _tmp119 from $t1 to $fp-472
	jal _PrintString   	# jump to function
	# PopParams 4
	add $sp, $sp, 4	# pop params off stack
	# _tmp120 = *(s)
	lw $t0, -20($fp)	# load s from $fp-20 into $t0
	lw $t1, 0($t0) 	# load with offset
	# PushParam _tmp120
	subu $sp, $sp, 4	# decrement sp to make space for param
	sw $t1, 4($sp)	# copy param value to stack
	# LCall _PrintInt
	# (save modified registers before flow of control change)
	sw $t1, -476($fp)	# spill _tmp120 from $t1 to $fp-476
	jal _PrintInt      	# jump to function
	# PopParams 4
	add $sp, $sp, 4	# pop params off stack
	# _tmp121 = "\n"
	.data			# create string constant marked with label
	_string15: .asciiz "\n"
	.text
	la $t0, _string15	# load label
	# PushParam _tmp121
	subu $sp, $sp, 4	# decrement sp to make space for param
	sw $t0, 4($sp)	# copy param value to stack
	# LCall _PrintString
	# (save modified registers before flow of control change)
	sw $t0, -480($fp)	# spill _tmp121 from $t0 to $fp-480
	jal _PrintString   	# jump to function
	# PopParams 4
	add $sp, $sp, 4	# pop params off stack
	# EndFunc
	# (below handles reaching end of fn body with no explicit return)
	move $sp, $fp		# pop callee frame off stack
	lw $ra, -4($fp)	# restore saved ra
	lw $fp, 0($fp)	# restore saved fp
	jr $ra		# return from function
