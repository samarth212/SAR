import { Router } from 'express';

export const router = Router();

const url = `${process.env.CPP_API_BASE_URL}/api/anomalies`;

router.get('/get-anomaly', async (req, res, next) => {
  try {
    const data = await fetch(url, { method: 'GET' });
    console.log(data);
  } catch (err) {
    next(err);
  }
});

export default router;
