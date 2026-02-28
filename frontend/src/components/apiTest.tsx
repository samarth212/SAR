import type { Anomaly } from '../types/anomaly';
import { useState, useEffect } from 'react';

type ApiTestProps = {
  label?: string;
  onClick?: () => void;
  disabled?: boolean;
};

export default function ApiTest({ label = 'Test API', onClick, disabled = false }: ApiTestProps) {
  const [anomalies, setAnomalies] = useState<Anomaly[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    let cancelled = false;

    const fetchAnomalies = async () => {
      const base = import.meta.env.VITE_API_BASE_URL;

      if (!base) {
        if (!cancelled) {
          setError('Missing VITE_API_BASE_URL');
          setAnomalies([]);
          setLoading(false);
        }
        return;
      }

      try {
        setLoading(true);
        setError(null);

        const resp = await fetch(`${base}/api/anomalies`);

        if (!resp.ok) {
          let message = `Request failed with status ${resp.status}`;

          try {
            const body = await resp.json();
            if (body && typeof body.error === 'string') {
              message = body.error;
            }
          } catch {
            // Ignore parse errors and keep the status-based message.
          }

          throw new Error(message);
        }

        const data: unknown = await resp.json();

        if (!Array.isArray(data)) {
          throw new Error('Invalid anomalies response');
        }

        if (!cancelled) {
          setAnomalies(data as Anomaly[]);
        }
      } catch (err) {
        if (!cancelled) {
          const message = err instanceof Error ? err.message : 'Failed to fetch anomalies';
          setError(message);
          setAnomalies([]);
        }
      } finally {
        if (!cancelled) {
          setLoading(false);
        }
      }
    };

    fetchAnomalies();

    return () => {
      cancelled = true;
    };
  }, []);

  return (
    <div>
      <button type="button" onClick={onClick} disabled={disabled || loading}>
        {label}
      </button>
      {error ? <p>{error}</p> : null}
      {!error ? <p>{anomalies.length} anomalies loaded</p> : null}
    </div>
  );
}
