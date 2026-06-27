import Tab from '@mui/material/Tab';
import Box from '@mui/material/Box';
import Tabs from '@mui/material/Tabs';
import Typography from '@mui/material/Typography';
import { NavLink } from 'react-router-dom';

type DashboardTickerNavProps = {
  tickers: string[];
  selectedTicker?: string;
};

function a11yProps(ticker: string) {
  return {
    id: `dashboard-tab-${ticker}`,
    'aria-controls': `dashboard-panel-${ticker}`,
  };
}

function tickerPath(ticker: string) {
  return `/dashboard/${encodeURIComponent(ticker)}`;
}

export default function DashboardTickerNav({ tickers, selectedTicker }: DashboardTickerNavProps) {
  const selectedIndex = tickers.findIndex((ticker) => ticker === selectedTicker);

  return (
    <Box
      component="aside"
      sx={{
        bgcolor: 'transparent',
        flexShrink: 0,
        height: '100%',
        overflowY: 'auto',
        px: 2,
        py: 3,
        width: 280,
      }}
    >
      <Tabs
        orientation="vertical"
        variant="scrollable"
        value={selectedIndex === -1 ? false : selectedIndex}
        aria-label="Dashboard ticker navigation"
        role="navigation"
        sx={{
          minWidth: 240,
          '& .MuiTabs-indicator': {
            bgcolor: '#22d3ee',
            borderRadius: 999,
            boxShadow: '0 0 18px rgba(34, 211, 238, 0.55)',
            width: 3,
          },
          '& .MuiTab-root': {
            alignItems: 'flex-start',
            borderRadius: 2,
            color: '#93a8bb',
            fontWeight: 700,
            minHeight: 48,
            mb: 0.75,
            px: 2,
            textTransform: 'none',
            transition: 'background 160ms ease, color 160ms ease',
            '&:hover': {
              bgcolor: 'rgba(34, 211, 238, 0.08)',
              color: '#edf7ff',
            },
            '&.Mui-selected': {
              bgcolor: 'rgba(34, 211, 238, 0.12)',
              color: '#edf7ff',
            },
          },
        }}
      >
        {tickers.length === 0 ? (
          <Typography sx={{ color: '#93a8bb', px: 2, py: 2 }}>
            no tickers tracked yet
          </Typography>
        ) : (
          tickers.map((ticker) => (
            <Tab
              key={ticker}
              component={NavLink}
              label={ticker}
              to={tickerPath(ticker)}
              {...a11yProps(ticker)}
            />
          ))
        )}
      </Tabs>
    </Box>
  );
}
