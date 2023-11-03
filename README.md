# NOSTR Machines

#1 Safe Box: An example of how to use DM and Zaps to interact with IoT devices
First example: A Multi-DM Safe Box

A Lilygo T-Display S3 will be our Nostr client and serve as our Safe Box for this time-limited competition; hence, it should be waiting for the previously hard-coded pubkeys to send it a DM.
It should become orange on the display when one of the DM is identified, suggesting that another pubkey needs to DM as well. It should flash green as soon as the other pubkey DM arrives, indicating that the conditions are met; it could then send a signal to the safe box itself to open the door. And for now, that's it.

![WhatsApp Image 2023-11-03 at 16 33 15](https://github.com/nitroxgas/nostrmachine/assets/6924947/6b8db513-9de9-42f5-8eed-76a7eb09679e)

To-Do:
* Sure, tidy the code;
* Add support to NIP-5, Zaps, and passphrases at the DM;
* Implement other use cases and share them the Github;
* Clean the code always;
* Implement time lock;
* Implement pubkey validation with Zaps
* Build other examples;

* Special shoutout to the LNbits and MakerBits communities, Ben Arc, BlackCoffe, and Calle for the amazing work and collaboration, and also to Abdre Neves for the mentoring!

* Project built and presented at SatsHack São Paulo on November 3, 2023. Presentation: https://docs.google.com/presentation/d/1dhbu2fdxXNxrFq2Rap_0n3heBFkCwFHG/edit?usp=sharing&ouid=103025837986292071011&rtpof=true&sd=true

Mihainuan @mihainuan

George Alexandre Silva, @nitroxgas



