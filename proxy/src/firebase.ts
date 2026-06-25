import { applicationDefault, cert, getApps, initializeApp, type ServiceAccount } from 'firebase-admin/app';
import { getFirestore } from 'firebase-admin/firestore';
import { readFileSync } from 'node:fs';

const projectId = process.env.FIREBASE_PROJECT_ID;
const clientEmail = process.env.FIREBASE_CLIENT_EMAIL;
const privateKey = process.env.FIREBASE_PRIVATE_KEY?.replace(/\\n/g, '\n');
const serviceAccountPath = process.env.FIREBASE_SERVICE_ACCOUNT_PATH;
const useApplicationDefault = Boolean(process.env.GOOGLE_APPLICATION_CREDENTIALS);

export const firebaseEnabled = Boolean(
  serviceAccountPath || useApplicationDefault || (projectId && clientEmail && privateKey),
);

if (firebaseEnabled && getApps().length === 0) {
  if (serviceAccountPath) {
    const serviceAccount = JSON.parse(readFileSync(serviceAccountPath, 'utf8')) as ServiceAccount;

    initializeApp({
      credential: cert(serviceAccount),
    });
  } else if (useApplicationDefault) {
    initializeApp({
      credential: applicationDefault(),
    });
  } else {
    initializeApp({
      credential: cert({
        projectId,
        clientEmail,
        privateKey,
      }),
    });
  }
}

export const db = firebaseEnabled ? getFirestore() : null;
