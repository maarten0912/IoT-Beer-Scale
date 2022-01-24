# Scale Protocol

## Scale endpoints

| Request Type      | Path          | Data                              | Resulting action|
| :---------------- | :-----------: | :-------------------------------: | -----------: |
| Post              | /             | `delay <miliseconds>`             | Change the frequency in which the scale operates (both measuring and responding to HTTP requests). Currently a delay of 0 is advised. |
| Post              | /             | `api <host> <port>`               | Change the API host and port for the flatsite API. If the scale fails to connect to the new API, it will automatically fall back to the previous connection. Host can be max 60 characters.|
| Post              | /             | `wifi <ssid> <password>`          | Change the WiFi that the scale is using. After a failed log-in <br/> attempt it will automatically fall back to the previous WiFi configuration. Ssid and pass can both be max 60 characters.|
| Post              | /             | `kg <weight> <measures>`          | This will make the scale do `<measures>` measures and after that send the average combined with the weight to the flatsite API. 5 measures take about 2 minutes.|
| Post              | /             | `tare`                            | This will tare the scale. Whatever weight is on it at that moment will be count as 0. This takes about 20 seconds. |
| Post              | /             | `endpoint <measure> <calibrate>`  | This will set the endpoints for the two flatsite API calls. E.g. `endpoint /api/measure /api/calibrate`. Measure and calibrate can both be max 60 characters.|

## Flatsite API endpoints the scale will use
| Request Type      | Path          | Data                                                  | Resulting action      |
| :---------------- | :-----------: | :---------------------------------------------------: | -----------: |
| Post              | /measure      | `{"reading": <reading>}`                              | The Flatsite API will store this measurement | 
| Post              | /calibrate    | `{"averagereading": <reading>, "weight": <weight>}`   | The Flatsite API will store a calibration point and use it whenever it needs to compute the weight on the scale |


## Default settings
These values can currently not be changed:

Scale API:
- IP is dynamically obtained
- Port `6969`
