import 'dotenv/config';
import cors from 'cors';
import express from 'express';
import { syncAnomalies } from './anomalies.js';
import { listTickers } from './tickers.js';
import { firebaseEnabled } from './firebase.js';

const app = express();
const port = Number(process.env.PORT ?? 3001);
const cppApiBaseUrl = (process.env.CPP_API_BASE_URL ?? 'http://localhost:8080').replace(/\/$/, '');

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
    res.json(tickers);
  } catch (error) {
    next(error);
  }
});

app.use(
  (error: unknown, _req: express.Request, res: express.Response, _next: express.NextFunction) => {
    const message = error instanceof Error ? error.message : 'Internal Server Error';

    res.status(500).json({ ok: false, error: message });
  },
);

app.listen(port, () => {
  console.log(`proxy listening on :${port}`);
});
