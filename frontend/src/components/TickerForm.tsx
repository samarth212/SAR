import { useState, type FormEvent } from 'react';
import { isValidTicker, normalizeTicker } from '../utils/tickers';

export type TickerOption = {
  symbol: string;
  name?: string;
};

type TickerFormProps = {
  onAddTicker: (ticker: string) => Promise<boolean>;
  tickerError: string | null;
  tickerOptions: readonly TickerOption[];
  trackedTickers: string[];
};

export default function TickerForm({
  onAddTicker,
  tickerError,
  tickerOptions,
  trackedTickers,
}: TickerFormProps) {
  const [value, setValue] = useState('');
  const [error, setError] = useState<string | null>(null);
  const [submitting, setSubmitting] = useState(false);

  const handleSubmit = async (event: FormEvent<HTMLFormElement>) => {
    event.preventDefault();
    if (submitting) {
      return;
    }

    const ticker = normalizeTicker(value);
    if (!isValidTicker(ticker)) {
      setError('choose a valid ticker');
      return;
    }

    if (!tickerOptions.some((option) => option.symbol === ticker)) {
      setError('ticker is not available');
      return;
    }

    setSubmitting(true);

    try {
      const added = await onAddTicker(ticker);
      if (!added) {
        setError('ticker is already tracked');
        return;
      }

      setValue('');
      setError(null);
    } catch (error) {
      const message = error instanceof Error ? error.message : 'Failed to track ticker';
      setError(message);
    } finally {
      setSubmitting(false);
    }
  };

  return (
    <form className="ticker-form" onSubmit={handleSubmit}>
      <label htmlFor="ticker-input">ticker</label>
      <div className="ticker-form-row">
        <select
          id="ticker-input"
          name="ticker"
          disabled={submitting || tickerOptions.length === 0}
          value={value}
          onChange={(event) => {
            setValue(event.target.value);
            setError(null);
          }}
        >
          <option value="">choose a ticker</option>
          {tickerOptions.map((ticker) => (
            <option key={ticker.symbol} value={ticker.symbol}>
              {ticker.name ? `${ticker.symbol} - ${ticker.name}` : ticker.symbol}
              {trackedTickers.includes(ticker.symbol) ? ' (tracked)' : ''}
            </option>
          ))}
        </select>
        <button type="submit" disabled={submitting || tickerOptions.length === 0}>
          {submitting ? 'adding' : 'add'}
        </button>
      </div>
      {tickerError ? <p className="form-error">{tickerError}</p> : null}
      {error ? <p className="form-error">{error}</p> : null}
    </form>
  );
}
