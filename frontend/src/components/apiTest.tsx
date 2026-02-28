import type { Anomaly } from '../types/anomaly';
import { useState, useEffect } from 'react';

type ApiTestProps = {
  label?: string;
  onClick?: () => void;
  disabled?: boolean;
};

export default function ApiTest({ label = 'Test API', onClick, disabled = false }: ApiTestProps) {
  useEffect(() => {
    const fetchAnomalies = async () => {
      const base = import.meta.env.VITE_API_BASE_URL;
      const resp = await fetch(`${base}/api/anomalies`);
      const data = await resp.json();
      setAnomalies(data);
    };
    fetchAnomalies();
  }, []);
  const [anomalies, setAnomalies] = useState<Anomaly[]>([]);
  const loading = true;

  return (
    <button type="button" onClick={onClick} disabled={disabled}>
      {label}
    </button>
  );
}
