import { FieldValue } from 'firebase-admin/firestore';
import { db } from './firebase.js';

type Anomaly = {
  symbol?: string;
  timestamp?: string;
  type?: number;
  source?: number;
  direction?: number;
  value?: number;
  mean?: number;
  stdev?: number;
  zscore?: number;
  lower?: number;
  upper?: number;
  k?: number;
  note?: string;
};

const anomalyId = (anomaly: Anomaly) => {
  const symbol = anomaly.symbol ?? 'unknown';
  const timestamp = anomaly.timestamp ?? 'no-time';
  const type = anomaly.type ?? 'no-type';
  const source = anomaly.source ?? 'no-source';
  const direction = anomaly.direction ?? 'no-direction';

  return `${symbol}_${timestamp}_${type}_${source}_${direction}`.replace(/[^A-Za-z0-9_-]/g, '_');
};

export const syncAnomalies = async (anomalies: Anomaly[]) => {
  if (!db || anomalies.length === 0) {
    return;
  }

  const batch = db.batch();

  for (const anomaly of anomalies) {
    const ref = db.collection('anomalies').doc(anomalyId(anomaly));
    batch.set(
      ref,
      {
        ...anomaly,
        syncedAt: FieldValue.serverTimestamp(),
      },
      { merge: true },
    );
  }

  await batch.commit();
};
