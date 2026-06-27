# Stock Anomaly Radar

Stock Anomaly Radar is a small full-stack app for watching market data and calling out unusual stock behavior. The idea is simple: choose the tickers you want to track, let the backend listen to live market updates for those symbols, and use the dashboard to see when something looks abnormal.

The React frontend has two main views. The tracker page has a dropdown of common tickers, stored locally in the frontend, so Firebase does not need to hold every possible stock option. When a ticker is added, it becomes a tracked ticker and gets saved through the proxy. The dashboard then shows the tracked tickers in the side nav and filters anomaly results by route, so `/dashboard/AAPL` focuses on AAPL.

The proxy is the middle layer between the browser, Firebase, and the C++ backend. It stores the tracked ticker collection in Firestore, forwards anomaly reads from the backend, and keeps the C++ subscription list in sync when tickers are added or removed.

The C++ backend connects to Alpaca's streaming market data. It subscribes to the currently tracked tickers, keeps rolling state for each symbol, and checks incoming trades, quotes, and bars for price, volume, and spread anomalies. When it detects something unusual, it exposes that anomaly data through the API so the dashboard can display the details.

At this stage the project is focused on the core loop: pick tickers, subscribe to live data, detect anomalies, and show them in a dashboard. The next natural step is making the anomaly display richer with charts, history, and better ranking so the most important signals are easier to spot.
