import { useEffect, useState } from 'react';
import { Link, Navigate, NavLink, Route, Routes, useLocation } from 'react-router-dom';
import './App.css';
import Dashboard from './components/Dashboard';
import TickerForm, { type TickerOption } from './components/TickerForm';
import TickerList from './components/TickerList';

const apiBaseUrl = import.meta.env.VITE_API_BASE_URL || 'http://localhost:3001';

function App() {
  const location = useLocation();
  const isDashboardRoute = location.pathname.startsWith('/dashboard');
  const [tickers, setTickers] = useState<string[]>([]);
  const [tickerOptions, setTickerOptions] = useState<TickerOption[]>([]);
  const [tickersLoading, setTickersLoading] = useState(true);
  const [tickerError, setTickerError] = useState<string | null>(null);

  useEffect(() => {
    let cancelled = false;

    const fetchTickers = async () => {
      try {
        setTickerError(null);

        const response = await fetch(`${apiBaseUrl}/api/tickers`);
        if (!response.ok) {
          throw new Error(`Failed to load tickers (${response.status})`);
        }

        const data: unknown = await response.json();
        if (!Array.isArray(data)) {
          throw new Error('Invalid tickers response');
        }

        const nextTickerOptions = data
          .map((ticker) => {
            if (!ticker || typeof ticker !== 'object') {
              return null;
            }

            const symbol = 'symbol' in ticker && typeof ticker.symbol === 'string'
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
          })
          .filter((ticker): ticker is TickerOption => ticker !== null);

        if (!cancelled) {
          setTickerOptions(nextTickerOptions);
        }
      } catch (error) {
        if (!cancelled) {
          const message = error instanceof Error ? error.message : 'Failed to load tickers';
          setTickerError(message);
          setTickerOptions([]);
        }
      } finally {
        if (!cancelled) {
          setTickersLoading(false);
        }
      }
    };

    fetchTickers();

    return () => {
      cancelled = true;
    };
  }, []);

  const addTicker = (ticker: string) => {
    if (tickers.includes(ticker)) {
      return false;
    }

    setTickers((currentTickers) => [...currentTickers, ticker]);
    return true;
  };

  const removeTicker = (ticker: string) => {
    setTickers((currentTickers) =>
      currentTickers.filter((currentTicker) => currentTicker !== ticker),
    );
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
                  tickerOptions={tickerOptions}
                  tickersLoading={tickersLoading}
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
