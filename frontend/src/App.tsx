import { useState } from 'react';
import './App.css';
import TickerForm from './components/TickerForm';
import TickerList from './components/TickerList';

const footerLinks = [
  { label: 'How it works', href: '/how-it-works' },
  { label: 'FAQ', href: '/faq' },
  { label: 'Terms', href: '/terms' },
  { label: 'Privacy', href: '/privacy' },
];

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
    <main className="app-shell">
      <section className="tracker-panel">
        <h1>stock anomaly dashboard</h1>
        <p>choose tickers to watch for anomaly tracking.</p>
        <TickerForm onAddTicker={addTicker} />
        <p>{tickers.length} tickers tracked</p>
        <TickerList tickers={tickers} onRemoveTicker={removeTicker} />
      </section>
      <footer className="dashboard-footer" aria-label="Dashboard links">
        {footerLinks.map((link) => (
          <a key={link.href} href={link.href}>
            {link.label}
          </a>
        ))}
      </footer>
    </main>
  );
}

export default App;
