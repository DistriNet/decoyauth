# Integration in Wi-Fi client & deamon

The repository contains a proof-of-concept integration of our DecoyAuth protocol into Linux's open-source Wi-Fi client, i.e., `wpa_supplicant`. Note that this is currently only a proof-of-concept, we are still analyzing and verifying security properties, and seeing where the protocol can be optimized. _Do not use this code in production._ **Help on analyzing the security of this design, or other feedback, is welcome!**


## 1. Compilation

First compile the code:

```
cd wpa_supplicant
cp defconfig .config
make -j 4
```


## 2. Starting the client

After running the steps in [Compilation](#1-Compilation) you can start `wpa_supplicant` as follows:

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


## 3. Code integrations

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


## Acknowledgments

Brecht Van de Sijpe wrote the initial version of this code as part of his master's thesis.

