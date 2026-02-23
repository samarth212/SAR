import { Router } from 'express';

export const router = Router();

const base = process.env.CPP_API_BASE_URL ?? 'http://localhost:8080';
const url = `${base}/api/anomalies`;

router.get('/get-anomalies', async (req, res, next) => {
  try {
    const resp = await fetch(url);
    const body = await resp.json();
    res.status(resp.status).json(body);
  } catch (err) {
    next(err);
  }
});

export default router;
