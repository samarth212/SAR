import { Router } from 'express';

export const router = Router();

router.get('/get-anomaly', async (req, res, next) => {
  try {
  } catch (err) {
    next(err);
  }
});

export default router;
