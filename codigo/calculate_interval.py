# %% New ipython cell ================
import pandas as pd
import numpy as np
import scipy.stats

# %% New ipython cell ================


def mean_confidence_interval(data, confidence=0.95):
    a = 1.0 * np.array(data)
    n = len(a)
    m, se = np.mean(a), scipy.stats.sem(a)
    h = se * scipy.stats.t.ppf((1 + confidence) / 2., n-1)
    return m, h

# %% New ipython cell ================


df = pd.read_csv("./data.csv", delimiter=',')

for column_name in df.columns:
    mh = mean_confidence_interval(df[column_name])
    print(f"para {column_name} temos media = {mh[0]} e intervalo = {mh[1]}")
