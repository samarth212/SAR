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
        bgcolor: 'background.paper',
        borderRight: 1,
        borderColor: 'divider',
        flexShrink: 0,
        height: '100%',
        overflowY: 'auto',
        width: 180,
      }}
    >
      <Tabs
        orientation="vertical"
        variant="scrollable"
        value={selectedIndex === -1 ? false : selectedIndex}
        aria-label="Dashboard ticker navigation"
        role="navigation"
        sx={{
          minWidth: 180,
          '& .MuiTab-root': {
            alignItems: 'flex-start',
            minHeight: 48,
          },
        }}
      >
        {tickers.length === 0 ? (
          <Typography color="text.secondary" sx={{ px: 3, py: 2 }}>
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
