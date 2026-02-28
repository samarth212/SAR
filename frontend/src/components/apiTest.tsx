import type { Anomaly } from './types/anomaly';
import { useState } from 'react';

type ApiTestProps = {
  label?: string;
  onClick?: () => void;
  disabled?: boolean;
};

export default function ApiTest({ label = 'Test API', onClick, disabled = false }: ApiTestProps) {
  const [anomalies, setAnomalies] = useState<Anomaly[]>([]);

  return (
    <button type="button" onClick={onClick} disabled={disabled}>
      {label}
    </button>
  );
}
