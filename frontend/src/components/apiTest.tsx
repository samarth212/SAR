type ApiTestProps = {
  label?: string;
  onClick?: () => void;
  disabled?: boolean;
};

export default function ApiTest({
  label = 'Test API',
  onClick,
  disabled = false,
}: ApiTestProps) {
  return (
    <button type="button" onClick={onClick} disabled={disabled}>
      {label}
    </button>
  );
}
