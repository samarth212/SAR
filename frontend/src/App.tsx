import { useState } from 'react';
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

  return (
    <main className="app-shell">
      <section className="tracker-panel">
        <h1>stock anomaly dashboard</h1>
        <p>choose tickers to watch for anomaly tracking.</p>
        <TickerForm onAddTicker={addTicker} />
        <p>{tickers.length} tickers tracked</p>
        <TickerList tickers={tickers} />
      </section>
    </main>
  );
}

export default App;
