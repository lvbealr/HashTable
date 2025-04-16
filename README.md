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

FIRST STEP: try to optimize crc32 by intrinsics
<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/f0gAewl.png" alt="FIRST"></a>
</p>

```shell
Hash Function   : Time (in ticks):
-------------------------------------------------
pjw32           : 29643795
adler32         : 8165153
sdbm32          : 4751951
fnv32           : 4642365
murmur3         : 6266542
crc32           : 7542501
crc32SSE        : 5871276
-------------------------------------------------
```

```c++
uint32_t crc32SSE(string *data) {
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
}
```