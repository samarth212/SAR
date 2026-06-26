import { useEffect, useState } from 'react';
import { Link, Navigate, NavLink, Route, Routes, useLocation } from 'react-router-dom';
import './App.css';
import Dashboard from './components/Dashboard';
import TickerForm, { type TickerOption } from './components/TickerForm';
import TickerList from './components/TickerList';
import { TOP_TICKERS } from './data/topTickers';

const apiBaseUrl = import.meta.env.VITE_API_BASE_URL || 'http://localhost:3001';

function parseTickerOption(ticker: unknown): TickerOption | null {
  if (!ticker || typeof ticker !== 'object') {
    return null;
  }

  const symbol =
    'symbol' in ticker && typeof ticker.symbol === 'string'
      ? ticker.symbol.trim().toUpperCase()
      : '';

  if (!symbol) {
    return null;
  }

  const tickerOption: TickerOption = { symbol };

  if ('name' in ticker && typeof ticker.name === 'string' && ticker.name.trim()) {
    tickerOption.name = ticker.name.trim();
  }

  return tickerOption;
}

async function readApiError(response: Response, fallback: string) {
  try {
    const body: unknown = await response.json();
    if (body && typeof body === 'object' && 'error' in body && typeof body.error === 'string') {
      return body.error;
    }
  } catch {
    return fallback;
  }

  return fallback;
}

function App() {
  const location = useLocation();
  const isDashboardRoute = location.pathname.startsWith('/dashboard');
  const [tickers, setTickers] = useState<string[]>([]);
  const [tickerError, setTickerError] = useState<string | null>(null);

  useEffect(() => {
    let cancelled = false;

    const fetchTrackedTickers = async () => {
      try {
        setTickerError(null);

        const response = await fetch(`${apiBaseUrl}/api/tickers`);
        if (!response.ok) {
          throw new Error(`Failed to load tracked tickers (${response.status})`);
        }

        const data: unknown = await response.json();
        if (!Array.isArray(data)) {
          throw new Error('Invalid tickers response');
        }

        const nextTickerOptions = data
          .map((ticker) => parseTickerOption(ticker))
          .filter((ticker): ticker is TickerOption => ticker !== null);

        if (!cancelled) {
          setTickers(nextTickerOptions.map((ticker) => ticker.symbol));
        }
      } catch (error) {
        if (!cancelled) {
          const message = error instanceof Error ? error.message : 'Failed to load tracked tickers';
          setTickerError(message);
        }
      }
    };

    fetchTrackedTickers();

    return () => {
      cancelled = true;
    };
  }, []);

  const addTicker = async (ticker: string) => {
    if (tickers.includes(ticker)) {
      return false;
    }

    const tickerOption = TOP_TICKERS.find((option) => option.symbol === ticker);
    const response = await fetch(`${apiBaseUrl}/api/tickers/${encodeURIComponent(ticker)}/track`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({ name: tickerOption?.name }),
    });

    if (!response.ok) {
      const message = await readApiError(response, `Failed to track ${ticker}`);
      throw new Error(message);
    }

    const updatedTicker = parseTickerOption(await response.json());
    const symbol = updatedTicker?.symbol ?? ticker;

    setTickers((currentTickers) =>
      currentTickers.includes(symbol) ? currentTickers : [...currentTickers, symbol],
    );
    return true;
  };

  const removeTicker = async (ticker: string) => {
    try {
      const response = await fetch(
        `${apiBaseUrl}/api/tickers/${encodeURIComponent(ticker)}/track`,
        {
          method: 'DELETE',
        },
      );

      if (!response.ok) {
        const message = await readApiError(response, `Failed to remove ${ticker}`);
        throw new Error(message);
      }

      setTickers((currentTickers) =>
        currentTickers.filter((currentTicker) => currentTicker !== ticker),
      );
      setTickerError(null);
    } catch (error) {
      const message = error instanceof Error ? error.message : 'Failed to remove ticker';
      setTickerError(message);
    }
  };

  return (
    <>
      <header className="app-navbar">
        <div className="app-navbar-inner">
          <Link className="app-brand" to="/tracker">
            SAR
          </Link>

          <nav className="app-nav" aria-label="Main navigation">
            <NavLink to="/tracker">tracker</NavLink>
            <NavLink to="/dashboard">dashboard</NavLink>
          </nav>
        </div>
      </header>

      <main className={isDashboardRoute ? 'app-shell dashboard-shell' : 'app-shell'}>
        <Routes>
          <Route path="/" element={<Navigate to="/tracker" replace />} />
          <Route
            path="/tracker"
            element={
              <section className="tracker-panel">
                <h1>stock anomaly tracker</h1>
                <p>choose tickers to watch for anomaly tracking.</p>
                <TickerForm
                  onAddTicker={addTicker}
                  tickerError={tickerError}
                  tickerOptions={TOP_TICKERS}
                  trackedTickers={tickers}
                />
                <p>{tickers.length} tickers tracked</p>
                <TickerList tickers={tickers} onRemoveTicker={removeTicker} />
              </section>
            }
          />
          <Route path="/dashboard" element={<Dashboard tickers={tickers} />} />
          <Route path="/dashboard/:ticker" element={<Dashboard tickers={tickers} />} />
        </Routes>
      </main>
    </>
  );
}

export default App;
