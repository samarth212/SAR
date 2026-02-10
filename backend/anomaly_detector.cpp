#include "anomaly_detector.h"

/*

Maybe keep the last 20-50 data points so you can calculate averages.
Step 3: Calculate What's "Normal"
For each new piece of data, calculate:

Average price of recent trades
Average volume of recent trades
Standard deviation (how spread out the prices usually are)

Think of standard deviation like this: if stock prices are usually between $100-$102, that's low
deviation. If they bounce between $95-$110, that's high deviation. Step 4: Detect Anomalies Compare
the new data point to your calculated "normal": Example for a price surge:

If new price > (average price + 2×standard deviation), that's unusual!
Or simpler: if new price is 5% higher than the recent average

Example for volume surge:

If new volume is 3× the average volume, something's happening!

Step 5: Decide What to Do
When you detect an anomaly:

Print an alert to the console
Log it to a file
Maybe track how long the anomaly lasts


*/
