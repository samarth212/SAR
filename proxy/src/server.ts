import 'dotenv/config';
import cors from 'cors';
import express from 'express';
import { syncAnomalies } from './anomalies.js';
import { listTickers, trackTicker, untrackTicker, TickerError, type Ticker } from './tickers.js';
import { firebaseEnabled } from './firebase.js';

const app = express();
const port = Number(process.env.PORT ?? 3001);
const cppApiBaseUrl = (process.env.CPP_API_BASE_URL ?? 'http://localhost:8080').replace(/\/$/, '');

async function readBackendError(response: Response) {
  try {
    const body: unknown = await response.json();
    if (body && typeof body === 'object' && 'error' in body && typeof body.error === 'string') {
      return body.error;
    }
  } catch {
    return `backend returned ${response.status}`;
  }

  return `backend returned ${response.status}`;
}

async function syncCppTrackedTickers(tickers: Ticker[]) {
  const trackedSymbols = tickers.map((ticker) => ticker.symbol);
  const response = await fetch(`${cppApiBaseUrl}/api/tickers/tracked`, {
    method: 'PUT',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify(trackedSymbols),
  });

  if (!response.ok) {
    throw new Error(await readBackendError(response));
  }
}

function syncCppTrackedTickersBestEffort(tickers: Ticker[]) {
  syncCppTrackedTickers(tickers).catch((error: unknown) => {
    const message = error instanceof Error ? error.message : 'unknown error';
    console.warn(`failed to sync tracked tickers to C++ backend: ${message}`);
  });
}

app.use(cors());
app.use(express.json());

app.get('/', (_req, res) => {
  res.json({ ok: true, route: 'root' });
});

app.get('/health', (_req, res) => {
  res.json({ ok: true, firebaseEnabled });
});

app.get('/api/anomalies', async (_req, res, next) => {
  try {
    const response = await fetch(`${cppApiBaseUrl}/api/anomalies`);
    const body: unknown = await response.json();

    if (response.ok && Array.isArray(body)) {
      await syncAnomalies(body);
    }

    res.status(response.status).json(body);
  } catch (error) {
    next(error);
  }
});

app.get('/api/tickers', async (_req, res, next) => {
  try {
    const tickers = await listTickers();
    syncCppTrackedTickersBestEffort(tickers);
    res.json(tickers);
  } catch (error) {
    next(error);
  }
});

app.post('/api/tickers/:symbol/track', async (req, res, next) => {
  try {
    const name = typeof req.body?.name === 'string' ? req.body.name : undefined;
    const ticker = await trackTicker(req.params.symbol, name);
    const tickers = await listTickers();
    await syncCppTrackedTickers(tickers);
    res.json(ticker);
  } catch (error) {
    next(error);
  }
});

app.delete('/api/tickers/:symbol/track', async (req, res, next) => {
  try {
    const ticker = await untrackTicker(req.params.symbol);
    const tickers = await listTickers();
    await syncCppTrackedTickers(tickers);
    res.json(ticker);
  } catch (error) {
    next(error);
  }
});

app.use(
  (error: unknown, _req: express.Request, res: express.Response, _next: express.NextFunction) => {
    const message = error instanceof Error ? error.message : 'Internal Server Error';
    const status = error instanceof TickerError ? error.status : 500;

    res.status(status).json({ ok: false, error: message });
  },
);

app.listen(port, () => {
  console.log(`proxy listening on :${port}`);
});
