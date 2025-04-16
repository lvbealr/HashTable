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

-O0
```shell
Hash Function   : Time (in ticks):
-------------------------------------------------
pjw32           : 38738975
adler32         : 9678774
sdbm32          : 5813982
fnv32           : 5660590
murmur3         : 7433759
crc32           : 12528555
-------------------------------------------------
```

-O3
```shell
Hash Function   : Time (in ticks):
-------------------------------------------------
pjw32           : 27264705
adler32         : 8927890
sdbm32          : 4912525
fnv32           : 4829567
murmur3         : 6259691
crc32           : 7155720
-------------------------------------------------
```