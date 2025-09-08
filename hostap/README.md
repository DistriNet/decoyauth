# Integration in Wi-Fi client & deamon

The repository contains a proof-of-concept integration of our DecoyAuth protocol into Linux's open-source Wi-Fi client, i.e., `wpa_supplicant`. Note that this is currently only a proof-of-concept, we are still analyzing and verifying security properties, and seeing where the protocol can be optimized. _Do not use this code in production._ **Help on analyzing the security of this design, or other feedback, is welcome!**


## 1. Compilation

First compile the `wpa_supplicant` code for the client:

```
cd wpa_supplicant
cp defconfig .config
make -j 4
cd ..
```

Then you can compile the `hostapd` code for the access point:

```
cd hostapd
cp defconfig .config
make -j 4
cd ..
```


## 2. Starting the client

After running the steps in [Compilation](#1-Compilation) you can start `wpa_supplicant` (the client) as follows:

```
cd wpa_supplicant
sudo ./wpa_supplicant -D nl80211 -c decoyauth-client.conf -i wlan0
```

To run our proof-of-concept using a simulated Wi-Fi drivers, meaning without needing a physical Wi-Fi dongle or network card, you can load the `mac80211_hwsim` driver. This allows you to test our implementation in a virtual environment and while avoiding packet loss or othe physical-layer constraints:

```
# This creates 4 simulated Wi-Fi interfaces
sudo modprobe mac80211_hwsim radios=4
```

An example client configuration file `decoyauth-client.conf` is as follows:

```
network={
    ssid="testnetwork"
    psk="PASSWORD_1"
    key_mgmt=SAE
    ieee80211w=2

    proto=RSN
    pairwise=CCMP
    group=CCMP
}
```


## 3. Starting the access point

After running the steps in [Compilation](#1-Compilation) you can start `hostapd` (the access point) as follows:

```
cd hostapd
sudo ./hostapd -D nl80211 -c decoyauth-client.conf -i wlan0
```

Note that if the client is using simulated Wi-Fi drivers, then hostapd must also use simulated ones, otherwise they will not be able to communicate.

An example hostapd configuration file `decoyauth-ap.conf` is as follows:

```
interface=wlan1
ssid=testnetwork

hw_mode=g
channel=1

wpa=2
wpa_passphrase=decoyauth
wpa_key_mgmt=SAE
rsn_pairwise=CCMP
ieee80211w=2
```

The actual supported passwords are currently initialized in the source code. To change them, you have to modify the function `sae_ap_prepare_commit` in `src/common/sae.c` and recompile hostapd.



## 4. Code integrations

Our changes were made on top of hostap commit [795075444fe847e11a](https://git.w1.fi/cgit/hostap/commit/?id=795075444fe847e11a04de541c95613335aad7a5). To see the precise difference that have been added to hostap, you can execute the following:

```
# Grab the original hostap code
git clone git://w1.fi/hostap.git hostap-original
cd hostap-original
git checkout 795075444fe847e11a04de541c95613335aad7a5
cd ..

# Show the difference between our code, where the second directory
# in this command is the hostap directory in this project
diff -ruN --exclude='.*' --exclude='build' hostap-original/ hostap/ | grep -v '^Binary files '
```

To integrate our extension, we added separate functions specific for the Access Point (AP) to `sae.c`. The client calls the original (updated) functions in `sae.c` while we modified `ap/ieee802_11.c` to call the AP-specific new functions.

Currently, the code supports usage of 16 simultaneous (decoy) passwords at the AP side, and these passwords are configured in `src/common/sae.c` in the function `sae_ap_prepare_commit`. The password used by the client can be configured through normal means, see the section [Starting the client](#2-Starting-the-client).

Some of the core files that have been modified are, and their correspondence the [white paper](../docs/whitepaper.pdf) are as follows:

- `ap/ieee802_11.c`: replaced with AP-specific functions to differentiate between client and AP handshake code.

- `common/encode.c`: corresponds to Algorithm 1, 2, and 3.

- `common/weaver.c`: corresponds to the _encode_ and _decode_ function in Figure 1 and Section 3.1.

- `common/sae.c`: contains some of the main changes to make hostap implement the DecoyAuth protocol.

- `crypto/crypto_openssl.c`: implements some of the core crypto functions that we added and utilize.


## 5. Example connection

The parameters `-dd -K` can be given to both `wpa_supplicant` and `hostapd` to output detailed debugging statements. Here `-dd` output detailed info and `-K` ensures that the values of secret keys are also shown. Below, you can see relevant output of this. Additionally, we provide the [network traffic .pcap](example.pcapng) that corresponds to this debug output.

### 5.1. Example `wpa_supplicant` output

The full debug output is available in [wpa_supplicant.log](wpa_supplicant.log). Relevant output related to decoyauth is:

```
[mathy@zbook-mathy wpa_supplicant]$ sudo ./wpa_supplicant -D nl80211 -i wlan2 -c decoyauth-client.conf -dd -K
ssid - hexdump_ascii(len=11):
     74 65 73 74 6e 65 74 77 6f 72 6b                  testnetwork     
PSK (ASCII passphrase) - hexdump_ascii(len=10):
     50 41 53 53 57 4f 52 44 5f 31                     PASSWORD_1      

wlan2: Event SCAN_RESULTS (3) received
wlan2:    selected BSS 02:00:00:00:00:00 ssid='testnetwork'
wlan2: Considering connect request: reassociate: 0  selected: 02:00:00:00:00:00  bssid: 00:00:00:00:00:00  pending: 00:00:00:00:00:00  wpa_state: SCANNING  ssid=0x60650e181040  current_ssid=(nil)
wlan2: SME: Selected SAE group 19
SAE: password - hexdump_ascii(len=10):
     50 41 53 53 57 4f 52 44 5f 31                     PASSWORD_1      
SAE: PWE derivation - addr1=02:00:00:00:01:00 addr2=02:00:00:00:00:00
SAE: PWE - hexdump(len=64): 57 92 ec 3a 2a 62 d0 6d d3 34 96 60 54 98 ef 88 a5 07 25 b9 8d a8 3e b3 a8 a1 f3 90 d5 42 fe 00 a9 a4 1d a4 5b 99 fb 61 1f db ab f4 6d 1a e3 36 6f 68 d7 f0 05 95 26 2a f0 07 d6 93 64 07 c4 a7

SAE: own commit-scalar - hexdump(len=32): 12 34 e5 b7 21 42 af 21 f8 08 11 8a d2 80 e3 4d 0c 83 79 87 c8 86 e4 e1 f6 27 97 d4 68 d8 97 79
SAE: own commit-element(x) - hexdump(len=32): 81 dc 45 f3 3a dc 67 e5 98 96 20 d0 f1 5f 48 62 78 d2 29 08 35 4b 68 ee b2 55 39 99 71 91 03 7d
SAE: own commit-element(y) - hexdump(len=32): f2 5a 78 c3 45 c3 63 4c d4 64 d3 c1 20 c9 02 b7 6a 26 0d 47 b0 27 3f d8 4a 1b 7b ad fc bc fe dc

wlan2: SME: Authentication response: peer=02:00:00:00:00:00 auth_type=3 auth_transaction=1 status_code=0
SME: USING PASSWORD: PASSWORD_1

SAE: Peer commit-scalar - hexdump(len=32): df 3c 95 c7 2c e8 b1 07 a4 7d e4 32 31 87 91 28 ef 44 8b 18 ce d8 d0 07 5b 41 5e 00 d8 d1 28 bd
SAE: num_passwords: 16
SAE: u_coefficients[i] - hexdump(len=32): 7b 82 c8 14 ec da af 50 c8 e8 26 26 9a 48 dd f3 50 67 90 cc 8f 54 3d 87 84 e8 cd 80 b6 99 1a bd
SAE: u_coefficients[i] - hexdump(len=32): 62 10 18 80 10 8f 37 db 19 fa 1e f1 5a fa 6e 9e ec be 52 7c 0c f5 b4 24 bc 09 fc aa b0 87 cd 1b

SAE: u_coefficients[i] - hexdump(len=32): e2 91 dd 8f b6 40 8e 17 09 04 f2 cc bf 8d 80 33 5a 06 42 16 5a c3 cf ee 20 7a ae ee 83 bf 41 46
SAE: v_coefficients[i] - hexdump(len=32): 4c 0b da a9 4f ef 94 06 f8 2a f8 f4 c9 7b a1 b0 4d 62 52 f2 d9 1a 99 65 bb 40 ef 6d c6 10 7f e2
SAE: v_coefficients[i] - hexdump(len=32): b5 1d 17 fb e1 fe 4a 0a 67 61 97 2a 82 58 5e 5d 64 16 10 df 87 df 45 9e 53 ec eb a9 32 9f a8 d8

SAE: v_coefficients[i] - hexdump(len=32): fd 1a d2 10 e5 13 01 c9 ac 63 53 ad c0 9e e6 aa e8 8b a1 57 8f 58 f6 ff 37 28 9e b7 3b 72 5f c2

SAE: hash - hexdump(len=32): 81 10 f5 c7 b9 4b 41 a5 92 0c fa 58 91 f0 87 be e9 a2 26 f2 e2 70 03 d4 4a 60 fd 77 e4 5a 44 4c
SAE: encoded_point[0] - hexdump(len=32): 50 f2 0f cd 48 48 73 53 08 56 3f d6 c4 c0 b6 4e 65 db d3 34 ab ef 0f 76 6b c6 e7 7d c3 eb 41 cb
SAE: encoded_point[1] - hexdump(len=32): 68 bd 62 33 01 4b e8 f0 9c d9 27 d1 9a 7d 0a cd 79 d5 22 7b c0 25 02 f3 3c 92 0c a9 f7 fd ee 49
SAE: k - hexdump(len=32): 5d 9e f4 73 af c7 ae 40 af 94 6c b7 a2 aa b6 fd a8 77 6d 21 fe 1b 67 5a 39 8c cb 21 48 17 80 1c
SAE: Derive keys - H2E=0 AKMP=0x400 = 000fac08 (SAE)
SAE: salt for keyseed derivation - hexdump(len=32): 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
SAE: keyseed - hexdump(len=32): 46 28 bd 0a 5b 04 4c 1d f6 d9 2e ab da 5b 66 ac d8 e7 a5 57 7e 0c 9a 6f 88 ea 0d a8 6d 84 f4 85
SAE: PMKID - hexdump(len=16): f1 71 7b 7e 4e 2b 60 29 9c 85 f5 bd 04 08 74 75
SAE: KCK - hexdump(len=32): c9 59 cb f3 0c 2f 9e 32 d5 93 3f 7b 1f f0 47 97 5a 48 b7 01 85 c6 c0 80 d3 6c 98 11 d7 fe 8b f2
SAE: PMK - hexdump(len=32): e9 83 a4 6b 36 07 4d 96 a1 a7 5c 09 da c4 21 d1 db 97 27 28 eb 0d 9d 0c 10 c0 74 e5 7c 30 3f 80

wlan2: SME: Trying to authenticate with 02:00:00:00:00:00 (SSID='testnetwork' freq=2412 MHz)
wlan2: State: AUTHENTICATING -> AUTHENTICATING
nl80211: Authentication request send successfully

wlan2: Event AUTH (10) received
wlan2: SME: Authentication response: peer=02:00:00:00:00:00 auth_type=3 auth_transaction=2 status_code=0
SME: Authentication response IEs - hexdump(len=34): 01 00 46 e8 c4 45 ab a1 63 1a 32 6f 8b 42 b4 93 d5 eb 3f b8 26 f6 0c b6 c4 b2 e1 20 6e 1c 2a 40 02 50
wlan2: SME SAE confirm
SAE: peer-send-confirm 1
SME: SAE completed - setting PMK for 4-way handshake
WPA: Set PMK based on external data - hexdump(len=32): e9 83 a4 6b 36 07 4d 96 a1 a7 5c 09 da c4 21 d1 db 97 27 28 eb 0d 9d 0c 10 c0 74 e5 7c 30 3f 80
wlan2: Trying to associate with 02:00:00:00:00:00 (SSID='testnetwork' freq=2412 MHz)
wlan2: State: AUTHENTICATING -> ASSOCIATING
nl80211: Association request send successfully

nl80211: Drv Event 38 (NL80211_CMD_ASSOCIATE) received for wlan2
wlan2: State: ASSOCIATING -> ASSOCIATED

wlan2: WPA: RX message 1 of 4-Way Handshake from 02:00:00:00:00:00 (ver=0)
wlan2: WPA: Sending EAPOL-Key 2/4
wlan2: RSN: RX message 3 of 4-Way Handshake from 02:00:00:00:00:00 (ver=0)
wlan2: WPA: Sending EAPOL-Key 4/4

EAPOL: SUPP_PAE entering state AUTHENTICATED
EAPOL authentication completed - result=SUCCESS
wlan2: WPA: Key negotiation completed with 02:00:00:00:00:00 [PTK=CCMP GTK=CCMP]
[mathy@zbook-mathy wpa_supplicant]$
```


### 5.2. Example `hostapd` output

The full debug output is available in [hostapd.log](hostapd.log). Relevant output related to decoyauth is:

```
[mathy@zbook-mathy hostapd]$ sudo ./hostapd decoyauth-ap.conf -dd -K
Using interface wlan1 with hwaddr 02:00:00:00:00:00 and ssid "testnetwork"
wlan1: Event RX_MGMT (18) received
  New STA
ap_sta_add: register ap_handle_timer timeout for 02:00:00:00:01:00 (300 seconds - ap_max_inactivity)
SAE: State Nothing -> Nothing for peer 02:00:00:00:01:00 (Init)
wlan1: STA 02:00:00:00:01:00 IEEE 802.11: start SAE authentication (RX commit, status=0 (SUCCESS))
SAE: Selecting supported ECC group 19
SAE: Peer commit-scalar - hexdump(len=32): 12 34 e5 b7 21 42 af 21 f8 08 11 8a d2 80 e3 4d 0c 83 79 87 c8 86 e4 e1 f6 27 97 d4 68 d8 97 79
SAE: Peer commit-element(x) - hexdump(len=32): 81 dc 45 f3 3a dc 67 e5 98 96 20 d0 f1 5f 48 62 78 d2 29 08 35 4b 68 ee b2 55 39 99 71 91 03 7d
SAE: Peer commit-element(y) - hexdump(len=32): f2 5a 78 c3 45 c3 63 4c d4 64 d3 c1 20 c9 02 b7 6a 26 0d 47 b0 27 3f d8 4a 1b 7b ad fc bc fe dc
SAE: Peer 02:00:00:00:01:00 state=Nothing auth_trans=1

SAE: password - hexdump_ascii(len=10):
     50 41 53 53 57 4f 52 44 5f 30                     PASSWORD_0      
SAE: PWE derivation - addr1=02:00:00:00:00:00 addr2=02:00:00:00:01:00
SAE: PWE - hexdump(len=64): a5 d3 29 4e ac 0f 9a 1e bb 4a a4 3b c7 1e 14 49 d1 d5 64 93 56 05 57 20 87 64 af af 6b f7 81 67 6e f2 08 cd 5b d0 df de b0 c0 51 a7 e7 33 d4 b8 99 65 4d 8d c8 db 2f 91 3b 2c 9e 2c 1d a5 bd e6

SAE: password - hexdump_ascii(len=10):
     50 41 53 53 57 4f 52 44 5f 31                     PASSWORD_1      
SAE: PWE derivation - addr1=02:00:00:00:00:00 addr2=02:00:00:00:01:00
SAE: PWE - hexdump(len=64): a5 d3 29 4e ac 0f 9a 1e bb 4a a4 3b c7 1e 14 49 d1 d5 64 93 56 05 57 20 87 64 af af 6b f7 81 67 6e f2 08 cd 5b d0 df de b0 c0 51 a7 e7 33 d4 b8 99 65 4d 8d c8 db 2f 91 3b 2c 9e 2c 1d a5 bd e6

SAE: password - hexdump_ascii(len=11):
     50 41 53 53 57 4f 52 44 5f 31 35                  PASSWORD_15     
SAE: PWE derivation - addr1=02:00:00:00:00:00 addr2=02:00:00:00:01:00
SAE: PWE - hexdump(len=64): d9 e4 af 48 e4 14 19 66 00 ae a9 89 d8 0e 6e cc b3 5e 2c 0f 4b 8a af 23 55 38 85 01 18 14 05 51 15 b1 63 83 ca 67 89 6f 27 bc b1 55 96 44 dd b1 8e 52 d7 a4 25 6e bc 52 da 08 1b 53 6e 95 e0 3f

SAE: own commit-scalar - hexdump(len=32): df 3c 95 c7 2c e8 b1 07 a4 7d e4 32 31 87 91 28 ef 44 8b 18 ce d8 d0 07 5b 41 5e 00 d8 d1 28 bd
SAE: Number of passwords - hexdump(len=4): 10 00 00 00

SAE: State Nothing -> Committed for peer 02:00:00:00:01:00 (Sent Commit)
SAE: k - hexdump(len=32): 36 dc 63 b9 15 b8 44 5a 6d b6 4d ac 53 97 c1 b8 33 01 5e b9 c2 ae d5 18 ec 85 59 2b 79 a0 27 1f

SAE: Deriving keys for index 0
SAE: KCK - hexdump(len=32): 0b dc 70 fe 0d b9 11 e8 82 fd c5 02 c1 0f 2e c0 33 8c 4e 89 93 58 af 3a 9b ae 28 d9 68 7c d9 cc
SAE: PMK - hexdump(len=32): da b1 97 ed 01 a6 5f ff 38 88 fe ab e2 1a 09 38 92 fb 5a 5a 38 81 bd 55 5b 09 76 00 cf 13 b5 da
SAE: k - hexdump(len=32): 5d 9e f4 73 af c7 ae 40 af 94 6c b7 a2 aa b6 fd a8 77 6d 21 fe 1b 67 5a 39 8c cb 21 48 17 80 1c

SAE: Deriving keys for index 1
SAE: KCK - hexdump(len=32): c9 59 cb f3 0c 2f 9e 32 d5 93 3f 7b 1f f0 47 97 5a 48 b7 01 85 c6 c0 80 d3 6c 98 11 d7 fe 8b f2
SAE: PMK - hexdump(len=32): e9 83 a4 6b 36 07 4d 96 a1 a7 5c 09 da c4 21 d1 db 97 27 28 eb 0d 9d 0c 10 c0 74 e5 7c 30 3f 80
SAE: k - hexdump(len=32): 68 09 1e a5 c5 f9 91 a9 90 e2 74 10 2a 4a bb 27 1e 6d d6 0b 6a 48 d4 ae 12 57 3b f5 da 64 cd 1e

SAE: Deriving keys for index 15
SAE: KCK - hexdump(len=32): 17 76 ab d5 49 6a e3 de 37 e3 ae 5e 60 78 0b 1d 81 d0 8f ab 05 46 ac d6 a1 71 f4 e3 b1 38 38 84
SAE: PMK - hexdump(len=32): 67 27 ad fa 01 70 6e 21 5c f5 ca 8d c9 91 70 a8 0f 46 ce 3e c2 89 39 65 7b d0 1a ef 65 5b fd c9

wlan1: Event RX_MGMT (18) received
authentication: STA=02:00:00:00:01:00 auth_alg=3 auth_transaction=2 status_code=0 wep=0 seq_ctrl=0xe90
wlan1: STA 02:00:00:00:01:00 IEEE 802.11: SAE authentication (RX confirm, status=0 (SUCCESS))
SAE: peer-send-confirm 1
SAE: peer-send-confirm 1
SAE: Confirmation successful
SAE: Peer 02:00:00:00:01:00 state=Committed auth_trans=2
authentication reply: STA=02:00:00:00:01:00 auth_alg=3 auth_transaction=2 resp=0 (IE len=34) (dbg=sae-send-confirm)
SAE: State Committed -> Confirmed for peer 02:00:00:00:01:00 (Sent Confirm)
SAE: Peer 02:00:00:00:01:00 state=Confirmed auth_trans=2
SAE: State Confirmed -> Accepted for peer 02:00:00:00:01:00 (Accept Confirm)
RSN: Cache PMK from SAE - hexdump(len=32): e9 83 a4 6b 36 07 4d 96 a1 a7 5c 09 da c4 21 d1 db 97 27 28 eb 0d 9d 0c 10 c0 74 e5 7c 30 3f 80
RSN: added PMKSA cache entry for 02:00:00:00:01:00
RSN: added PMKID - hexdump(len=16): f1 71 7b 7e 4e 2b 60 29 9c 85 f5 bd 04 08 74 75

wlan1: Event RX_MGMT (18) received
association request: STA=02:00:00:00:01:00 capab_info=0x431 listen_interval=5 seq_ctrl=0xea0
  new AID 1
Add associated STA 02:00:00:00:01:00 (added_unassoc=1 auth_alg=3 ft_over_ds=0 reassoc=0 authorized=0 ft_tk=0 fils_tk=0)
nl80211: send_mlme - da=02:00:00:00:01:00 sa=02:00:00:00:00:00 bssid=02:00:00:00:00:00 noack=0 freq=0 no_cck=0 offchanok=0 wait_time=0 no_encrypt=0 fc=0x10 (WLAN_FC_STYPE_ASSOC_RESP) nlmode=3

WPA: Assign ANonce - hexdump(len=32): 44 f4 fe 6a 60 75 cd 67 5a ab fd ae 04 a9 c5 49 74 91 2b 76 e1 e6 25 f6 02 af e2 02 bf 84 5e ee
wlan1: STA 02:00:00:00:01:00 WPA: sending 1/4 msg of 4-Way Handshake
wlan1: STA 02:00:00:00:01:00 WPA: received EAPOL-Key frame (2/4 Pairwise)
wlan1: STA 02:00:00:00:01:00 WPA: sending 3/4 msg of 4-Way Handshake
nl80211: Set STA flags - ifname=wlan1 addr=02:00:00:00:01:00 total_flags=0x6d flags_or=0x1 flags_and=0xffffffff authorized=1
wlan1: AP-STA-CONNECTED 02:00:00:00:01:00
wlan1: STA 02:00:00:00:01:00 IEEE 802.1X: authorizing port
wlan1: STA 02:00:00:00:01:00 RADIUS: starting accounting session 25ECACB545D7EDA0
wlan1: STA 02:00:00:00:01:00 WPA: pairwise key handshake completed (RSN)
wlan1: EAPOL-4WAY-HS-COMPLETED 02:00:00:00:01:00
```


## Acknowledgments

Brecht Van de Sijpe wrote the initial version of this code as part of his master's thesis.

