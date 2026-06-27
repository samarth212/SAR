type TickerListProps = {
  tickers: string[];
  onRemoveTicker: (ticker: string) => void;
};

export default function TickerList({ tickers, onRemoveTicker }: TickerListProps) {
  if (tickers.length === 0) {
    return <p className="empty-state">No tickers tracked yet.</p>;
  }

  return (
    <ul className="ticker-list">
      {tickers.map((ticker) => (
        <li key={ticker}>
          <span>{ticker}</span>
          <button type="button" onClick={() => onRemoveTicker(ticker)}>
            Remove
          </button>
        </li>
      ))}
    </ul>
  );
}
