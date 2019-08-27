# Quest 5 Report

Authors: Genny Norman, Cameron MacDonald Surman, Ruby Zaveri, 12-3-18

## Summary

In this Quest we build a smart key that was able to unlock our security hub by via a button press. When the button was pressed it transmitted a short range message to the hub. The hub unlocks and logs the presence of that key.



## Evaluation Criteria
1. Fob Relays to security hub and hub sends ID and code to server; server responds to fob, fob green light turns on.
2. Logs fob key accesses to database.
3. Database is on Rpi
4. Web-based management shows real time active unlocked fobs and history of unlocked fobs.
5. Uses fobs with 3 unique IDs.
6. Security vulnerabilities are assessed.


## Solution Design

We built a key and a hub able to receive and transmit data. The key sent a specific ID to the fob that enabled the fob to be unlocked. This process was displayed on our web client.

Fob - Used a RX to receive a the 4 digit password that our key was initilized with.

Key - Used TX to transmit a password in order to unlock the fob.

Web Client - Key and the Fob sent information (locked/unlocked, IDs etc) to the web client via GET requests. The client displayed the data from previous attempts of unlocking the device. It also displayed the most current attempt with the ID number.

Database - The database stored previous unlocking attempts. It stored information from the fob and the key including the ID some other data and our specific group ID (go16).

## Possible Security Breaches

We used security through obscurity - nobody knows what is on there therefore they can't break the code. If someone was to figure out that it was a two 4 digit codes that corresponded to each other they might be able to hack it (man in the middle attack)

## Sketches and Photos

[Video of our Solution](https://drive.google.com/open?id=1sbreZVgz1KlcVqc9igXXkGBUlsTSEhSR)


Receiver and Transmitter Devices (Key and Fob)
![top view](https://i.imgur.com/9LC8RCW.jpg)
Side view
![side view](https://i.imgur.com/mMeArPo.jpg)

Screenshots of Our web client
![web client](https://i.imgur.com/lykLKGk.png)


## Supporting Artifacts

- [CanvasJS example](https://canvasjs.com/javascript-charts/stacked-column-chart/)
