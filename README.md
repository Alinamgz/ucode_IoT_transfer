# &ensp;Transfer
This challenge is made to get aqcuanted with wireless data transmission (RF) and basic encryption.

Each board generates its key pair (Private and Public keys).<br>
Tx waits for public key from Rx. After Rx's key received, User is asked to press button to send Tx's Public key to Rx.<br>
As key exchange is completed, Tx can read users input via UART, encrypt it and send via RF.<br>
RX receives encrypted msg, decrypts it and writes it via UART<br>
F.e. to send `Lorem Ipsum` placeholder type: `lorem` + `ENTER`. Or you can type whatever you want. To edit your input use `LEFT ARROW` btn. <br>

## &ensp; &ensp;  <b> How to run this project? </b>

<b> &ensp; Hardware </b>
 - 2 x [Board LAUNCHXL-CC1352R1](https://www.ti.com/tool/LAUNCHXL-CC1352R1#description)
 - 2 x Micro USB cable

<b> &ensp; Software </b>
 - [CodeComposerStudio](https://www.ti.com/tool/download/CCSTUDIO) (CCS)
- SimpleLink SDK for your given LaunchPad (CCS may recomend you to install one)
 - [UniFlash](https://www.ti.com/tool/download/UNIFLASH)

 ### &ensp; &ensp; Running this project
 - git clone
 - open project with CCS (NB! One folder per time i.e t00 only or t04 only)
 - select `[project_name]` in your project explorer
 - configure a project for a specific LaunchPad: `targetConfigs/CC1352R1F3.ccxml` -> `Target Configuration` -> `Texas Instruments XDS110 USB Debug Probe` -> select `Debug Probe Selection` - `Select by serial numberand` enter your `device ID`
    - `device ID` can be found via `ls /dev`, in <b>UniFlash</b> or at `ccs/ccs_base/common/uscif/xds110/xdsdfu`
 - press `hammer` icon to build this project
 - there are various ways to flash the board with built project:
    - select `[project_name]` in your project explorer and ...
   - ... press `curly brackets in a folder` icon
   </br>OR
   - ... press Run -> Load -> `[project_name]`
![screenshot](https://user-images.githubusercontent.com/54025456/109845882-43b68980-7c56-11eb-97dd-72f7ce694c9f.png)

   #### OR
   - Open <b>UniFlash</b> and follow [this video guide](https://www.youtube.com/watch?v=V3-xcRmu5S0&t=51s) for device autodetect, and [this guide](http://software-dl.ti.com/ccs/esd/uniflash/docs/v5_0/quick_start_guide/uniflash_quick_start_guide.html) to flash the device.

## &ensp;  What to `.gitignore`?
Which CCS project files should be checkd in, and which should be `.gitignore`d? [Read here](https://software-dl.ti.com/ccs/esd/documents/sdto_ccs_source-control.html)
