export function errorHandler(err, req, res, next) {
  const status = err?.statusCode || err?.status || 500;
  const message = status >= 500 ? 'internal_error' : err?.message || 'error';

  res.status(status).json({ error: message });
}
