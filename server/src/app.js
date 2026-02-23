import express from 'express';
import cors from 'cors';
import morgan from 'morgan';

import { notFound } from './middleware/notFound.js';
import { errorHandler } from './middleware/errorHandler.js';

export function createApp() {
  const app = express();

  app.disable('x-powered-by');

  app.use(cors());
  app.use(express.json({ limit: '1mb' }));

  if (process.env.NODE_ENV !== 'test') {
    app.use(morgan('dev'));
  }

  // Intentionally no routes yet.

  app.use(notFound);
  app.use(errorHandler);

  return app;
}
