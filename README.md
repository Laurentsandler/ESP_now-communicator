# ESP-Now long range communicator

## Description

A long range text based communicator based on esp-now

## Motivation

My school banned phones, so I wanted to build a small communicator that me and my friend can use.

## Hardware Design

The custom PCB was designed using EasyEDA, and is 2 layers. The battery fits behind it, the case has locked in buttons.

EasyEDA Project Link: <EASYEDA_PROJECT_URL_PLACEHOLDER>

### PCB Layout & 3D Renders

<img width="688" height="866" alt="image" src="https://github.com/user-attachments/assets/22b9b112-5db4-44e2-ba2e-78cf0d271a31" />
<img width="688" height="866" alt="image" src="https://github.com/user-attachments/assets/1d688949-601a-41da-bb1c-fbb6943e919c" />
<img width="688" height="866" alt="image" src="https://github.com/user-attachments/assets/6ccecc7b-2ddf-4812-a724-e9ec44e84f59" />


### Case

<CASE_DESIGN_DESCRIPTION_PLACEHOLDER>

<img width="1123" height="818" alt="image" src="https://github.com/user-attachments/assets/2b9d1126-d969-438f-b241-2b3cb91c5160" />
<img width="933" height="818" alt="image" src="https://github.com/user-attachments/assets/63216ec6-f913-4616-b818-edefcb88487c" />
<img width="1236" height="818" alt="image" src="https://github.com/user-attachments/assets/8b31281d-81f9-4add-b4ce-76600ceae6f2" />


Onshape Project Link: [<ONSHAPE_PROJECT>](https://cad.onshape.com/documents/b60efe79f3e39b50ec4340e8/w/98b2614707c57877de69ff90/e/7286b2544836b97ccc1a2148?renderMode=0&uiState=69e0c1a965c4cbf28e8a9a3c)



## Features

- Sending text based data between devices
- Offline detection
- Secure transmition

## System Architecture

```text
Device 1 <----> Device 2
```

## Software Dependencies

Install these libraries:
- #include <Arduino.h>
- #include <WiFi.h>
- #include <esp_now.h>
- #include <Wire.h>
- #include <Adafruit_GFX.h>
- #include <Adafruit_SH110X.h>
  
## Configuration

### Module Setup
1. Connect battery to JST connector
3. Connect the antenna to the esp32 module, and mount it through the hole in the case.
4. Wire the Display: Connect the 1.3" OLED display using I2C (SDA to pin `8`, SCL to pin `9`, VCC to 3.3V, and GND to Ground).
5. Upload Firmware: Open the project in PlatformIO and flash the exact same codebase (`src/main.cpp`) to both ESP32-S3 boards.


## Usage

- Auto-Discovery: Wait up to 3 seconds. The devices will broadcast heartbeat PINGs and automatically lock onto each other's MAC addresses using the PAIRING_SECRET.
- Verify Status: Check the OLED screen; the top status indicator should automatically update from Status: OFFLINE to Status: ONLINE.
- Communicate: Use the 0-9 buttons to type messages (T9 multi-tap style), the bottom-left button (M) to backspace, and the bottom-right button (S) to send the message privately to the paired device


## Bill of Materials (BOM)

| Item | Description | Purpose | Qty | Unit Price | Link | Supplier |
|------|------------|--------|-----|------------|------|----------|
| JLCPCB PCB | Custom PCB | Main computer that interfaces with components | 1 | $64.43 | N/A | JLCPCB |
| 1.3" OLED I2C Display (128x64, White) | OLED Display Module | Display messages and content | 2 | $6.36 | [Link](https://fr.aliexpress.com/item/4000191763740.html?spm=a2g0o.productlist.main.20.5f6ef3kif3kiOm&algo_pvid=a36f4e10-d45d-431d-a09f-b34da730a144&algo_exp_id=a36f4e10-d45d-431d-a09f-b34da730a144-7&pdp_ext_f=%7B%22order%22%3A%22305%22%2C%22eval%22%3A%221%22%2C%22fromPage%22%3A%22search%22%7D&pdp_npi=6%40dis%21USD%212.78%213.03%21%21%212.78%213.03%21%40211b6a7a17751605234495294edd97%2112000042920644699%21sea%21FR%217032395364%21X%211%210%21n_tag%3A-29913%3Bd%3Adbb9d545%3Bm03_new_user%3A-29895&curPageLogUid=nOCwXdIm4cS9&utparam-url=scene%3Asearch%7Cquery_from%3A%7Cx_object_id%3A4000191763740%7C_p_origin_prod%3A) | AliExpress |
| 8dBi WiFi Antenna (2.4GHz) | External WiFi Antenna | Supports long-distance communication | 1 | $5.66 | [Link](https://fr.aliexpress.com/item/1005007449002305.html?spm=a2g0o.productlist.main.1.4320LctgLctg8b&algo_pvid=c4755105-49a2-489c-8ea5-e53ec9182a49&algo_exp_id=c4755105-49a2-489c-8ea5-e53ec9182a49-0&pdp_ext_f=%7B%22order%22%3A%224281%22%2C%22spu_best_type%22%3A%22price%22%2C%22eval%22%3A%221%22%2C%22fromPage%22%3A%22search%22%7D&pdp_npi=6%40dis%21USD%216.28%215.66%21%21%2142.98%2138.76%21%40211b612517751604415362446e6a3d%2112000042558597363%21sea%21FR%217032395364%21X%211%210%21n_tag%3A-29913%3Bd%3Adbb9d545%3Bm03_new_user%3A-29895%3BpisId%3A5000000203821659&curPageLogUid=TfBXPuhS8Fzy&utparam-url=scene%3Asearch%7Cquery_from%3A%7Cx_object_id%3A1005007449002305%7C_p_origin_prod%3A) | AliExpress |
| 103450 3.7V 2000mAh LiPo Battery | Lithium Polymer Battery | Main power source | 2 | $7.81 | [Link](https://fr.aliexpress.com/item/1005011659846396.html?mp=1&pdp_npi=6@dis!EUR!EUR%209.19!EUR%206.89!!EUR%206.75!!!@210384b917751603503898743ede8d!12000056172145258!ct!FR!7032395364!!1!0!&gatewayAdapt=glo2fra) | AliExpress |

## Total Cost
**$84.26**

## License

MIT License
