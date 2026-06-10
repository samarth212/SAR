import { useState, type FormEvent } from 'react';
import { isValidTicker, normalizeTicker } from '../utils/tickers';

type TickerFormProps = {
  onAddTicker: (ticker: string) => boolean;
};

export default function TickerForm({ onAddTicker }: TickerFormProps) {
  const [value, setValue] = useState('');
  const [error, setError] = useState<string | null>(null);

  const handleSubmit = (event: FormEvent<HTMLFormElement>) => {
    event.preventDefault();

    const ticker = normalizeTicker(value);
    if (!isValidTicker(ticker)) {
      setError('enter a valid ticker');
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
        <input
          id="ticker-input"
          name="ticker"
          placeholder="AAPL"
          value={value}
          onChange={(event) => {
            setValue(event.target.value);
            setError(null);
          }}
        />
        <button type="submit">add</button>
      </div>
      {error ? <p className="form-error">{error}</p> : null}
    </form>
  );
}
