# DecoyAuth

<img align="right" src="multipw.png" width="100px">

This project will research, develop, and standardize an innovative zero-knowledge authentication protocol that supports decoy tokens. The decoy tokens act like a reverse honeypot, where usage of a stolen or leaked token indicates a security breach, triggering appropriate security measures.

<img align="right" src="decoyauth.png" width="100px">

This is done as part of the Decoy-Auth project funded by [NGI Sargasso](https://ngisargasso.eu/innovators/).

# Background

Zero-knowledge authentication has major advantages. In particular, when using general-purpose keys, or traditional passwords, the counterparty only learns whether the correct key was used, without learning the value of the used key. Unfortunately, using decoy tokens (or decoy passwords) with existing zero-knowledge protocols is impossible. In this project, we overcome this limitation by adding support for decoy keys to zero-knowledge authentication protocols. The decoy tokens act as a reverse honey pot: a decoy key is indistinguishable from a real key, and when an adversary steals or uses a decoy key, the counterparty can detect this and take appropriate security measures.

The result will be the design, implementation, and standardization of an innovative zero-knowledge authentication protocol that supports decoy tokens. By supporting a decoy token mechanism, we expect a substantial increase in breach detections, leading to faster and more effective responses to security incidents.

More broadly, we hope our work will also be the basis of other future research on extending zero-knowledge authentication. In particular, we believe it might also form inspiration for adding multi-password support to WPA3.

# Latest Updates

- February 2025: Presentation on our initial design, the [slides are public](docs/slides-feb2025.pdf).
- 18 March 2025: [Proof-of-concept Python code](scripts/README.md) of a first protocol design was added. This is only for experimentation, do not use it in production.
- 22 April 2025: A [whitepaper](docs/whitepaper.pdf) with a draft description of the protocol is available.
- 27 April 2025: [Unit tests and test vectors](scripts/TESTS.md) have been generated and added.
- 27 July 2025: [Proof-of-concept C code](c-prototype/README.md) has been written for a draft protocol.
- 8 September 2025: [Proof-of-concept hostap code](hostap/README.md) that integrates our draft protocol.

