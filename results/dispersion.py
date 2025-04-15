import pandas as pd
import matplotlib.pyplot as plt

def readCsvData(fileName):
    return pd.read_csv(fileName, header = None, names = ['Bucket Index', 'Bucket Size'])

def findDispersion(data):
    mean = data['Bucket Size'].mean()
    variance = ((data['Bucket Size'] - mean) ** 2).mean()
    return variance

hashFunctionNames = ['adler32', 'crc32', 'fnv32', 'murmur3', 'pjw32', 'sdbm32']

adler32 = readCsvData('data/adler32.csv')
crc32   = readCsvData('data/crc32.csv')
fnv32   = readCsvData('data/fnv32.csv')
murmur3 = readCsvData('data/murmur3.csv')
pjw32   = readCsvData('data/pjw32.csv')
sdbm32  = readCsvData('data/sdbm32.csv')

dispersionData = {
    'adler32': findDispersion(adler32),
    'crc32':   findDispersion(crc32),
    'fnv32':   findDispersion(fnv32),
    'murmur3': findDispersion(murmur3),
    'pjw32':   findDispersion(pjw32),
    'sdbm32':  findDispersion(sdbm32)
}

for name, dispersion in dispersionData.items():
    print(f"{name}: {dispersion:.4f}")
