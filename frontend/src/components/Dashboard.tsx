import type { Anomaly } from '../types/anomaly';
import { useState, useEffect } from 'react';
import { useParams } from 'react-router-dom';
import DashboardTickerNav from './DashboardTickerNav';

type DashboardProps = {
  tickers: string[];
};

const ANOMALY_TYPE_LABELS = [
  'Price',
  'Volume',
  'Spread',
  'Volatility',
  'Range',
  'Gap',
  'Liquidity',
  'Stale data',
  'Parse error',
] as const;

const ANOMALY_SOURCE_LABELS = ['Trade', 'Quote', 'Bar'] as const;
const ANOMALY_DIRECTION_LABELS = ['Up', 'Down', 'None'] as const;

function anomalyTypeLabel(type: Anomaly['type']) {
  return ANOMALY_TYPE_LABELS[type] ?? 'Unknown';
}

function anomalySourceLabel(source: Anomaly['source']) {
  return ANOMALY_SOURCE_LABELS[source] ?? 'Unknown';
}

function anomalyDirectionLabel(direction: Anomaly['direction']) {
  return ANOMALY_DIRECTION_LABELS[direction] ?? 'Unknown';
}

function formatNumber(value: number) {
  return new Intl.NumberFormat('en-US', {
    maximumFractionDigits: 4,
  }).format(value);
}

function formatTimestamp(timestamp: string) {
  if (!timestamp) {
    return 'unknown time';
  }

  const date = new Date(timestamp);
  if (Number.isNaN(date.getTime())) {
    return timestamp;
  }

  return new Intl.DateTimeFormat('en-US', {
    dateStyle: 'medium',
    timeStyle: 'medium',
  }).format(date);
}

export default function Dashboard({ tickers }: DashboardProps) {
  const { ticker } = useParams();
  const selectedTicker = ticker?.toUpperCase();
  const [anomalies, setAnomalies] = useState<Anomaly[]>([]);
  const [error, setError] = useState<string | null>(null);
  const visibleAnomalies = selectedTicker
    ? anomalies.filter((anomaly) => anomaly.symbol.toUpperCase() === selectedTicker)
    : anomalies;

  useEffect(() => {
    let cancelled = false;

    const fetchAnomalies = async () => {
      const base = import.meta.env.VITE_API_BASE_URL || 'http://localhost:3001';

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
  }, []);

  return (
    <div className="dashboard-layout">
      <div className="dashboard-sidebar">
        <DashboardTickerNav tickers={tickers} selectedTicker={selectedTicker} />
      </div>

      <div className="dashboard-content">
        <h1>{selectedTicker ?? 'dashboard'}</h1>
        {error ? <p>{error}</p> : null}
        {!error ? (
          <>
            <div className="dashboard-summary">
              <div>
                <span>anomalies</span>
                <strong>{visibleAnomalies.length}</strong>
              </div>
              <div>
                <span>tracked tickers</span>
                <strong>{tickers.length}</strong>
              </div>
            </div>

            {visibleAnomalies.length === 0 ? (
              <p className="empty-state">no anomalies found for this view</p>
            ) : (
              <div className="anomaly-list">
                {visibleAnomalies.map((anomaly, index) => (
                  <article
                    className="anomaly-card"
                    key={`${anomaly.symbol}-${anomaly.timestamp}-${anomaly.type}-${index}`}
                  >
                    <div className="anomaly-card-header">
                      <div>
                        <h2>{`${anomaly.symbol} ${anomalyTypeLabel(anomaly.type)}`}</h2>
                        <p>{formatTimestamp(anomaly.timestamp)}</p>
                      </div>
                      <span className={`direction-badge direction-${anomaly.direction}`}>
                        {anomalyDirectionLabel(anomaly.direction)}
                      </span>
                    </div>

                    <dl className="anomaly-metrics">
                      <div>
                        <dt>source</dt>
                        <dd>{anomalySourceLabel(anomaly.source)}</dd>
                      </div>
                      <div>
                        <dt>value</dt>
                        <dd>{formatNumber(anomaly.value)}</dd>
                      </div>
                      <div>
                        <dt>mean</dt>
                        <dd>{formatNumber(anomaly.mean)}</dd>
                      </div>
                      <div>
                        <dt>z-score</dt>
                        <dd>{formatNumber(anomaly.zscore)}</dd>
                      </div>
                      <div>
                        <dt>lower</dt>
                        <dd>{formatNumber(anomaly.lower)}</dd>
                      </div>
                      <div>
                        <dt>upper</dt>
                        <dd>{formatNumber(anomaly.upper)}</dd>
                      </div>
                    </dl>

                    {anomaly.note ? <p className="anomaly-note">{anomaly.note}</p> : null}
                  </article>
                ))}
              </div>
            )}
          </>
        ) : null}
      </div>
    </div>
  );
}
