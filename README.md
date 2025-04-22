<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/krVMYr2.png" alt="Project Logo"></a>
</p>

## 📖 Version / Версия
- [🇷🇺 RU](#RU)
- [🇺🇸 ENG](#ENG)

---

## 🇷🇺 RU <a name="RU"></a>

## 📝 Содержимое

- [О проекте](#aboutRU)
- [Принцип работы](#how_it_worksRU)
- [Анализ хэш-функций](#hash_functionsRU)
- [Платформозависимые оптимизации](#optimizationsRU)
- [Заключение](#conclusionRU)
- [Установка](#installationRU)
- [Инструменты](#built_usingRU)
- [Авторы](#authorsRU)

---

## 🧐 О проекте <a name="aboutRU"></a>

Проект представляет собой реализацию оптимизированной хэш-таблицы для хранения и быстрого извлечения данных. Хэш-таблица использует различные хэш-функции для распределения данных по ячейкам памяти, минимизируя коллизии и ускоряя доступ к данным. Разрешение коллизий осуществляется [методом цепочек](https://neerc.ifmo.ru/wiki/index.php?title=%D0%A0%D0%B0%D0%B7%D1%80%D0%B5%D1%88%D0%B5%D0%BD%D0%B8%D0%B5_%D0%BA%D0%BE%D0%BB%D0%BB%D0%B8%D0%B7%D0%B8%D0%B9).

Оптимизирована функция поиска элементов в хэш-таблице с минимальным использованием платформозависимого кода. Для профилирования применялись инструменты [Valgrind](https://valgrind.org/), [Kcachegrind](https://github.com/KDE/kcachegrind) и [Perf](https://perf.wiki.kernel.org/index.php/Main_Page).

### Произведенные оптимизации:
- Оптимизация вычисления хэш-функции `crc32` с использованием Intel Intrinsics.
- Ускорение сравнения строк с использованием Intel Intrinsics.
- Реализация создания и инициализации ячейки в hash-bucket на языке assembler x86-64 (NASM).
- Развертка циклов и распараллеливание вычислений хэш-функции `adler32`.

---

## ⚙️ Принцип работы хэш-таблицы <a name="how_it_worksRU"></a>

Хэш-таблица обеспечивает ускоренный доступ к данным по сравнению с линейным поиском. Основной принцип — вычисление хэша для каждого элемента и использование его для адресации в массиве данных. При правильной реализации операции чтения и записи выполняются за **O(1)**. Однако коллизии (совпадение хэшей для разных данных) неизбежны. В проекте используется метод цепочек: каждая ячейка массива содержит указатель на двусвязный список элементов с одинаковым хэшем.

> [!NOTE]  
> Проект учебный, поэтому рассматривается хранение только уникальных данных.

<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/4dyOrbF.jpeg" alt="Структура хэш-таблицы"></a>
</p>

### Коэффициент заполнения (load factor)

Коэффициент заполнения — это отношение числа элементов в хэш-таблице к количеству ячеек массива. С ростом коэффициента увеличивается вероятность коллизий, что снижает производительность. В проекте выбран коэффициент заполнения **15**, обеспечивающий баланс между производительностью и использованием памяти. При превышении этого значения размер массива увеличивается в 2 раза, и данные перехэшируются.

---

## 📈 Анализ хэш-функций <a name="hash_functionsRU"></a>

Исследовалась производительность хэш-таблицы с различными хэш-функциями:
- **PJW32**
- **Adler32**
- **SDBM32**
- **FNV32**
- **Murmur3**
- **CRC32**

### Параметры тестирования
Тестировалась хэш-таблица со структурой:

```c++
struct string {
    char  *data;  // Указатель на строку
    size_t size;  // Длина строки
};
```

Тестовые данные — 19,289 уникальных слов (длина до 32 символов) из произведений Шекспира в нижнем регистре.

### Методика измерений
Хэш-таблица заполнялась словарем, а данные о заполненности bucket’ов экспортировались в CSV для построения гистограмм распределения хэшей.

### Результаты измерений

#### PJW32
- **Описание**: [PJW hash function](https://en.wikipedia.org/wiki/PJW_hash_function), предложена Питером Уайнбергером в 1981 году. Использует побитовые и арифметические операции. Хорошо работает с короткими строками и числами.
- **Дисперсия**: 3769.30
- **Код**:
```c++
uint32_t pjw32(string *data, uint32_t modulo) {
    char  *message = data->data;
    size_t length  = data->size;

    uint32_t hash = 0;
    uint32_t test = 0;

    for (size_t i = 0; i < length; i++) {
        hash = (hash << 4) + (uint8_t)(message[i]);

        if ((test = (hash & 0xF0000000)) != 0) {
            hash = ((hash ^ (test >> 24)) & 0xFFFFFFF);
        }
    }

    return hash % modulo;
}
```
- **Распределение**:
  <p align="center"><img src="https://i.imgur.com/ZFhm4Tg.png" alt="PJW32"></p>

#### Adler32
- **Описание**: [Adler-32](https://en.wikipedia.org/wiki/Adler-32), предложена в 1995 году. Использует два 16-битных числа. Быстрая и эффективная для коротких строк.
- **Дисперсия**: 587.01
- **Код**:
```c++
uint32_t adler32(string *data) {
    const char *message = data->data;
    size_t      length  = data->size;

    uint32_t a = 1;
    uint32_t b = 0;

    for (size_t i = 0; i < length; i++) {
        a = (a + (uint8_t)(message[i])) % 65521;
        b = (b + a) % 65521;
    }

    return (b << 16) + a;
}
```
- **Распределение**:
  <p align="center"><img src="https://i.imgur.com/wKQpBWL.png" alt="Adler32"></p>

#### SDBM32
- **Описание**: [SDBM](https://en.wikipedia.org/wiki/SDBM), предложена в 1997 году. Использует побитовые и арифметические операции. Хорошо работает с короткими строками.
- **Дисперсия**: 20.71
- **Код**:
```c++
uint32_t sdbm32(string *data) {
    char  *message = data->data;
    size_t length  = data->size;

    uint32_t hash = 0;

    for (size_t i = 0; i < length; i++) {
        hash = (uint8_t)(message[i]) + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}
```
- **Распределение**:
  <p align="center"><img src="https://i.imgur.com/WHYCYpN.png" alt="SDBM32"></p>

#### FNV32
- **Описание**: [Fowler–Noll–Vo](https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function), предложена в 1991 году. Эффективна для коротких строк.
- **Дисперсия**: 18.11
- **Код**:
```c++
uint32_t fnv32(string *data) {
    char  *message = data->data;
    size_t length  = data->size;

    uint32_t hash = 0x811C9DC5;

    for (size_t i = 0; i < length; i++) {
        hash ^= (uint8_t)(message[i]);
        hash *= 16777619;
    }

    return hash;
}
```
- **Распределение**:
  <p align="center"><img src="https://i.imgur.com/1IDbXQq.png" alt="FNV32"></p>

#### Murmur3
- **Описание**: [MurmurHash](https://en.wikipedia.org/wiki/MurmurHash), предложена в 2008 году. Обеспечивает хорошее распределение.
- **Дисперсия**: 17.66
- **Код**:
```c++
uint32_t loadWord(const char *p) {
    return (uint32_t)((uint8_t)p[0])
           | ((uint8_t)p[1] << 8)
           | ((uint8_t)p[2] << 16)
           | ((uint8_t)p[3] << 24);
}

uint32_t rotl32(uint32_t x, int8_t r) {
    return (x << r) | (x >> (32 - r));
}

uint32_t murmur3Wrapper(string *data, uint32_t seed) {
    return murmur3(data, seed);
}

uint32_t murmur3(string *data, uint32_t seed) {
    const char *message = data->data;
    size_t length = data->size;

    const uint32_t c1 = 0xCC9E2D51;
    const uint32_t c2 = 0x1B873593;
    const uint32_t c3 = 0x85EBCA6B;
    const uint32_t c4 = 0xC2B2AE35;

    uint32_t h       = seed;
    size_t   nblocks = length / 4;

    for (size_t i = 0; i < nblocks; i++) {
        uint32_t k = loadWord(message + i * 4);
        k *= c1;
        k = rotl32(k, 15);
        k *= c2;

        h ^= k;
        h = rotl32(h, 13);
        h = h * 5 + 0xE6546B64;
    }

    const uint8_t *tail = (const uint8_t*)(message + nblocks * 4);
    uint32_t k1 = 0;

    switch (length & 3) {
        case 3: k1 ^= tail[2] << 16;
        case 2: k1 ^= tail[1] << 8;
        case 1: k1 ^= tail[0];
            k1 *= c1;
            k1 = rotl32(k1, 15);
            k1 *= c2;
            h ^= k1;
    }

    h ^= length;
    h ^= h >> 16;
    h *= c3;
    h ^= h >> 13;
    h *= c4;
    h ^= h >> 16;

    return h;
}
```
- **Распределение**:
  <p align="center"><img src="https://i.imgur.com/airAcEB.png" alt="Murmur3"></p>

#### CRC32
- **Описание**: [Cyclic redundancy check](https://en.wikipedia.org/wiki/Cyclic_redundancy_check), предложена в 1961 году. Использует побитовые операции.
- **Дисперсия**: 17.01
- **Код**:
```c++
uint32_t crc32(string *data) {
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
- **Распределение**:
  <p align="center"><img src="https://i.imgur.com/gPV8Cks.png" alt="CRC32"></p>

### Выбор наиболее эффективной хэш-функции
При коэффициенте заполнения 15 функция **`CRC32`** показала лучшую дисперсию (17.01), обеспечивая оптимальное распределение данных. Поэтому она выбрана для дальнейших оптимизаций.

---

## 🛠️ Платформозависимые оптимизации <a name="optimizationsRU"></a>

Время выполнения измерялось инструкцией `__rdtsc` (такты процессора). Относительный прирост производительности:  
**δ = (1 - T_curr / T_prev) × 100%**,  
абсолютный:  
**Δ = (T_0 - T_curr) / T_0 × 100%**, где:
- **T_prev** — время предыдущей версии,
- **T_curr** — время текущей версии,
- **T_0** — время неоптимизированной версии.

> [!NOTE]  
> Тесты проводились на Intel Core i5-1235U (12 ядер, 16 потоков, 10 нм, 1.3 ГГц, буст до 4.4 ГГц).

### Базовые результаты
Сравнение времени работы хэш-функций с флагами компиляции `-O0` и `-O3`:

| Хэш-функция | Время (-O0, такты) | Время (-O3, такты) |
|-------------|---------------------|-------------------|
| PJW32       | 37,712,862,828      | 1,306,637,912     |
| Adler32     | 9,718,050,054       | 1,472,587,563     |
| SDBM32      | 5,339,882,153       | 1,319,988,597     |
| FNV32       | 5,637,812,992       | 1,303,962,430     |
| Murmur3     | 7,296,310,082       | 1,255,268,347     |
| CRC32       | 11,270,438,507      | 3,328,219,633     |

Относительный и абсолютный прирост производительности с `-O3`:

| Хэш-функция | Относительный прирост (%) | Абсолютный прирост (%) |
|-------------|---------------------------|------------------------|
| PJW32       | 96.53%                    | 96.53%                 |
| Adler32     | 84.83%                    | 84.83%                 |
| SDBM32      | 75.36%                    | 75.36%                 |
| FNV32       | 76.92%                    | 76.92%                 |
| Murmur3     | 82.80%                    | 82.80%                 |
| CRC32       | 70.52%                    | 70.52%                 |

#### Оптимизация хэш-функции crc32 с помощью Intel Intrinsics
Граф вызовов функций (Kcachegrind) показал, что основное время функции `crc32` занимает цикл вычислений:

<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/3kx1NKG.png" alt="Kcachegrind crc32"></a>
</p>

Использованы инструкции `_mm_crc32_uXX` для ускорения. Новая версия функции:

```c++
uint32_t crc32(string *data) {
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

Время сократилось с 3,328,219,633 до 1,241,451,508 тактов. Результаты:

| Хэш-функция | Время (-O0, такты) | Время (-O3, такты) | Время (-O3 + crc32 Intrinsics, такты) |
|-------------|---------------------|-------------------|---------------------------------------|
| PJW32       | 37,712,862,828      | 1,306,637,912     | 1,313,601,291                         |
| Adler32     | 9,718,050,054       | 1,472,587,563     | 1,491,444,247                         |
| SDBM32      | 5,339,882,153       | 1,319,988,597     | 1,309,027,952                         |
| FNV32       | 5,637,812,992       | 1,303,962,430     | 1,324,217,785                         |
| Murmur3     | 7,296,310,082       | 1,255,268,347     | 1,272,084,617                         |
| CRC32       | 11,270,438,507      | 3,328,219,633     | 1,241,451,508                         |

Прирост производительности:

| Хэш-функция | Относительный прирост (%) | Абсолютный прирост (%) |
|-------------|---------------------------|------------------------|
| PJW32       | -0.53%                    | 96.51%                 |
| Adler32     | -1.28%                    | 84.65%                 |
| SDBM32      | 0.83%                     | 75.48%                 |
| FNV32       | -1.55%                    | 76.51%                 |
| Murmur3     | -1.33%                    | 82.56%                 |
| CRC32       | 62.69%                    | 88.98%                 |

#### Оптимизация функции strcmp с помощью Intel Intrinsics
После оптимизации `crc32` функция `strcmp` стала узким местом:

<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/NWi0VVg.png" alt="Kcachegrind strcmp"></a>
</p>

Использованы `_mm256_loadu_si256` и `_mm256_cmpeq_epi8` для ускорения. Новая версия:

```c++
inline int fastStrcmp(const char *a, const char *b) {
    __m256i va = _mm256_loadu_si256((const __m256i *)a);
    __m256i vb = _mm256_loadu_si256((const __m256i *)b);

    __m256i  cmp  = _mm256_cmpeq_epi8(va, vb);
    uint32_t mask = _mm256_movemask_epi8(cmp);

    return (mask != 0xFFFFFFFF);
}
```

Результаты:

| Хэш-функция | Время (-O0, такты) | Время (-O3, такты) | Время (-O3 + crc32 Intrinsics, такты) | Время (-O3 + crc32 Intrinsics + strcmp Intrinsics, такты) |
|-------------|---------------------|-------------------|---------------------------------------|----------------------------------------------------------|
| PJW32       | 37,712,862,828      | 1,306,637,912     | 1,313,601,291                         | 1,324,039,074                                            |
| Adler32     | 9,718,050,054       | 1,472,587,563     | 1,491,444,247                         | 1,477,661,359                                            |
| SDBM32      | 5,339,882,153       | 1,319,988,597     | 1,309,027,952                         | 1,312,882,939                                            |
| FNV32       | 5,637,812,992       | 1,303,962,430     | 1,324,217,785                         | 1,331,798,050                                            |
| Murmur3     | 7,296,310,082       | 1,255,268,347     | 1,272,084,617                         | 1,280,916,477                                            |
| CRC32       | 11,270,438,507      | 3,328,219,633     | 1,241,451,508                         | 1,228,967,071                                            |

Прирост производительности:

| Хэш-функция | Относительный прирост (%) | Абсолютный прирост (%) |
|-------------|---------------------------|------------------------|
| PJW32       | -0.79%                    | 96.48%                 |
| Adler32     | 0.92%                     | 84.79%                 |
| SDBM32      | -0.29%                    | 75.41%                 |
| FNV32       | -0.57%                    | 76.37%                 |
| Murmur3     | -0.69%                    | 82.44%                 |
| CRC32       | 1.00%                     | 89.09%                 |

<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/dYRwIAV.png" alt="strcmp optimization"></a>
</p>

#### Оптимизация создания и инициализации ячейки в hash-bucket на языке assembler x86-64 (NASM)
Функция `createNode` также оказалась в топе вызовов:

<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/4oiAZmc.png" alt="Kcachegrind createNode"></a>
</p>

Ассемблерная версия (NASM):

```nasm
section .text
global createNode
extern malloc, free

createNode:
    push rbx
    push r12
    push r13

    mov r12, rdi                ; wordPtr
    mov r13, rsi                ; length

    mov edi, 16
    call malloc
    test rax, rax
    jz .error

    mov rbx, rax
    mov [rbx + 8], r13          ; newNode->size = length

    lea rdi, [r13 + 1]
    call malloc
    test rax, rax
    jz .freeError

    mov [rbx], rax              ; newNode->data = rax

    mov rdi, rax                ; destination
    mov rsi, r12                ; source
    mov rcx, r13                ; length
    rep movsb                   ; copy length byte
    mov byte [rax + r13], 0

    mov rax, rbx                ; return newNode
    jmp .end

.freeError:
    mov rdi, rbx
    call free

.error:
    xor rax, rax

.end:
    pop r13
    pop r12
    pop rbx

    ret
```

Результаты:

| Хэш-функция | Время (-O0, такты) | Время (-O3, такты) | Время (-O3 + crc32 Intrinsics, такты) | Время (-O3 + crc32 Intrinsics + strcmp Intrinsics, такты) | Время (-O3 + crc32 Intrinsics + strcmp Intrinsics + createNode, такты) |
|-------------|---------------------|-------------------|---------------------------------------|----------------------------------------------------------|------------------------------------------------------------------------|
| PJW32       | 37,712,862,828      | 1,306,637,912     | 1,313,601,291                         | 1,324,039,074                                            | 1,202,688,652                                                          |
| Adler32     | 9,718,050,054       | 1,472,587,563     | 1,491,444,247                         | 1,477,661,359                                            | 1,425,231,359                                                          |
| SDBM32      | 5,339,882,153       | 1,319,988,597     | 1,309,027,952                         | 1,312,882,939                                            | 1,193,839,445                                                          |
| FNV32       | 5,637,812,992       | 1,303,962,430     | 1,324,217,785                         | 1,331,798,050                                            | 1,219,107,790                                                          |
| Murmur3     | 7,296,310,082       | 1,255,268,347     | 1,272,084,617                         | 1,280,916,477                                            | 1,261,683,952                                                          |
| CRC32       | 11,270,438,507      | 3,328,219,633     | 1,241,451,508                         | 1,228,967,071                                            | 1,133,372,681                                                          |

Прирост производительности:

| Хэш-функция | Относительный прирост (%) | Абсолютный прирост (%) |
|-------------|---------------------------|------------------------|
| PJW32       | 9.16%                     | 96.81%                 |
| Adler32     | 3.55%                     | 85.34%                 |
| SDBM32      | 9.07%                     | 77.64%                 |
| FNV32       | 8.46%                     | 78.37%                 |
| Murmur3     | 1.50%                     | 82.71%                 |
| CRC32       | 7.78%                     | 89.94%                 |

<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/KFth290.png" alt="createNode optimization"></a>
</p>

#### Оптимизация функции adler32 с помощью ассемблерных вставок
Оптимизирована функция `adler32` с использованием ассемблерных вставок для развертки циклов:

```c++
uint32_t adler32(string *data) {
    const char *message = data->data;
    size_t length = data->size;

    uint32_t a = 1;
    uint32_t b = 0;

    #ifdef OPTIMIZE_ADLER32

    size_t i = 0;

    for (; i + 4 <= length; i += 4) {
        uint32_t bytes;
        memcpy(&bytes, message + i, 4);

        __asm__ volatile (
                "mov    %1,    %%ecx\n"
                "movzbl %%cl,  %%ecx\n"
                "addl   %%ecx, %0\n"
                "addl   %0,    %2\n"
                "mov    %1,    %%ecx\n"
                "shr    $8,    %%ecx\n"
                "movzbl %%cl,  %%ecx\n"
                "addl   %%ecx, %0\n"
                "addl   %0,    %2\n"
                "mov    %1,    %%ecx\n"
                "shr    $16,   %%ecx\n"
                "movzbl %%cl,  %%ecx\n"
                "addl   %%ecx, %0\n"
                "addl   %0,    %2\n"
                "mov    %1,    %%ecx\n"
                "shr    $24,   %%ecx\n"
                "movzbl %%cl,  %%ecx\n"
                "addl   %%ecx, %0\n"
                "addl   %0,    %2\n"
                : "+r"(a), "+r"(bytes), "+r"(b)
                :
                : "ecx"
        );
    }

    for (; i < length; i++) {
        a = (a + (uint8_t)(message[i])) % 65521;
        b = (b + a) % 65521;
    }

    a %= 65521;
    b %= 65521;

    #else
        for (size_t i = 0; i < length; i++) {
            a = (a + (uint8_t)(message[i])) % 65521;
            b = (b + a) % 65521;
        }
    #endif

    return (b << 16) + a;
}
```

Результаты:

| Хэш-функция | Время (-O0, такты) | Время (-O3, такты) | Время (-O3 + crc32 Intrinsics, такты) | Время (-O3 + crc32 Intrinsics + strcmp Intrinsics, такты) | Время (-O3 + crc32 Intrinsics + strcmp Intrinsics + createNode, такты) | Время (-O3 + crc32 Intrinsics + strcmp Intrinsics + createNode + adler32, такты) |
|-------------|---------------------|-------------------|---------------------------------------|----------------------------------------------------------|------------------------------------------------------------------------|---------------------------------------------------------------------------------|
| PJW32       | 37,712,862,828      | 1,306,637,912     | 1,313,601,291                         | 1,324,039,074                                            | 1,202,688,652                                                          | 1,211,860,754                                                                   |
| Adler32     | 9,718,050,054       | 1,472,587,563     | 1,491,444,247                         | 1,477,661,359                                            | 1,425,231,359                                                          | 1,162,102,435                                                                   |
| SDBM32      | 5,339,882,153       | 1,319,988,597     | 1,309,027,952                         | 1,312,882,939                                            | 1,193,839,445                                                          | 1,185,024,245                                                                   |
| FNV32       | 5,637,812,992       | 1,303,962,430     | 1,324,217,785                         | 1,331,798,050                                            | 1,219,107,790                                                          | 1,171,217,440                                                                   |
| Murmur3     | 7,296,310,082       | 1,255,268,347     | 1,272,084,617                         | 1,280,916,477                                            | 1,261,683,952                                                          | 1,104,467,606                                                                   |
| CRC32       | 11,270,438,507      | 3,328,219,633     | 1,241,451,508                         | 1,228,967,071                                            | 1,133,372,681                                                          | 1,066,876,350                                                                   |

Прирост производительности:

| Хэш-функция | Относительный прирост (%) | Абсолютный прирост (%) |
|-------------|---------------------------|------------------------|
| PJW32       | -0.76%                    | 96.79%                 |
| Adler32     | 18.46%                    | 88.04%                 |
| SDBM32      | 0.74%                     | 77.80%                 |
| FNV32       | 3.93%                     | 79.23%                 |
| Murmur3     | 12.46%                    | 84.86%                 |
| CRC32       | 5.87%                     | 90.53%                 |

<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/w0DTpGm.png" alt="adler32 optimization"></a>
</p>

---

## 🛐 Заключение <a name="conclusionRU"></a>

Исследовалась оптимизация хэш-функций и хэш-таблиц. Анализ показал, что `CRC32` обеспечивает лучшее распределение данных. Оптимизации с использованием Intel Intrinsics и ассемблера значительно ускорили работу функций `crc32`, `strcmp`, `createNode` и `adler32`. Итоговый прирост производительности:

| Хэш-функция | Абсолютный прирост (%) |
|-------------|------------------------|
| PJW32       | 96.79%                 |
| Adler32     | 88.04%                 |
| SDBM32      | 77.80%                 |
| FNV32       | 79.23%                 |
| Murmur3     | 84.86%                 |
| CRC32       | 90.53%                 |

Результаты зависят от реализации и платформы. Платформозависимые оптимизации эффективны, но снижают переносимость кода и усложняют поддержку. Их применение следует оценивать с учетом задач.

---

## ⚙️ Установка <a name="installationRU"></a>

1. Склонируйте репозиторий и обновите подмодули:
   ```shell
   git clone git@github.com:lvbealr/HashTable.git
   cd HashTable
   git submodule update --init --remote --recursive
   ```
2. Скомпилируйте проект:
   ```shell
   mkdir build && cd build
   cmake -S .. -B .
   make
   ```
3. Запустите программу:
   ```shell
   ./HashTable <путь_к_входным_данным> <путь_к_тестовым_данным>
   # Пример: ./HashTable ../txt/text.txt ../txt/test.txt
   ```

**Зависимости**: GCC/Clang, CMake 3.10+, NASM.

---

## ⛏ Инструменты <a name="built_usingRU"></a>

- [CMake](https://cmake.org/) — автоматизация сборки.
- [customWarning](https://github.com/lvbealr/customWarning) — пользовательские предупреждения.
- [colorPrint](https://github.com/lvbealr/colorPrint) — цветной вывод в консоль.

---

## ✍ Авторы <a name="authorsRU"></a>

- [@lvbealr](https://github.com/lvbealr) — разработка и реализация.

---

## 🇺🇸 ENG <a name="ENG"></a>

## 📝 Contents

- [About the Project](#aboutENG)
- [How it Works](#how_it_worksENG)
- [Hash Functions Analysis](#hash_functionsENG)
- [Platform-Dependent Optimizations](#optimizationsENG)
- [Conclusion](#conclusionENG)
- [Installation](#installationENG)
- [Built Using](#built_usingENG)
- [Authors](#authorsENG)

---

## 🧐 About the Project <a name="aboutENG"></a>

This project implements an optimized hash table for storing and quickly retrieving data. The hash table employs various hash functions to distribute data across memory cells, minimizing collisions and accelerating access. Collisions are resolved using the [chaining method](https://en.wikipedia.org/wiki/Hash_table#Collision_resolution).

The element search function was optimized with minimal platform-dependent code. Profiling was performed using [Valgrind](https://valgrind.org/), [Kcachegrind](https://github.com/KDE/kcachegrind), and [Perf](https://perf.wiki.kernel.org/index.php/Main_Page).

### Performed Optimizations:
- Optimization of the `crc32` hash function using Intel Intrinsics.
- Acceleration of string comparison using Intel Intrinsics.
- Implementation of node creation and initialization in the hash table using x86-64 assembly (NASM).
- Loop unrolling and parallelization of the `adler32` hash function.

---

## ⚙️ How the Hash Table Works <a name="how_it_worksENG"></a>

A hash table provides faster data access compared to linear search. The core principle is to compute a hash for each element and use it as an address in the data array. With proper implementation, read and write operations achieve **O(1)** complexity. However, collisions (when different data produce the same hash) are inevitable. The project uses chaining: each array cell contains a pointer to a doubly linked list of elements with the same hash.

> [!NOTE]  
> This is an educational project, so only unique data storage is considered.

<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/4dyOrbF.jpeg" alt="Hash Table Structure"></a>
</p>

### Load Factor

The load factor is the ratio of the number of elements to the number of cells in the array. As the load factor increases, the probability of collisions rises, reducing performance. A load factor of **15** is used to balance performance and memory usage. When exceeded, the array size doubles, and data is rehashed.

---

## 📈 Hash Functions Analysis <a name="hash_functionsENG"></a>

The performance of the hash table was analyzed with the following hash functions:
- **PJW32**
- **Adler32**
- **SDBM32**
- **FNV32**
- **Murmur3**
- **CRC32**

### Testing Parameters
The hash table stored structures of the form:

```c++
struct string {
    char  *data;  // Pointer to the string
    size_t size;  // Length of the string
};
```

Test data consisted of 19,289 unique words (up to 32 characters) from Shakespeare's works in lowercase.

### Measurement Methodology
The hash table was filled with the dictionary, and bucket occupancy data was exported to CSV for histogram plotting.

### Measurement Results

#### PJW32
- **Description**: [PJW hash function](https://en.wikipedia.org/wiki/PJW_hash_function), proposed by Peter Weinberger in 1981. Uses bitwise and arithmetic operations. Effective for short strings and numbers.
- **Variance**: 3769.30
- **Code**:
```c++
uint32_t pjw32(string *data, uint32_t modulo) {
    char  *message = data->data;
    size_t length  = data->size;

    uint32_t hash = 0;
    uint32_t test = 0;

    for (size_t i = 0; i < length; i++) {
        hash = (hash << 4) + (uint8_t)(message[i]);

        if ((test = (hash & 0xF0000000)) != 0) {
            hash = ((hash ^ (test >> 24)) & 0xFFFFFFF);
        }
    }

    return hash % modulo;
}
```
- **Distribution**:
  <p align="center"><img src="https://i.imgur.com/ZFhm4Tg.png" alt="PJW32"></p>

#### Adler32
- **Description**: [Adler-32](https://en.wikipedia.org/wiki/Adler-32), proposed in 1995. Uses two 16-bit numbers. Fast and efficient for short strings.
- **Variance**: 587.01
- **Code**:
```c++
uint32_t adler32(string *data) {
    const char *message = data->data;
    size_t      length  = data->size;

    uint32_t a = 1;
    uint32_t b = 0;

    for (size_t i = 0; i < length; i++) {
        a = (a + (uint8_t)(message[i])) % 65521;
        b = (b + a) % 65521;
    }

    return (b << 16) + a;
}
```
- **Distribution**:
  <p align="center"><img src="https://i.imgur.com/wKQpBWL.png" alt="Adler32"></p>

#### SDBM32
- **Description**: [SDBM](https://en.wikipedia.org/wiki/SDBM), proposed in 1997. Uses bitwise and arithmetic operations. Effective for short strings.
- **Variance**: 20.71
- **Code**:
```c++
uint32_t sdbm32(string *data) {
    char  *message = data->data;
    size_t length  = data->size;

    uint32_t hash = 0;

    for (size_t i = 0; i < length; i++) {
        hash = (uint8_t)(message[i]) + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}
```
- **Distribution**:
  <p align="center"><img src="https://i.imgur.com/WHYCYpN.png" alt="SDBM32"></p>

#### FNV32
- **Description**: [Fowler–Noll–Vo](https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function), proposed in 1991. Efficient for short strings.
- **Variance**: 18.11
- **Code**:
```c++
uint32_t fnv32(string *data) {
    char  *message = data->data;
    size_t length  = data->size;

    uint32_t hash = 0x811C9DC5;

    for (size_t i = 0; i < length; i++) {
        hash ^= (uint8_t)(message[i]);
        hash *= 16777619;
    }

    return hash;
}
```
- **Distribution**:
  <p align="center"><img src="https://i.imgur.com/1IDbXQq.png" alt="FNV32"></p>

#### Murmur3
- **Description**: [MurmurHash](https://en.wikipedia.org/wiki/MurmurHash), proposed in 2008. Provides good distribution.
- **Variance**: 17.66
- **Code**:
```c++
uint32_t loadWord(const char *p) {
    return (uint32_t)((uint8_t)p[0])
           | ((uint8_t)p[1] << 8)
           | ((uint8_t)p[2] << 16)
           | ((uint8_t)p[3] << 24);
}

uint32_t rotl32(uint32_t x, int8_t r) {
    return (x << r) | (x >> (32 - r));
}

uint32_t murmur3Wrapper(string *data, uint32_t seed) {
    return murmur3(data, seed);
}

uint32_t murmur3(string *data, uint32_t seed) {
    const char *message = data->data;
    size_t length = data->size;

    const uint32_t c1 = 0xCC9E2D51;
    const uint32_t c2 = 0x1B873593;
    const uint32_t c3 = 0x85EBCA6B;
    const uint32_t c4 = 0xC2B2AE35;

    uint32_t h       = seed;
    size_t   nblocks = length / 4;

    for (size_t i = 0; i < nblocks; i++) {
        uint32_t k = loadWord(message + i * 4);
        k *= c1;
        k = rotl32(k, 15);
        k *= c2;

        h ^= k;
        h = rotl32(h, 13);
        h = h * 5 + 0xE6546B64;
    }

    const uint8_t *tail = (const uint8_t*)(message + nblocks * 4);
    uint32_t k1 = 0;

    switch (length & 3) {
        case 3: k1 ^= tail[2] << 16;
        case 2: k1 ^= tail[1] << 8;
        case 1: k1 ^= tail[0];
            k1 *= c1;
            k1 = rotl32(k1, 15);
            k1 *= c2;
            h ^= k1;
    }

    h ^= length;
    h ^= h >> 16;
    h *= c3;
    h ^= h >> 13;
    h *= c4;
    h ^= h >> 16;

    return h;
}
```
- **Distribution**:
  <p align="center"><img src="https://i.imgur.com/airAcEB.png" alt="Murmur3"></p>

#### CRC32
- **Description**: [Cyclic redundancy check](https://en.wikipedia.org/wiki/Cyclic_redundancy_check), proposed in 1961. Uses bitwise operations.
- **Variance**: 17.01
- **Code**:
```c++
uint32_t crc32(string *data) {
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
- **Distribution**:
  <p align="center"><img src="https://i.imgur.com/gPV8Cks.png" alt="CRC32"></p>

### Selection of the Most Efficient Hash Function
With a load factor of 15, **`CRC32`** demonstrated the best variance (17.01), ensuring optimal data distribution. It was chosen for further optimizations.

---

## 🛠️ Platform-Dependent Optimizations <a name="optimizationsENG"></a>

Execution time was measured using the `__rdtsc` instruction (CPU cycles). Relative performance improvement:  
**δ = (1 - T_curr / T_prev) × 100%**,  
absolute:  
**Δ = (T_0 - T_curr) / T_0 × 100%**, where:
- **T_prev** — previous version time,
- **T_curr** — current version time,
- **T_0** — unoptimized version time.

> [!NOTE]  
> Tests were conducted on an Intel Core i5-1235U (12 cores, 16 threads, 10 nm, 1.3 GHz, boost to 4.4 GHz).

### Baseline Results
Comparison of hash function execution times with `-O0` and `-O3` flags:

| Hash Function | Time (-O0, cycles) | Time (-O3, cycles) |
|---------------|---------------------|-------------------|
| PJW32         | 37,712,862,828      | 1,306,637,912     |
| Adler32       | 9,718,050,054       | 1,472,587,563     |
| SDBM32        | 5,339,882,153       | 1,319,988,597     |
| FNV32         | 5,637,812,992       | 1,303,962,430     |
| Murmur3       | 7,296,310,082       | 1,255,268,347     |
| CRC32         | 11,270,438,507      | 3,328,219,633     |

Performance improvements with `-O3`:

| Hash Function | Relative Improvement (%) | Absolute Improvement (%) |
|---------------|--------------------------|--------------------------|
| PJW32         | 96.53%                   | 96.53%                   |
| Adler32       | 84.83%                   | 84.83%                   |
| SDBM32        | 75.36%                   | 75.36%                   |
| FNV32         | 76.92%                   | 76.92%                   |
| Murmur3       | 82.80%                   | 82.80%                   |
| CRC32         | 70.52%                   | 70.52%                   |

#### CRC32 Optimization with Intel Intrinsics
A Kcachegrind call graph showed that the `crc32` function's main bottleneck was its computation loop:

<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/3kx1NKG.png" alt="Kcachegrind crc32"></a>
</p>

The `_mm_crc32_uXX` instructions were used for acceleration. New version:

```c++
uint32_t crc32(string *data) {
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

Time reduced from 3,328,219,633 to 1,241,451,508 cycles. Results:

| Hash Function | Time (-O0, cycles) | Time (-O3, cycles) | Time (-O3 + crc32 Intrinsics, cycles) |
|---------------|---------------------|-------------------|---------------------------------------|
| PJW32         | 37,712,862,828      | 1,306,637,912     | 1,313,601,291                         |
| Adler32       | 9,718,050,054       | 1,472,587,563     | 1,491,444,247                         |
| SDBM32        | 5,339,882,153       | 1,319,988,597     | 1,309,027,952                         |
| FNV32         | 5,637,812,992       | 1,303,962,430     | 1,324,217,785                         |
| Murmur3       | 7,296,310,082       | 1,255,268,347     | 1,272,084,617                         |
| CRC32         | 11,270,438,507      | 3,328,219,633     | 1,241,451,508                         |

Performance improvements:

| Hash Function | Relative Improvement (%) | Absolute Improvement (%) |
|---------------|--------------------------|--------------------------|
| PJW32         | -0.53%                   | 96.51%                   |
| Adler32       | -1.28%                   | 84.65%                   |
| SDBM32        | 0.83%                    | 75.48%                   |
| FNV32         | -1.55%                   | 76.51%                   |
| Murmur3       | -1.33%                   | 82.56%                   |
| CRC32         | 62.69%                   | 88.98%                   |

#### strcmp Optimization with Intel Intrinsics
After optimizing `crc32`, `strcmp` became a bottleneck:

<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/NWi0VVg.png" alt="Kcachegrind strcmp"></a>
</p>

The `_mm256_loadu_si256` and `_mm256_cmpeq_epi8` instructions were used. New version:

```c++
inline int fastStrcmp(const char *a, const char *b) {
    __m256i va = _mm256_loadu_si256((const __m256i *)a);
    __m256i vb = _mm256_loadu_si256((const __m256i *)b);

    __m256i  cmp  = _mm256_cmpeq_epi8(va, vb);
    uint32_t mask = _mm256_movemask_epi8(cmp);

    return (mask != 0xFFFFFFFF);
}
```

Results:

| Hash Function | Time (-O0, cycles) | Time (-O3, cycles) | Time (-O3 + crc32 Intrinsics, cycles) | Time (-O3 + crc32 Intrinsics + strcmp Intrinsics, cycles) |
|---------------|---------------------|-------------------|---------------------------------------|----------------------------------------------------------|
| PJW32         | 37,712,862,828      | 1,306,637,912     | 1,313,601,291                         | 1,324,039,074                                            |
| Adler32       | 9,718,050,054       | 1,472,587,563     | 1,491,444,247                         | 1,477,661,359                                            |
| SDBM32        | 5,339,882,153       | 1,319,988,597     | 1,309,027,952                         | 1,312,882,939                                            |
| FNV32         | 5,637,812,992       | 1,303,962,430     | 1,324,217,785                         | 1,331,798,050                                            |
| Murmur3       | 7,296,310,082       | 1,255,268,347     | 1,272,084,617                         | 1,280,916,477                                            |
| CRC32         | 11,270,438,507      | 3,328,219,633     | 1,241,451,508                         | 1,228,967,071                                            |

Performance improvements:

| Hash Function | Relative Improvement (%) | Absolute Improvement (%) |
|---------------|--------------------------|--------------------------|
| PJW32         | -0.79%                   | 96.48%                   |
| Adler32       | 0.92%                    | 84.79%                   |
| SDBM32        | -0.29%                   | 75.41%                   |
| FNV32         | -0.57%                   | 76.37%                   |
| Murmur3       | -0.69%                   | 82.44%                   |
| CRC32         | 1.00%                    | 89.09%                   |

<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/dYRwIAV.png" alt="strcmp optimization"></a>
</p>

#### Node Creation Optimization in x86-64 Assembly (NASM)
The `createNode` function was identified as a bottleneck:

<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/4oiAZmc.png" alt="Kcachegrind createNode"></a>
</p>

An assembly version (NASM) was implemented:

```nasm
section .text
global createNode
extern malloc, free

createNode:
    push rbx
    push r12
    push r13

    mov r12, rdi                ; wordPtr
    mov r13, rsi                ; length

    mov edi, 16
    call malloc
    test rax, rax
    jz .error

    mov rbx, rax
    mov [rbx + 8], r13          ; newNode->size = length

    lea rdi, [r13 + 1]
    call malloc
    test rax, rax
    jz .freeError

    mov [rbx], rax              ; newNode->data = rax

    mov rdi, rax                ; destination
    mov rsi, r12                ; source
    mov rcx, r13                ; length
    rep movsb                   ; copy length byte
    mov byte [rax + r13], 0

    mov rax, rbx                ; return newNode
    jmp .end

.freeError:
    mov rdi, rbx
    call free

.error:
    xor rax, rax

.end:
    pop r13
    pop r12
    pop rbx

    ret
```

Results:

| Hash Function | Time (-O0, cycles) | Time (-O3, cycles) | Time (-O3 + crc32 Intrinsics, cycles) | Time (-O3 + crc32 Intrinsics + strcmp Intrinsics, cycles) | Time (-O3 + crc32 Intrinsics + strcmp Intrinsics + createNode, cycles) |
|---------------|---------------------|-------------------|---------------------------------------|----------------------------------------------------------|------------------------------------------------------------------------|
| PJW32         | 37,712,862,828      | 1,306,637,912     | 1,313,601,291                         | 1,324,039,074                                            | 1,202,688,652                                                          |
| Adler32       | 9,718,050,054       | 1,472,587,563     | 1,491,444,247                         | 1,477,661,359                                            | 1,425,231,359                                                          |
| SDBM32        | 5,339,882,153       | 1,319,988,597     | 1,309,027,952                         | 1,312,882,939                                            | 1,193,839,445                                                          |
| FNV32         | 5,637,812,992       | 1,303,962 blows,430     | 1,324,217,785                         | 1,331,798,050                                            | 1,219,107,790                                                          |
| Murmur3       | 7,296,310,082       | 1,255,268,347     | 1,272,084,617                         | 1,280,916,477                                            | 1,261,683,952                                                          |
| CRC32         | 11,270,438,507      | 3,328,219,633     | 1,241,451,508                         | 1,228,967,071                                            | 1,133,372,681                                                          |

Performance improvements:

| Hash Function | Relative Improvement (%) | Absolute Improvement (%) |
|---------------|--------------------------|--------------------------|
| PJW32         | 9.16%                    | 96.81%                   |
| Adler32       | 3.55%                    | 85.34%                   |
| SDBM32        | 9.07%                    | 77.64%                   |
| FNV32         | 8.46%                    | 78.37%                   |
| Murmur3       | 1.50%                    | 82.71%                   |
| CRC32         | 7.78%                    | 89.94%                   |

<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/KFth290.png" alt="createNode optimization"></a>
</p>

#### Adler32 Optimization with Assembly Inlines
The `adler32` function was optimized using assembly inlines for loop unrolling:

```c++
uint32_t adler32(string *data) {
    const char *message = data->data;
    size_t length = data->size;

    uint32_t a = 1;
    uint32_t b = 0;

    #ifdef OPTIMIZE_ADLER32

    size_t i = 0;

    for (; i + 4 <= length; i += 4) {
        uint32_t bytes;
        memcpy(&bytes, message + i, 4);

        __asm__ volatile (
                "mov    %1,    %%ecx\n"
                "movzbl %%cl,  %%ecx\n"
                "addl   %%ecx, %0\n"
                "addl   %0,    %2\n"
                "mov    %1,    %%ecx\n"
                "shr    $8,    %%ecx\n"
                "movzbl %%cl,  %%ecx\n"
                "addl   %%ecx, %0\n"
                "addl   %0,    %2\n"
                "mov    %1,    %%ecx\n"
                "shr    $16,   %%ecx\n"
                "movzbl %%cl,  %%ecx\n"
                "addl   %%ecx, %0\n"
                "addl   %0,    %2\n"
                "mov    %1,    %%ecx\n"
                "shr    $24,   %%ecx\n"
                "movzbl %%cl,  %%ecx\n"
                "addl   %%ecx, %0\n"
                "addl   %0,    %2\n"
                : "+r"(a), "+r"(bytes), "+r"(b)
                :
                : "ecx"
        );
    }

    for (; i < length; i++) {
        a = (a + (uint8_t)(message[i])) % 65521;
        b = (b + a) % 65521;
    }

    a %= 65521;
    b %= 65521;

    #else
        for (size_t i = 0; i < length; i++) {
            a = (a + (uint8_t)(message[i])) % 65521;
            b = (b + a) % 65521;
        }
    #endif

    return (b << 16) + a;
}
```

Results:

| Hash Function | Time (-O0, cycles) | Time (-O3, cycles) | Time (-O3 + crc32 Intrinsics, cycles) | Time (-O3 + crc32 Intrinsics + strcmp Intrinsics, cycles) | Time (-O3 + crc32 Intrinsics + strcmp Intrinsics + createNode, cycles) | Time (-O3 + crc32 Intrinsics + strcmp Intrinsics + createNode + adler32, cycles) |
|---------------|---------------------|-------------------|---------------------------------------|----------------------------------------------------------|------------------------------------------------------------------------|---------------------------------------------------------------------------------|
| PJW32         | 37,712,862,828      | 1,306,637,912     | 1,313,601,291                         | 1,324,039,074                                            | 1,202,688,652                                                          | 1,211,860,754                                                                   |
| Adler32       | 9,718,050,054       | 1,472,587,563     | 1,491,444,247                         | 1,477,661,359                                            | 1,425,231,359                                                          | 1,162,102,435                                                                   |
| SDBM32        | 5,339,882,153       | 1,319,988,597     | 1,309,027,952                         | 1,312,882,939                                            | 1,193,839,445                                                          | 1,185,024,245                                                                   |
| FNV32         | 5,637,812,992       | 1,303,962,430     | 1,324,217,785                         | 1,331,798,050                                            | 1,219,107,790                                                          | 1,171,217,440                                                                   |
| Murmur3       | 7,296,310,082       | 1,255,268,347     | 1,272,084,617                         | 1,280,916,477                                            | 1,261,683,952                                                          | 1,104,467,606                                                                   |
| CRC32         | 11,270,438,507      | 3,328,219,633     | 1,241,451,508                         | 1,228,967,071                                            | 1,133,372,681                                                          | 1,066,876,350                                                                   |

Performance improvements:

| Hash Function | Relative Improvement (%) | Absolute Improvement (%) |
|---------------|--------------------------|--------------------------|
| PJW32         | -0.76%                   | 96.79%                   |
| Adler32       | 18.46%                   | 88.04%                   |
| SDBM32        | 0.74%                    | 77.80%                   |
| FNV32         | 3.93%                    | 79.23%                   |
| Murmur3       | 12.46%                   | 84.86%                   |
| CRC32         | 5.87%                    | 90.53%                   |

<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/w0DTpGm.png" alt="adler32 optimization"></a>
</p>

---

## 🛐 Conclusion <a name="conclusionENG"></a>

The project investigated the optimization of hash functions and hash tables. Analysis showed that `CRC32` provides the best data distribution. Optimizations using Intel Intrinsics and assembly significantly accelerated the `crc32`, `strcmp`, `createNode`, and `adler32` functions. Final performance improvements:

| Hash Function | Absolute Improvement (%) |
|---------------|--------------------------|
| PJW32         | 96.79%                   |
| Adler32       | 88.04%                   |
| SDBM32        | 77.80%                   |
| FNV32         | 79.23%                   |
| Murmur3       | 84.86%                   |
| CRC32         | 90.53%                   |

Results depend on the implementation and platform. Platform-dependent optimizations are effective but reduce code portability and increase maintenance complexity. Their use should be evaluated based on specific tasks.

---

## ⚙️ Installation <a name="installationENG"></a>

1. Clone the repository and update submodules:
   ```shell
   git clone git@github.com:lvbealr/HashTable.git
   cd HashTable
   git submodule update --init --remote --recursive
   ```
2. Build the project:
   ```shell
   mkdir build && cd build
   cmake -S .. -B .
   make
   ```
3. Run the program:
   ```shell
   ./HashTable <path_to_input_data> <path_to_test_data>
   # Example: ./HashTable ../txt/text.txt ../txt/test.txt
   ```

**Dependencies**: GCC/Clang, CMake 3.10+, NASM.

---

## ⛏ Built Using <a name="built_usingENG"></a>

- [CMake](https://cmake.org/) — Build automation.
- [customWarning](https://github.com/lvbealr/customWarning) — Custom warnings.
- [colorPrint](https://github.com/lvbealr/colorPrint) — Colored console output.

---

## ✍ Authors <a name="authorsENG"></a>

- [@lvbealr](https://github.com/lvbealr) — Development and implementation.

---