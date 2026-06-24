import 'dotenv/config';
import cors from 'cors';
import express from 'express';

const app = express();
const port = Number(process.env.PORT ?? 3001);
const cppApiBaseUrl = (process.env.CPP_API_BASE_URL ?? 'http://localhost:8080').replace(/\/$/, '');

app.use(cors());
app.use(express.json());

app.get('/', (_req, res) => {
  res.json({ ok: true, route: 'root' });
});

app.get('/health', (_req, res) => {
  res.json({ ok: true });
});

app.get('/api/anomalies', async (_req, res, next) => {
  try {
    const response = await fetch(`${cppApiBaseUrl}/api/anomalies`);
    const body = await response.text();

    res
      .status(response.status)
      .type(response.headers.get('content-type') ?? 'application/json')
      .send(body);
  } catch (error) {
    next(error);
  }
});

app.listen(port, () => {
  console.log(`proxy listening on :${port}`);
});
