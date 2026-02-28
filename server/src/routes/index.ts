import { Router } from 'express';

export const router = Router();

router.get('/get-anomalies', async (req, res, next) => {
  const base = process.env.CPP_API_BASE_URL ?? 'http://localhost:8080';
  const url = `${base}/api/anomalies`;
  try {
    const resp = await fetch(url);
    const body = await resp.json();
    res.status(resp.status).json(body);
  } catch (err) {
    next(err);
  }
});

export default router;
