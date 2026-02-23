import cors from 'cors';
import express, { type Express } from 'express';
import morgan from 'morgan';

import { errorHandler } from './middleware/errorHandler.js';
import { notFound } from './middleware/notFound.js';

export function createApp(): Express {
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

