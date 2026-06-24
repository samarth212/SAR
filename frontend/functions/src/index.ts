import { initializeApp } from 'firebase-admin/app';
import { getFirestore, FieldValue } from 'firebase-admin/firestore';
import { onRequest } from 'firebase-functions/v2/https';
import { defineSecret } from 'firebase-functions/params';

initializeApp();

const db = getFirestore();
const ingestToken = defineSecret('INGEST_TOKEN');

export const ingestAnomaly = onRequest({ secrets: [ingestToken] }, async (req, res) => {
  if (req.method !== 'POST') {
    res.status(405).json({ error: 'POST only' });
    return;
  }

  if (req.get('x-ingest-token') !== ingestToken.value()) {
    res.status(401).json({ error: 'Unauthorized' });
    return;
  }

  const anomaly = req.body;
  if (!anomaly || typeof anomaly !== 'object' || Array.isArray(anomaly)) {
    res.status(400).json({ error: 'Expected anomaly JSON object' });
    return;
  }

  await db.collection('anomalies').add({
    ...anomaly,
    createdAt: FieldValue.serverTimestamp(),
  });

  res.status(201).json({ ok: true });
});
