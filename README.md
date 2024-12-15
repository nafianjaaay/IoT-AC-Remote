# IoT AC Remote

### TRANSMITTER
Digunakan untuk mendapatkan data orang masuk dan keluar lalu datanya dikirim ke firebase RTDB.
Gunakan file `TX_Board_ESP32`. Wajib connect wifi stabil dan 2 sensor harus terpasang dengan baik.

### RECEIVER
Digunakan untuk menerima data dari firebase RTDB lalu mengirim IR Signal ke AC berdasarkan jumlah orang dalam ruangan. Gunakan file `RX_BOARD_ESP32_1IR` apabila IR yang sedang terpasang 1 (Default pada D32). Apabila sedang terpasang 2 IR gunakan file `RX_BOARD_ESP32_2IR`. Wajib connect wifi stabil dan sensor harus terpasang dengan baik. Apabila pakai program 1IR maka jumlah IR harus 1, apabila pakai program 2IR maka jumlah IR harus 2.

