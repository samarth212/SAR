import Box from '@mui/material/Box';
import Button from '@mui/material/Button';
import Stack from '@mui/material/Stack';
import Typography from '@mui/material/Typography';

type DashboardTickerNavProps = {
  tickers: string[];
};

export default function DashboardTickerNav({ tickers }: DashboardTickerNavProps) {
  if (tickers.length === 0) {
    return <Typography color="text.secondary">no tickers tracked yet</Typography>;
  }

  return (
    <Box component="nav" aria-label="Dashboard ticker navigation">
      <Stack direction="row" spacing={1} useFlexGap sx={{ flexWrap: 'wrap' }}>
        {tickers.map((ticker) => (
          <Button key={ticker} type="button" variant="outlined" size="small">
            {ticker}
          </Button>
        ))}
      </Stack>
    </Box>
  );
}
