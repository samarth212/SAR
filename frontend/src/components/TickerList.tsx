type TickerListProps = {
  tickers: string[];
};

export default function TickerList({ tickers }: TickerListProps) {
  if (tickers.length === 0) {
    return <p className="empty-state">no tickers tracked yet</p>;
  }

  return (
    <ul className="ticker-list">
      {tickers.map((ticker) => (
        <li key={ticker}>{ticker}</li>
      ))}
    </ul>
  );
}
