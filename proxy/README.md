# SAR proxy

Minimal local proxy for the React app.

```
npm install
npm run dev
```

Defaults:

- proxy: `http://localhost:3001`
- C++ API: `http://localhost:8080`

## Firebase

To sync anomalies into Firestore, create `proxy/.env` from `.env.example`.

Get the service account values from Firebase Console:

Project settings -> Service accounts -> Generate new private key.

Set:

```bash
FIREBASE_SERVICE_ACCOUNT_PATH=/absolute/path/to/service-account.json
```

Do not commit the service account JSON file.
