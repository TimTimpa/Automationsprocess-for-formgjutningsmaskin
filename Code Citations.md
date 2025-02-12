# Code Citations

## License: unknown
https://github.com/ZiTAL/arduino/tree/7e8cdbd8e7c662f706bad7d7e540d5c9fba79aa2/hardware/espressif/esp32/libraries/DNSServer/examples/CaptivePortal/CaptivePortal.ino

```
connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.
```


## License: unknown
https://github.com/PepLluis/edison_WebBlinking/tree/61003db48ac2efaa99edfc2f7699f746246fa634/edisonWebBlinking.ino

```
.available();
  if (client) {
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if
```


## License: BSD_2_Clause
https://github.com/abstractguy/TSO_project/tree/440c1a03318c122bf657ea6b5bcf140b43c9ab01/software/arduino/esp32/arduino-esp32/libraries/DNSServer/examples/CaptivePortal/CaptivePortal.ino

```
";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println
```


## License: unknown
https://github.com/VALERIO-CALCAGNI/Esp-32-Http-server/tree/df284686c5638cccdf3ec3c40bdd9e950a34d8c8/readme.md

```
String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (currentLine.length() == 0)
```


## License: unknown
https://github.com/Hikaribussei-lab/control_program/tree/ca7b36d00312366ce5daa37b0c7ad952780643b5/learning/codes_fromISSP/Tani_feedback_Arduino/PortentaPDFeedbackServer/PortentaPDFeedbackServer_m7/PortentaPDFeedbackServer_m7.ino

```
client = server.available();
  if (client) {
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n'
```

