import cors from 'cors';
import express from 'express';
import helmet from 'helmet';
import anomaliesRouter from './routes/anomalies';

const app = express();

app.use(helmet());
app.use(cors());
app.use(express.json());

app.get('/health', (_req, res) => res.json({ ok: true }));
app.get('/', (_req, res) => res.status(200).json({ ok: true, route: 'root' }));

app.use('/api/anomalies', anomaliesRouter);

app.use((err: any, _req: any, res: any, _next: any) => {
  const status = err?.status ?? err?.response?.status ?? 500;
  const message = err?.message || err?.response?.data?.message || 'Internal Server Error';
  res.status(status).json({ ok: false, error: message });
});

export default app;
