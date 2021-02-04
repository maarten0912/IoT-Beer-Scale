# Scale Protocol

## Scale endpoints

| Request Type      | Path          | Data                              | Resulting action|
| :---------------- | :-----------: | :-------------------------------: | -----------: |
| Post              | /             | `delay <miliseconds>`             | Change the frequency for the `measure` mode. |
| Post              | /             | `api <host> <port>`               | Change the API host and port for the flatsite API. Host can be max 40 characters.|
| Post              | /             | `wifi <ssid> <password>`          | Change the WiFi that the scale is using. After a failed logging in <br/> attempt it will automatically fall back to the previous WiFi configuration. Ssid and pass can both be max 30 characters.|
| Post              | /             | `kg <weight> <measures>`          | This will make the scale do `<measures>` measures and after that send the average combined with the weight to the flatsite API. 5 measures take about 2 minutes.|
| Post              | /             | `tare`                            | This will tare the scale. Whatever weight is on it at that moment will be count as 0. This takes about 20 seconds. |
| Post              | /             | `endpoint <measure> <calibrate>`  | This will set the endpoints for the two flatsite API calls. E.g. `endpoint /api/measure /api/calibrate`. Measure and calibrate can both be max 30 characters.|
## Flatsite API endpoints the scale will use
| Request Type      | Path          | Data                                                  | Resulting action      |
| :---------------- | :-----------: | :---------------------------------------------------: | -----------: |
| Post              | /measure      | `{"reading": <reading>}`                              | Set mode to measure. Scale will send a measure every `delay` times to flatsite API. | 
| Post              | /calibrate    | `{"averagereading": <reading>, "weight": <weight>}`   | Set mode to calibrate. Scale will wait for a `kg` request. |


## Default settings
These values can currently be changed using the protocol, but the values will reset after reboot:

WiFi:
- SSID `Rikkert`
- Password `hetwachtwoordismaarten`

Flatsite API:
- Host `maarten.student.utwente.nl`
- Port `8080`
- Measure endpoint `/measure`
- Calibrate endpoint `/calibrate`

Delay: `5000`ms

These values can currently not be changed:
Scale API:
- IP is dynamically obtained
- Port `6969`