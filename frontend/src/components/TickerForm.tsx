import { useState, type FormEvent } from 'react';
import { isValidTicker, normalizeTicker } from '../utils/tickers';

export type TickerOption = {
  symbol: string;
  name?: string;
};

type TickerFormProps = {
  onAddTicker: (ticker: string) => boolean;
  tickerError: string | null;
  tickerOptions: TickerOption[];
  tickersLoading: boolean;
};

export default function TickerForm({
  onAddTicker,
  tickerError,
  tickerOptions,
  tickersLoading,
}: TickerFormProps) {
  const [value, setValue] = useState('');
  const [error, setError] = useState<string | null>(null);

  const handleSubmit = (event: FormEvent<HTMLFormElement>) => {
    event.preventDefault();

    const ticker = normalizeTicker(value);
    if (!isValidTicker(ticker)) {
      setError('choose a valid ticker');
      return;
    }

    if (!tickerOptions.some((option) => option.symbol === ticker)) {
      setError('ticker is not available');
      return;
    }

    const added = onAddTicker(ticker);
    if (!added) {
      setError('ticker is already tracked');
      return;
    }

    setValue('');
    setError(null);
  };

  return (
    <form className="ticker-form" onSubmit={handleSubmit}>
      <label htmlFor="ticker-input">ticker</label>
      <div className="ticker-form-row">
        <select
          id="ticker-input"
          name="ticker"
          disabled={tickersLoading || tickerOptions.length === 0}
          value={value}
          onChange={(event) => {
            setValue(event.target.value);
            setError(null);
          }}
        >
          <option value="">{tickersLoading ? 'loading tickers...' : 'choose a ticker'}</option>
          {tickerOptions.map((ticker) => (
            <option key={ticker.symbol} value={ticker.symbol}>
              {ticker.name ? `${ticker.symbol} - ${ticker.name}` : ticker.symbol}
            </option>
          ))}
        </select>
        <button type="submit" disabled={tickersLoading || tickerOptions.length === 0}>
          add
        </button>
      </div>
      {tickerError ? <p className="form-error">{tickerError}</p> : null}
      {error ? <p className="form-error">{error}</p> : null}
    </form>
  );
}
