	.text
	.align	2
	.global	AutoHookWriteSram
	.code	16
	.thumb_func

/* 
 * AutoHookWriteSram(src=r0, addr=r1, len=r2)
 *
 * The first 16 bytes intentionally match the patcher's write_sram_signature.
 * This function is NOT the main save path. GBAGI saves normally to SRAM first,
 * then calls this once as a harmless trigger so the auto batteryless payload
 * can start its flush countdown.
 */
AutoHookWriteSram:
	.hword	0xB530		@ 30 B5
	.hword	0x1C05		@ 05 1C   add r5, r0, #0
	.hword	0x1C0C		@ 0C 1C   add r4, r1, #0
	.hword	0x1C13		@ 13 1C   add r3, r2, #0
	.hword	0x4A0B		@ 0B 4A   ldr r2, [pc, #44]
	.hword	0x8810		@ 10 88   ldrh r0, [r2, #0]
	.hword	0x490B		@ 0B 49   ldr r1, [pc, #44]
	.hword	0x4008		@ 08 40   and r0, r1

	add	r2, r2, r4
	cmp	r3, #0
	beq	.Ldone

.Lloop:
	ldrb	r0, [r5, #0]
	strb	r0, [r2, #0]
	add	r5, r5, #1
	add	r2, r2, #1
	sub	r3, r3, #1
	bne	.Lloop

.Ldone:
	ldrh	r0, [r2, #0]
	pop	{r4, r5, pc}

	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

	.align	2
.Lbase:
	.word	0x0E000000

.Lmask:
	.word	0x0000FFFF

	.align	2
	.global	BatterylessCallArm
	.thumb_func

/* BatterylessCallArm(armAddr=r0): call an ARM entrypoint and return to thumb. */
BatterylessCallArm:
	push	{r4, lr}
	mov		r4, r0
	ldr		r0, .LretThumb
	mov		lr, r0
	bx		r4
.LretHere:
	pop		{r4, pc}

	.align	2
.LretThumb:
	.word	.LretHere + 1

	.align	2
	.global	BatterylessCallThumb
	.thumb_func

/* BatterylessCallThumb(thumbAddr=r0): call a THUMB entrypoint and return. */
BatterylessCallThumb:
	push	{r4, lr}
	mov		r4, r0
	ldr		r0, .LretThumbCall
	mov		lr, r0
	bx		r4
.LretThumbHere:
	pop		{r4, pc}

	.align	2
.LretThumbCall:
	.word	.LretThumbHere + 1

	.align	2
	.global	BatterylessManualFlush
	.thumb_func

/* BatterylessManualFlush(): find the manual payload entry from the IRQ vector
 * and call it only when the ROM is patched in keypad/manual mode. */
BatterylessManualFlush:
	push	{r4, r5, lr}
	ldr		r4, .LirqVector
	ldr		r0, [r4]
	ldr		r1, .LromStart
	cmp		r0, r1
	blo		.LmanualDone
	ldr		r1, .LromEnd
	cmp		r0, r1
	bhs		.LmanualDone

	ldr		r1, .LkeypadHandlerOffset
	sub		r0, r0, r1
	ldr		r1, .LflushModeOffset
	ldrh	r1, [r0, r1]
	cmp		r1, #1
	bne		.LmanualDone

	ldr		r1, .LmanualEntryOffset
	add		r0, r0, r1
	bl		BatterylessCallArm

.LmanualDone:
	pop		{r4, r5, pc}

	.align	2
.LirqVector:
	.word	0x03FFFFFC
.LromStart:
	.word	0x08000000
.LromEnd:
	.word	0x0A000000
.LkeypadHandlerOffset:
	.word	0x00000154
.LflushModeOffset:
	.word	0x00000004
.LmanualEntryOffset:
	.word	0x00000020
