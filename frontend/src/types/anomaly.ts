export type AnomalyType = 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8;

export type AnomalySource = 0 | 1 | 2;

export type AnomalyDirection = 0 | 1 | 2;

export type Anomaly = {
  type: AnomalyType;
  source: AnomalySource;
  direction: AnomalyDirection;
  symbol: string;
  timestamp: string;
  value: number;
  mean: number;
  stdev: number;
  zscore: number;
  lower: number;
  upper: number;
  k: number;
  note: string;
};
