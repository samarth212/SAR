import 'dotenv/config';

import { createApp } from './app.js';

const port = Number(process.env.PORT || 3001);
const app = createApp();

app.listen(port, () => {
  // Keep logging simple; user will wire details later.
  console.log(`server listening on :${port}`);
});
