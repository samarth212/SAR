const TICKER_PATTERN = /^[A-Z][A-Z0-9.-]{0,9}$/;

export function normalizeTicker(value: string) {
  return value.trim().toUpperCase();
}

export function isValidTicker(value: string) {
  return TICKER_PATTERN.test(value);
}
