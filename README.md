# Multi-Password Support for WPA3

<img align="right" src="multipw.png" width="100px">

_<p align="center">Part of [NGI Sargasso's](https://ngisargasso.eu/innovators/) Decoy-Auth project.</p>_

This project investigates how to support **multiple simultaneous passwords in modern WPA3 networks**.
This means each user or device can be given a unique password, which simplifies user and device
management while also allowing the user's identity to be derived from their password.

<img align="right" src="decoyauth.png" width="100px">

This is done as part of the Decoy-Auth project, funded by NGI Sargasso, aiming to design
a zero-knowledge authentication protocol that supports multiple passwords. This allows using
multiple passwords for network access and allows using decoy (honey) passwords that act as
reverse honeypots to detect security breaches.

# Background

With WPA2, it is possible to assign unique passwords to different users or devices, allowing the
access point to automatically determine which password is being used. However, this functionality
is no longer possible in WPA3 due to its zero-knowledge authentication protocol. In other words,
with WPA3 the access point can only verify whether a _specific_ password is used by the client,
rather than identifying _which_ password was used. This limitation is common among modern
Password-Authenticated Key Exchange (PAKE) protocols, where servers or access points are
restricted to accepting only one predefined password per authentication attempt.

This project aims to overcome this limitation by extending WPA3—or developing a new protocol if
necessary—so that an access point can accept multiple different passwords. The objective is to
achieve this without requiring usernames or password identifiers, ensuring both security, privacy,
and ease of use.

# Latest Updates

- 7 February 2025: Presentation at the [PAKE'25 workshop](https://afonsoarriaga.github.io/pake-25/) in Luxembourg. The [slides are public](docs/pake2025-slides.pdf).
- 18 March 2025: [Proof-of-concept Python code](scripts/README.md) of a first protocol design was added. This is only for experimentation, do not use it in production.

