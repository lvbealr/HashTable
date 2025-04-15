import pandas as pd
import matplotlib.pyplot as plt

def readCsvData(fileName):
    return pd.read_csv(fileName, header = None, names = ['Bucket Index', 'Bucket Size'])

def plotAndSave(data, outputFileName, plotTitle):
    plt.style.use('dark_background')
    plt.figure(figsize = (12, 7))

    bars = plt.bar(
        data['Bucket Index'],
        data['Bucket Size'],
        color = '#FA1C95',
        edgecolor = '#FF1493',
        linewidth = 0.5,
        hatch = '/////'
    )

    plt.title(plotTitle, fontsize = 14, pad = 20, color = '#FA1C95')
    plt.xlabel('Bucket Index', fontsize = 12, color = '#FA1C95')
    plt.ylabel('Bucket Size', fontsize = 12, color = '#FA1C95')
    plt.xticks(rotation = 45, color = '#FA1C95')
    plt.yticks(rotation = 0,  color = '#FA1C95')
    plt.grid(axis = 'y', linestyle = '--', alpha = 0.7)

    plt.savefig(outputFileName, dpi=400, bbox_inches = 'tight')
    plt.close()

hashFunctionNames = ['adler32', 'crc32', 'fnv32', 'murmur3', 'pjw32', 'sdbm32']

for name in hashFunctionNames:
    data = readCsvData(f'data/{name}.csv')
    print(f"First lines from {name}.csv:")
    print(data.head())
    plotAndSave(data, f'img/{name}.png', name)