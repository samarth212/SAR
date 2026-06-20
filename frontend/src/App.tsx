import { useState } from 'react';
import { Link, Navigate, NavLink, Route, Routes } from 'react-router-dom';
import './App.css';
import Dashboard from './components/Dashboard';
import TickerForm from './components/TickerForm';
import TickerList from './components/TickerList';

function App() {
  const [tickers, setTickers] = useState<string[]>([]);

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

      <main className="app-shell">
        <Routes>
          <Route path="/" element={<Navigate to="/tracker" replace />} />
          <Route
            path="/tracker"
            element={
              <section className="tracker-panel">
                <h1>stock anomaly tracker</h1>
                <p>choose tickers to watch for anomaly tracking.</p>
                <TickerForm onAddTicker={addTicker} />
                <p>{tickers.length} tickers tracked</p>
                <TickerList tickers={tickers} onRemoveTicker={removeTicker} />
              </section>
            }
          />
          <Route
            path="/dashboard"
            element={
              <section className="tracker-panel">
                <h1>anomaly dashboard</h1>
                <Dashboard tickers={tickers} />
              </section>
            }
          />
        </Routes>
      </main>
    </>
  );
}

export default App;
