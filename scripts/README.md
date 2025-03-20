# Multi-Password Dragonfly: Proof-of-Concept in Python

The repository contains a proof-of-concept implementation that adds multi-password functionality to the WPA3's Dragonfly handshake, also known as Simultaneous Authentication of Equals (SAE) in the IEEE 802.11 standard. With this extension, an Access Point (AP) will accept multiple passwords, and the AP will learn which password the client selected. It is designed to provide the same security guarantees as standard WPA3, i.e., resistance against offline dictionary attacks, forward secrecy, etc.

Note that this is currently only a proof-of-concept, we are still analyzing and verifying security properties, and seeing where the protocol can be optimized. _Do not use this code in production._ **Help on analyzing the security of this design, or other feedback, is welcome!**

## 1. Usage

To execute the script, first create a Python3 virtual environment. This only needs to be done once:

```
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

You can now execute the script using:

```
source venv/bin/activate
python3 multipw.py
```

The script will simulate a client (STA) that is connecting to an Access Point (AP).

## 2. Example Output

```
[02:34:33] [AP]  Generating 6 passwords
[02:34:33] [STA] Sending Commit frame to AP
[02:34:33] [STA]    rA = 4780629983874244704811463191972804250538218185282895222421079296132865440575
[02:34:33] [STA]    mA = 80244814990471731399264624959908550529105372215986323903904169890547227017272
[02:34:33] [STA]    sA = 85025444974345976104076088151881354779643590401269219126325249186680092457847
[02:34:33] [STA]    EA = CAA926D37AA6308A35C05AE259AF0EC0E8825CCCC42C63492C06F552676D4A74, 60981680B33A1CE32586B26749D1AA74C7FE37BE17BFC2800631E2BA79B36F5E
[02:34:33] [AP]  Generated global scalar sB
[02:34:33] [AP]     sB = 49654288084121117789359389875926970081985849934399579202375099334554125425894
[02:34:33] [AP]  Generated Element EB for each password
[02:34:33] [AP]  Encoded all EB points using Elligator Squared to (u,v) values
[02:34:33] [AP]  Interpolated all (u,v) values based on the hashed passwords
[02:34:33] [AP]  Sending global scalar sB and the interpolated coefficients to the client
[02:34:33] [STA] Retrieved the decoded EB element based on the received coefficients
[02:34:33] [STA]    Decoded EB = 7C5D532754106D1FD55C6B60A6AB65C8AAACD92AA8FE28B961DC84A900031FE1, 2D6F64ADB2962CD8560AE6A7952FCB59D3F13C03C1BA3235FCB3308951563F23
[02:34:33] [STA] Calculated the negotiated key and the KCK and PMK keys
[02:34:33] [STA]    client_k = 32780657976393656807032478464653773115927738149197337111920281538096993986600
[02:34:33] [STA] Sending the calculated the confirm value to the AP
[02:34:33] [STA]    client_confirm = 3a6199bf764d5e2c28f3016b0c0059fece84f5c5a9f379d7aee4de04ba76d8bd
[02:34:33] [AP]  Interating over all password to find matching confirm value...
[02:34:33] [AP]      Calculated confirm value cf502f4942b3381dbfaee3ef9a39c5079648584b0db7a3aca99cc468f5460b9d
[02:34:33] [AP]      Calculated confirm value 44cd30a58c770841660cb12d2d0ced47db3755d1ad3f8d7fee8210a946013255
[02:34:33] [AP]      Calculated confirm value 229af3d7f8d1b93bad16db211724917e8ea3aa5c55bee33664e6b5a860331ab3
[02:34:33] [AP]      Calculated confirm value 3a6199bf764d5e2c28f3016b0c0059fece84f5c5a9f379d7aee4de04ba76d8bd
[02:34:33] [AP]  Found the password that the client used: password_3_519
[02:34:33] [AP]  Sending confirm value from AP side
```

## 3. High-Level Description

The implemented protocol corresponds to slide 32 in our [PAKE'25 presentation](../docs/pake2025-slides.pdf). In summary, our extension is inspired by the [O-PAKE protocol](https://link.springer.com/chapter/10.1007/978-3-319-23318-5_11) and, in particular, relies on index hiding message encoding (IHME). At a high level, the protocol works as follows:

- The client sends a normal Commit frame to the AP.
- The AP now replies with _n_ Commit frames, where _n_ equals the number of passwords. Instead of sending these commit frames directly, the elliptic curve points in these commit frames are encoded using IHME. That is, a polynomial is [interpolated](https://en.wikipedia.org/wiki/Polynomial_interpolation) through n data points, where the x coordinate of each data point equals a hashed passwords, and the y coordinate of each data point equals the encoding of the elliptic curve point in each commit frame. The coefficients of the polynomial _f_ are transmitted to the client.
- The client calculates _f(hash(pw))_ to obtain the elliptic curve point that matches the client's password. If the client uses a wrong password, this results in a random but valid elliptic curve point. The client performs the usual Dragonfly computations based on the elliptic curve point and replies to the AP using a Confirm value.
- The AP iterators over all possible passwords until the password is found that results in the same Confirm value as received fromt the client. If no match is found, authentication is failed. If a match is found, the AP learns which password the client has used.
- Finally, if a password match was found, the AP also replies with a Confirm value to the client.

Extra remarks:
- Note that IHME, i.e., polynomial interpolation, is only used to encode the elliptic curve point. The scalar value used in Dragonfly's Commit frames is sent in plaintext and shared across all commit frames.
- It is essential that _f(x)_ results in a valid elliptic curve point for all _x_. We use [Elligator Squared](https://eprint.iacr.org/2014/043.pdf) to encode an elliptic curve point into two numbers _(u,v)_ where every combination of these numbers represents a valid curve point. We then separately perform polynomial interpolation for the u-values and v-values.
- When the AP iterates over all passwords, this should be done in a random order. Otherwise, an adversary can perform a timing attack to identify whether different clients use the same password.
- For the polynomial interpolation step, we first precompute the [inverse of the Vandermonde matrix](https://dl.acm.org/doi/pdf/10.1145/1966913.1966950), and then use this matrix to separately encode the u-values and v-values that are output by Elligator Squared.


## 4. Possible Extensions

- Our implementation of Elligator Squared can likely be optimized to be faster and take slightly less output, see [ElligatorSwift](https://eprint.iacr.org/2022/759.pdf).
- In case Dragonfly switches to [Safe Curves](https://safecurves.cr.yp.to/ind.html), then [Elligator 2](https://elligator.org/) encoding can be used, which encodes elliptic curve points using only a single number. This means that data overhead would be reduced by half.
- Dragonfly has not been proven secure when the first message is reused. However, [reusing the first message has been proven secure for CPace](https://eprint.iacr.org/2024/234.pdf). This would allow reusing the interpolated polynomial across multiple handshakes.
- More generally, how that patents such as [US6226383B1](https://patents.google.com/patent/US6226383) are expired, we consider usage of **[CPace](https://datatracker.ietf.org/doc/draft-irtf-cfrg-cpace/) instead of Dragonfly** to be a more efficient and cleaner solution.


## Acknowledgments

Brecht Van de Sijpe helped with the implementation as part of his master's thesis.
