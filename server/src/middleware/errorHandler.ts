import type { NextFunction, Request, Response } from 'express';

type HttpError = {
  status?: number;
  statusCode?: number;
  message?: string;
};

export function errorHandler(err: unknown, req: Request, res: Response, next: NextFunction) {
  const e = err as HttpError;
  const status = e?.statusCode || e?.status || 500;
  const message = status >= 500 ? 'internal_error' : e?.message || 'error';

  res.status(status).json({ error: message });
}

