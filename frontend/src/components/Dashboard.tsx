import type { Anomaly } from '../types/anomaly';
import { useState, useEffect } from 'react';
import { useParams } from 'react-router-dom';
import DashboardTickerNav from './DashboardTickerNav';

type DashboardProps = {
  tickers: string[];
};

export default function Dashboard({ tickers }: DashboardProps) {
  const { ticker } = useParams();
  const [anomalies, setAnomalies] = useState<Anomaly[]>([]);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    let cancelled = false;

    const fetchAnomalies = async () => {
      const base = import.meta.env.VITE_API_BASE_URL || 'http://localhost:8080';

      try {
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
            console.log('non-critical error');
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
      }
    };

    fetchAnomalies();

    const intervalId = window.setInterval(fetchAnomalies, 3000);

    return () => {
      cancelled = true;
      window.clearInterval(intervalId);
    };
  }, [tickers]);

  return (
    <div className="dashboard-layout">
      <div className="dashboard-sidebar">
        <DashboardTickerNav tickers={tickers} selectedTicker={ticker} />
      </div>

      <div className="dashboard-content">
        <h1>{ticker}</h1>
        {error ? <p>{error}</p> : null}
        {!error ? (
          <>
            <p>{`total anomalies: ${anomalies.length}`}</p>
            <p>hei chinni my baby lovey pie</p>
          </>
        ) : null}
      </div>
    </div>
  );
}
