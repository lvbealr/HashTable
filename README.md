<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/krVMYr2.png" alt="Project Logo"></a>
</p>

---

<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/4dyOrbF.jpeg" alt="struct"></a>
</p>

---

`MAX_LOAD_FACTOR = 15`

<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/ZFhm4Tg.png" alt="pjw32"></a>
</p>

`Dispersion: 3769.2957`

---

<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/wKQpBWL.png" alt="adler32"></a>
</p>

`Dispersion: 587.0115`

---

<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/WHYCYpN.png" alt="sdbm32"></a>
</p>

`Dispersion: 20.7059`

---

<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/1IDbXQq.png" alt="fnv32"></a>
</p>

`Dispersion: 18.1092`

---

<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/airAcEB.png" alt="murmur3"></a>
</p>

`Dispersion: 17.6561`

---

<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/gPV8Cks.png" alt="crc32"></a>
</p>

`Dispersion: 17.0106`

---

-O0 (no optimizations)
```shell
Hash Function   : Time (in ticks):
-------------------------------------------------
pjw32           : 42862363
adler32         : 9778834
sdbm32          : 5808845
fnv32           : 5709340
murmur3         : 7621252
crc32           : 13028882
-------------------------------------------------
```

-O3 (no optimizations)
```shell

Hash Function   : Time (in ticks):
-------------------------------------------------
pjw32           : 28179724
adler32         : 7981539
sdbm32          : 4470152
fnv32           : 4689847
murmur3         : 6327387
crc32           : 7349910
-------------------------------------------------
```

---

FIRST STEP: try to optimize crc32 by intrinsics
<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/f0gAewl.png" alt="FIRST"></a>
</p>

```shell
Hash Function   : Time (in ticks):
-------------------------------------------------
pjw32           : 30791079
adler32         : 8457270
sdbm32          : 4503297
fnv32           : 4405018
murmur3         : 6389746
crc32           : 4586641
-------------------------------------------------
```

---

```c++
uint32_t crc32(string *data) {
    #ifndef OPTIMIZE_CRC32
        char  *message = data->data;
        size_t length  = data->size;

        uint32_t crc = 0xFFFFFFFF;

        for (size_t i = 0; i < length; i++) {
            crc ^= (uint32_t)message[i];

            for (size_t j = 0; j < 8; j++) {
                crc = (crc >> 1) ^ (0x82F63B78 & -(crc & 1)); // crc32 - castagnoli
            }
        }

        return crc ^ 0xFFFFFFFF;
    #else
        uint32_t crc32 = 0xFFFFFFFF;

        unsigned char *buffer = (unsigned char *)data->data;
        size_t         length = data->size;

        size_t quotient = length / 8;

        while (quotient--) {
            crc32 = _mm_crc32_u64(crc32, *(uint64_t *)buffer);
            buffer += 8;
        }

        if (length & 4) {
            crc32 = _mm_crc32_u32(crc32, *(uint32_t *)buffer);
            buffer += 4;
        }

        if (length & 2) {
            crc32 = _mm_crc32_u16(crc32, *(uint16_t *)buffer);
            buffer += 2;
        }

        if (length & 1) {
            crc32 = _mm_crc32_u8(crc32, *(uint8_t *)buffer);
        }

        return crc32 ^ 0xFFFFFFFF;
    #endif
}
```

OK! (optimized with intrinsics) 
<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/45ZiKaB.png" alt="optimizecrc32"></a>
</p>

---

```nasm
global myStrcmp

section .text

myStrcmp:
    xor rdx, rdx
    xor rax, rax

.loop:
    movdqu xmm1, [rdi + rdx]
    pcmpistri xmm1, [rsi + rdx], 0x18

    jc .diff
    jz .equal

    add rdx, 16
    jmp .loop

.equal:
    xor eax, eax
    ret

.diff:
    lea r9, [rdx + rcx]
    movzx eax, byte [rdi + r9]
    movzx r8d, byte [rsi + r9]
    sub eax, r8d
    ret
```
<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/wFeEK8J.png" alt="optimizestrcmp"></a>
</p>

```shell
Hash Function   : Time (in ticks):
-------------------------------------------------
pjw32           : 17755750
adler32         : 6446420
sdbm32          : 4195657
fnv32           : 3930603
murmur3         : 4325159
crc32           : 4158028
-------------------------------------------------
```
