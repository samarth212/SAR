import 'dotenv/config';

import app from './app';
import { config } from './config/index';

app.listen(config.port, () => {
  console.log(`server listening on :${config.port}`);
});
