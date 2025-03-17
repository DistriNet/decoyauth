# Proof-of-Concept in Python

The repository contains a proof-of-concept implementation of our DecoyAuth protocol. Note that this is currently only a proof-of-concept, we are still analyzing and verifying security properties, and seeing where the protocol can be optimized. _Do not use this code in production._ **Help on analyzing the security of this design, or other feedback, is welcome!**

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
python3 decoyauth.py
```

The script will simulate a client (STA) that is connecting to an Access Point (AP).

## 2. Example Output

```
[14:35:50] [AP]  Generating 6 keys
[14:35:50] [STA] Sending Commit frame to AP
[14:35:50] [STA]    rA = 61770774598897354265563211032699903532967966875938790812070862712445791294426
[14:35:50] [STA]    mA = 111893292263628745388967572053310081255547804124851003651860192889191048433728
[14:35:50] [STA]    sA = 57871977652169850891833336136602411258518815776654034121508796540568327683785
[14:35:50] [STA]    EA = 6B83648A3119883FD2D68297C5F7D6BD654C5A1277EFF90E9763612FF70DA2C7, E308DBDF8F21CA0B7729D780F53FDCBC000089735ED56F91FB625856925EA04B
[14:35:50] [AP]  Generated global scalar sB
[14:35:50] [AP]     sB = 1326983285130808930482715311191542582271607776986648738597612248339997416199
[14:35:50] [AP]  Generated Element EB for each key
[14:35:50] [AP]  Encoded all EB points to values
[14:35:50] [AP]  Sending all values to the client
[14:35:50] [STA] Retrieved the decoded EB element based on the received values
[14:35:50] [STA]    Decoded EB = 451D074A68D29AFEFB3118580E947A055B34070EA889B84FCFFBAFF678DF110, 217AD45F957A9647C41F943380E4C0329750F9121005DD1273843C940B9A5CE0
[14:35:50] [STA] Calculated the negotiated key and the KCK and PMK keys
[14:35:50] [STA]    client_k = 94683776479305982245761672275645709392025240814192568856317405541261650310660
[14:35:50] [STA] Sending the calculated the confirm value to the AP
[14:35:50] [STA]    client_confirm = 8a6a3c5a5be1588d11f319f181c913c5586e4f22a4b8b07f170e94d8ef0f04d4
[14:35:50] [AP]  Finding matching confirm value...
[14:35:50] [AP]      Calculated confirm value 2e9d8a5c73663801c2477832d572740d072f8975e972924e52619d6e3bf2bf4d
[14:35:50] [AP]      Calculated confirm value b8a851db67b8a3d9dc6f24797a9a968d0e50366829888f887233db2283cd259e
[14:35:50] [AP]      Calculated confirm value 76164b9acc882b97a1572c5d196018bdea3291e9671c8ae0b45da0be64b1e07e
[14:35:50] [AP]      Calculated confirm value 8a6a3c5a5be1588d11f319f181c913c5586e4f22a4b8b07f170e94d8ef0f04d4
[14:35:50] [AP]  Confirmed key!
[14:35:50] [AP]  Sending confirm value to the client
```

## 3. High-Level Description

The implemented protocol corresponds to slide 32 in our [February 2025 presentation](../docs/slides-feb2025.pdf). In summary, our extension is inspired by the [O-PAKE protocol](https://link.springer.com/chapter/10.1007/978-3-319-23318-5_11). At a high level, the protocol works as follows:

- The client sends a normal Commit frame to the AP.
- The AP now replies with an O-PAKE style encoding of the key messages.
- The client uses its own key with O-PAKE style decoding to get the corresponding message. If the key is invalid, this results in a random but valid elliptic curve point. The client performs the usual Dragonfly computations based on the elliptic curve point and replies to the AP using a Confirm value.
- The AP searches for a key match by verifying the received Confirm value. If this fails, authentication fails. If this succeeds, authentication has completed.
- Finally, the AP replies with a Confirm value to the client.


## Acknowledgments

Brecht Van de Sijpe helped with the implementation as part of his master's thesis.
