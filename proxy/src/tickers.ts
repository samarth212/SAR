import { FieldValue, type DocumentSnapshot } from 'firebase-admin/firestore';
import { db } from './firebase.js';

const normalizeTicker = (ticker: string) => ticker.trim().toUpperCase();
const TICKER_PATTERN = /^[A-Z][A-Z0-9.-]{0,9}$/;

export type Ticker = {
  symbol: string;
  name?: string;
};

export class TickerError extends Error {
  status: number;

  constructor(status: number, message: string) {
    super(message);
    this.status = status;
  }
}

const tickerFromDoc = (doc: DocumentSnapshot): Ticker | null => {
  const data = doc.data();
  if (!data) {
    return null;
  }

  const symbol =
    typeof data.symbol === 'string' ? normalizeTicker(data.symbol) : normalizeTicker(doc.id);

  if (!symbol || data.tracked === false) {
    return null;
  }

  const ticker: Ticker = { symbol };

  if (typeof data.name === 'string' && data.name.trim()) {
    ticker.name = data.name.trim();
  }

  return ticker;
};

export const listTickers = async (): Promise<Ticker[]> => {
  if (!db) {
    return [];
  }

  const snapshot = await db.collection('tickers').get();
  const tickers = snapshot.docs
    .map((doc) => tickerFromDoc(doc))
    .filter((ticker): ticker is Ticker => ticker !== null);

  return tickers.sort((a, b) => a.symbol.localeCompare(b.symbol));
};

export const trackTicker = async (symbol: string, name?: string): Promise<Ticker> => {
  if (!db) {
    throw new TickerError(503, 'Firebase is not configured');
  }

  const normalizedSymbol = normalizeTicker(symbol);
  if (!TICKER_PATTERN.test(normalizedSymbol)) {
    throw new TickerError(400, 'Invalid ticker');
  }

  const ref = db.collection('tickers').doc(normalizedSymbol);
  const trackedTicker: Ticker = {
    symbol: normalizedSymbol,
  };

  if (name?.trim()) {
    trackedTicker.name = name.trim();
  }

  await ref.set({
    ...trackedTicker,
    trackedAt: FieldValue.serverTimestamp(),
  });

  return trackedTicker;
};

export const untrackTicker = async (symbol: string): Promise<Ticker> => {
  if (!db) {
    throw new TickerError(503, 'Firebase is not configured');
  }

  const normalizedSymbol = normalizeTicker(symbol);
  if (!TICKER_PATTERN.test(normalizedSymbol)) {
    throw new TickerError(400, 'Invalid ticker');
  }

  await db.collection('tickers').doc(normalizedSymbol).delete();

  return { symbol: normalizedSymbol };
};
