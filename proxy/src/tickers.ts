import { FieldValue } from 'firebase-admin/firestore';
import { db } from './firebase.js';

const normalizeTicker = (ticker: string) => ticker.trim().toUpperCase();

export const syncTickers = async (tickers: string[]) => {
  if (!db || tickers.length === 0) {
    return 0;
  }

  const uniqueTickers = [...new Set(tickers.map(normalizeTicker).filter(Boolean))];
  if (uniqueTickers.length === 0) {
    return 0;
  }

  const batch = db.batch();

  for (const ticker of uniqueTickers) {
    const ref = db.collection('tickers').doc(ticker);
    batch.set(
      ref,
      {
        symbol: ticker,
        syncedAt: FieldValue.serverTimestamp(),
      },
      { merge: true },
    );
  }

  await batch.commit();
  return uniqueTickers.length;
};
