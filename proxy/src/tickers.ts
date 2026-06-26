import { db } from './firebase.js';

const normalizeTicker = (ticker: string) => ticker.trim().toUpperCase();

export type Ticker = {
  symbol: string;
  name?: string;
};

export const listTickers = async () => {
  if (!db) {
    return [];
  }

  const snapshot = await db.collection('tickers').get();
  const tickers = snapshot.docs
    .map((doc) => {
      const data = doc.data();
      const symbol =
        typeof data.symbol === 'string' ? normalizeTicker(data.symbol) : normalizeTicker(doc.id);
      const enabled = typeof data.enabled === 'boolean' ? data.enabled : true;

      if (!symbol || !enabled) {
        return null;
      }

      const ticker: Ticker = { symbol };
      if (typeof data.name === 'string' && data.name.trim()) {
        ticker.name = data.name.trim();
      }

      return ticker;
    })
    .filter((ticker): ticker is Ticker => ticker !== null);

  return tickers.sort((a, b) => a.symbol.localeCompare(b.symbol));
};
