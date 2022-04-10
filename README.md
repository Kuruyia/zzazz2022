# TheZZAZZGlitch Fools 2022 - Kuruyia

This repo contains some codes and notes about how I've done the challenges of [TheZZAZZGlitch's April Fools Event 2022](https://zzazzdzz.github.io/fools2022/index.html).

I'd like to thank [TheZZAZZGlitch](https://www.youtube.com/user/TheZZAZZGlitch) for organizing this event (and all the past ones of course), as well as the community for playing it. It's been very fun and interesting to play and overcome all the challenges that it had to offer.

I've actually played the [2018 event](https://zzazzdzz.github.io/fools2018/), but only scored second place back then because I didn't have enough time to complete everything.

But now things have changed: armed with a mediocre knowledge of Pokémon Emerald, and the willingness to sacrifice my nights ('cause honestly who needs to sleep duh), I could finally once more overtake the beast that is Glitchland.

## Writeups
Those writeups concerns exclusively the Cracker Cavern challenges, in which you were presented with 4 challenges that needed to be solved to get those sweet, sweet points on the leaderboard.

They won't necessarily be accurate to what I've done exactly as I'm writing those after having beaten all of the challenges, so I recommand that you also take the time to read other people's writeups to have more perspectives (also because my writeups are probably badly written, there are no images and it has bad humor), they'll probably be linked in this event results video.

With that out of the way, let's get started!

### Cracker Cavern I & II
I won't get into too much details concerning those two challenges, as they were very easy, but for completeness sake they are still being written about.

The first challenge, Cracker Cavern I, required the player to walk through walls to get in a blocked off area.

So, let's open mGBA's memory search window, move a bit and look for where the player's coordinates are stored.

I quickly saw that the player X position is at address `0x2037360`, so I just changed the value at that address to teleport next to the staircase and voilà, first challenge done!

The second challenge, Cracker Cavern II, was a bit more complicated as the player needs to load a map with a specific ID (`0x1337`).
If you didn't play the event, basically (as it was with the 2018 event), every map in the game is stored on a server over the Internet, and when the game needs to load a new map, it actually requests them via the (virtual) Link cable to a Windows client, that will then make the HTTP request to get the map from the server and give the data to the game via the same Link cable.

I started by disconnecting this Link cable so the game couldn't communicate with the client anymore. I saw that the client mentioned something about "JoyBus", which is actually a proprietary communication protocol made by Nintendo.

After finding where in the memory the registers were to get stuff sent over this protocol (registers `JOY_TRANS_L` and `JOY_TRANS_R`, documented over at [problemkaputt.de](https://problemkaputt.de/gbatek.htm#siojoybusmode)), I noted what bytes the game was trying to send, and then connected back the Link cable between the emulator and the client.

I then fired up WireShark to analyze the traffic between those two (the data was actually being transmitted over TCP/IP, so I just needed to sniff on the loopback adapter), searched for the packets that contained the bytes I noted earlier, and done that a few more times by changing maps to see what bytes were changing and what bytes were not.

I quickly saw where in the packet the map ID was, and then just searched for this map ID in the game memory to see where it was there. Once this was found out, I just froze the value at this address using a CodeBreaker cheat code, and loaded a map. This completed challenge 2!

### Cracker Cavern III
Now the real stuff begins: in this challenge, the player can talk to an NPC that will ask for a password, and tell us if it was correct or not. Easy right? So I started trying every combinations by hand, but quickly realized that the password has 16 characters that could contain any letter of the alphabet (uppercase or lowercase), any number and any of the special characters that was available. That's more than 70 different characters, and 70^16 is a lot of numbers.

Because I badly wanted to live a fulfilling life during the next few centuries, I continued any... I decided that automating this task might be a solution worth trying.

The first thing to do was to look for where the keyboard buffer was stored in the memory. mGBA's search memory tool was once again saving my life and I quickly saw that this buffer was located @ `0x020045E0`. By typing the sequence of characters ".j.j" (without the double quotes), which translated to `0xDEADDEAD`, I could then, after having validated the entry, search the memory for this magic number and find where the keyboard buffer was being copied to.

I found three addresses: `0x02021CC4`, `0x02021DC4` and `0x020297D8`.

By placing read watchpoints at those addresses (that will basically halt the game from being executed whenever the game is trying to read from those addresses), I could determine that the address that was hiding something from me was `0x02021DC4`: indeed, after the screen finished fading out (which I suspected was the job of the keyboard), this address was being read right before the NPC told us if the password was wrong or wrong.

So, I quickly looked at the register dump, stacktrace and WHAT THE HELL IS ALL OF THIS?

At that point, what I forgot to take into account was that the GBA was a very different beast from the GB, and it allowed a lot of fancy stuff for the developers.

After browsing the [pokeemerald code](https://github.com/pret/pokeemerald) (a Pokémon Emerald reverse-engineering project) for a bit, I realized that the game had a system of Tasks that were each assigned a role (such as managing weather, or displaying the keyboard), which is a code architecture much more advanced than what I was expecting (being still in the 1st/2nd gen Pokémon games mindset at that time).

After being amazed by this, I discovered that the pokeemerald project also had a [symbol map](https://raw.githubusercontent.com/pret/pokeemerald/symbols/pokeemerald.sym) that was hiding in another branch, so I could finally make sense of what was written in the stacktrace (this symbol map was a huge thing for the rest of this challenge, even if I don't talk about it that much).

So I quickly looked for the name of the function that was trying to read the copy of the keyboard buffer, and it turned out that it was an innocent function named `ScrCmd_loadbytefromptr`.

That led me to looking at its source code to understand what it was doing.

That led me to understand that this was actually part of something that was interpreting bytecode. Ok nothing out of the ordinary.

**WAIT WHAAAAAT?!**

That was the second time my mind was blown at the architecture of this game: the game engine has an entire scripting engine for the maps that allows them to do high level stuff such as teleport the player, show a message box and add items to the player inventory, but also low level stuff such as reading and writing arbitrarily to the memory, do comparisons, (conditionally) jump to other areas of the script...

And all of the possible opcodes are, of course, [documented on pokeemerald](https://github.com/pret/pokeemerald/blob/master/asm/macros/event.inc), which was amazing.

And there it lied: in one of the registers of the ARM CPU, one value that suspiciously looked like an address. Maybe was it the address of the script instruction being currently run? Indeed it was. Comparing the value of the opcode for this instruction (`loadbytefromptr`: `0x12`) and what was lying on that address confirmed this.

Jackpot! I now know exactly where the script is located, and how to interpret all of this, I can now just poke around and see that the entire script section that does something with the keyboard input is located from address `0x0201835E` to `0x02018994`.

A quick glance at all of this code, I could clearly see that the same block was being copy-pasted a lot of time. This block is actually what was computing the checksum from the keyboard buffer, and two interesting facts could be seen: 1) the code only takes into account the first 10 characters of the keyboard buffer, meaning the password is 10 characters and not 16, and 2) there was not one, but two separate checksums that were computed.

This last fact could be confirmed by looking at the end of this script, where I found (and expected to find) the code that was checking those two checksums: the first checksum value was `0xD4B9`, and the second checksum value was `0xB0EF`. We were expected to find a password that would generate those two checksums.

([This file](https://github.com/Kuruyia/zzazz2022/blob/main/challenge3_script.txt) contains some hand-made dissasembly of part of this script.)

(Also, the password is also checked server-side, so we can't easily cheat.)

At that point, I had the algorithm to compute both checksums from a password, and the expected results, time to write a bruteforce program!

My first idea was to just yank the script so it could be executed in an interpreter I would have written, which is exactly [what I've done](https://github.com/Kuruyia/zzazz2022/blob/main/dev/zzazz2022_challenge3/main.cpp): I've implemented enough of the opcodes, thanks once again to the pokeemerald project, to execute this entire script, and just simulated the keyboard buffer that would cycle each of its characters between `[A-Za-z0-9., ]{10}` (pardon my Regex).

After launching the bruteforcer, the results were very disappointing: 1,600 checksums/s. With 65^10 combinations, that would take a lot of centuries to complete. It was 1:30 AM and I badly wanted to cry.

But then, out of nowhere, a revelation: let's poke around with the input and see what's going on on the output!

And this is where everything went for the best: I quickly remarked that each character of the input had a "weight" in the output, and that each character was independent from the other ones.

A very simplified example of this would be:
- Password "AA" leads to checksum `0x00`
- Password "BA" leads to checksum `0x09`
- Password "CA" leads to checksum `0x12` (`0x09` * 2)
- Password "AB" leads to checksum `0x45`
- Password "BB" leads to checksum `0x4E` (`0x09` + `0x45`)

We can clearly see that character #1 has a "weight" of `0x09`, and character #2 has a "weight" of `0x45`. That is pretty much what was happening with this algorithm.

This is how I replaced 1.5kB worth of script with a few multiply bois. So I [implemented that](https://github.com/Kuruyia/zzazz2022/blob/main/dev/zzazz2022_challenge3_taketwo/main.cpp), verified that it was giving the correct checksums for known input, and let it do its bruteforce shenanigans.

What about performance? We're talking 8M checksums/s... oh wait, forgot the `-O3` flag, more like 50M checksum/s now. Yes, 50 _millions_. A bit better than the previous 1.6k checksums/s I'll admit.

So I left it running for quite a while, and it finally gave me a valid password. Quite a while being approximately 12 seconds of course.

So I got back to Cracker Cavern III, talked to the NPC, entered the password `l72SkAAAAA`, let it verify the password client-slide and server-side, and bingo, Cracker Cavern III was finally done! Now it's time to catch some well-deserved _zzzZZZzzz_.

### Cracker Cavern IV

Finally, we're arriving at the last challenge: the one to rule them all.

We are immediately greeted by a signpost and an NPC. The signpost informs us that the NPC delivers "silver" certificates, but it'd like to see a "gold" one. Unfortunately, those certificates are protected by millitary-grade (big words here) AES encryption.

(I'd recommend reading a bit about the [AES](https://en.wikipedia.org/wiki/Advanced_Encryption_Standard) and [Block cipher mode of operation](https://en.wikipedia.org/wiki/Block_cipher_mode_of_operation) Wikipedia pages to get some context if you're unfamiliar with how all of that works)

Speaking to the NPC confirms that in fact, it contacts the server to generate a "silver" certificate for the holder name "Aelita". Trying to speak with the NPC again leads us to two choices: appraising the certificate, or generate a new one. Appraising the certificate seemingly makes another request to the server that will verify this certificate and give us all the infos it has: the holder name "Aelita", the certificate type "silver", but also a random 6-number serial number and an authority "CrackerFour".

(Btw, we're not talking about certificates as in X.509 certificates, we're just dealing with a regular plain ol' encrypted blob of data)

With all of that, let's start playing the game!

First of all, I fired up Firefox and went to the HTML5 client so I could clearly see the traffic that was happening when the game was contacting the server (at that point, I had already done that to write my [map dumper](https://github.com/Kuruyia/zzazz2022/tree/main/dev/zzazz2022_mapdumper), so I already had something setup to authenticate with the server and start making requests), so I could get a better grasp at what was happening behind the scenes.

The NPC was making two requests to the server: one for generating the certificate, and another one for appraising it. Let's look at the first request:

That request's payload contained `holder=Aelita/type=silver`, and the response (after decoding the base64) contained 64 bytes of seemingly random data, which is exactly 4 times the block size of AES. Seems good so far!

Let's try attacking that endpoint first, changing a value here and there and... nevermind, this seems to be a dead end, we can't use the server to generate a "gold" certificate for us. Anyway, while fiddling around with this endpoint, I noted some interesting facts:
- The payload MUST exactly be 32 bytes. Nothing more, nothing less, or else the server rejects the request.
- I can willingly change the holder name, no checks are being done on that. It's even possible to entirely omit the `holder` field.
- On the other side, I, unfortunately, couldn't change the certificate `type` field to anything else than `silver`. Doing so would make the server reject this request, saying that we're not authorized to generate another certificate type (even if the holder name is "CrackerFour" :sadface:).
- Byte `0xFF` is what terminates the payload. It can be omitted if the payload size takes the whole 32 bytes.

Another interesting fact is that, while with the default holder name `Aelita`, the generated certificate size was 64 bytes, if I tried to generate a certificate with the maximum amount of letters in the holder name, a fifth cipher block would be generated, making the certificate size 80 bytes.

Ok, so the certificate generation endpoint seems to be a no-go. What about the appraisal endpoint?

WHY IS THE REQUEST SIZE 260 BYTES?!

Let's caaaaaalm down, let's talk to the NPC once more so it can appraise the certificate once again: the request was exactly the same, good; let's talk to the NPC so it can generate a new certificate and appraise it: apart from where the certificate is stored in the request, every other byte is the same.

To this day, I still don't know what all of those bytes are about, and I'd be more than happy to look at the source code of the client/server once they're released to understand what's all of that about (and if those bytes are even are used anyway).

So all of that's very good, I can just yank the response, save it in a file, and then just read that file from my script whenever I want to make a request to that endpoint, taking care of replacing the certificate data with whatever I want (certificate data (encoded in base64) being very clearly located from bytes `0x04` to `0x5B` in this request).

After confirming that this would indeed work, I started dissecting the response that this endpoint was giving: it's got the exact same data format we pass for requesting the generation of a certificate, and has all 4 fields that the NPC was saying.

The response was something like `authority=CrackerFour/serial=123456/holder=Aelita/type=silver`, and it seemed like it was just the decrypted contents of the certificate.

Because indeed it was! After some fiddling around, I quickly confirmed this fact, let's put those two next to each other:

```
Encrypted data                                      Decrypted data
21 32 27 A7 E8 5C 4B 07 CF D8 2E 6B BD 30 32 CA     authority=Cracke
F8 36 EF 12 57 CE 00 1C C5 F2 2B FE 6E 1E 2D DC     rFour/serial=123
D9 34 7D 7E A5 E7 83 9A AB EB 78 24 DA 69 8F E7     456/holder=Aelit
E2 33 D1 06 23 F0 F7 1F 43 01 2C 46 8F 1A E4 16     a/type=silver
```

Indeed, it perfectly fits! Another confirmation can be provided with the holder name size: a 9-characters long holder name would generate 4 cipher blocks (64 bytes of encrypted data), while a 10-characters long holder name would generate 5 cipher blocks (80 bytes of encrypted data). Exactly what we'd expect!

Annnnnd this is where it all went downhill: I had mistakenly expected the encryption method to be `AES-ECB` (where every block is independent from the other ones), and this assumption could have entirely been verified by taking 10 seconds to generate two certificates with the same holder name. In that case, only cipher blocks #2 and #3 would have genuinely changed, and cipher blocks #4 (and #5, if present) would have changed because of that.

So I unfortunately lost a few hours by testing various ideas that couldn't possibly work because of this false assumption. But I made interesting observations about that endpoint so not everything was lost:
- The first cipher block is always the same, because the data it contains is always `authority=Cracke`.
- The `authority` field MUST be present, and its value must be `CrackerFour`.
- The `serial` field is, when legitimately generated by the server, just 6 numbers, but the appraisal endpoint doesn't enforce that format, and it can be basically whatever I wanted (I couldn't test if its presence was required tho').
- The `holder` field is not checked, nor is its presence required by the endpoint. (Interesting fact, when a certificate is generated with no `holder` field specified in the request, the server automatically adds a `holder` field whose value is `undefined`, so there's still a holder name).
- The `type` field MUST either be `silver` or `gold` (what I've wanted to reach for so long), anything else and the server rejects the request.

But then, while I was brainlessly changing random bytes in the encrypted data, out of nowhere, a _miracle_ happened.

The server replied with something I had seen too many times already: `invalid cert type s:lver`.

Waiiiiiiiiiiiit a minute... I was used to this response by now, but so far the certificate type always got replaced by garbage bytes, because I was thrashing its cipher block.

And now, it seemed that I unwillingly and randomly changed one of its characters?!

I tried changing the same byte, and the exact same character got replaced by something else. Tried changing the byte after, and the character after also got replaced by something else. I knew I was on the right track, although I didn't know how.

Now that I was able to change the characters of the certificate type, and having quickly understood that the relation between the encrypted byte I was changing and the character it got decrypted to was just an XOR operation, I carefully changed 5 bytes of the certificate type to read the following: `gold[0xFF]`.

I quickly then sent this newly-crafted certificate for appraisal, and saw that not only was the certificate accepted by the server, but also that its type was indeed `gold`. A quick F5 on the event page, and I had indeed achieved Cracker Cavern IV and got the max score of 1338, yay...

...except I still wasn't happy: indeed I had completed this challenge, but that wasn't fulfilling. I didn't know how I did that, I just had an extreme amount of luck by flipping that random byte. I had to know why it worked, and at that time, I was feeling empty, feeling like a fraud, and I even started to write overexaggerated stuff about how I felt back then.

Anyhow, because I was also feeling lazy, I decided to write something that would [play the lottery for me](https://github.com/Kuruyia/zzazz2022/tree/main/dev/zzazz2022_autolottery), watched some YouTube video and went to sleep.

And only 5 minutes after having closed down my laptop, my brain decided to start working again and told me something along the lines of "hey m8, think I found why it worked, how 'bout you open your laptop and write everything down. that was not a question. thx".

So that's exactly what I did, some 1AM thoughts, on why what I had done worked:

Actually, instead of what I originally thought, this certificate must be encrypted using `AES-CBC`, which introduces a direct dependency between a cipher block and the one preceding.

In fact, with `AES-CBC`, when you're done decrypting a cipher block, you must XOR the result of this decryption with the previous cipher block to obtain the correct plaintext.

Does that ring a bell? That's exactly what I've accidentally done to solve this challenge: changed the bytes that were 16 bytes before the value of the `type` field so I could manipulate the result of this XOR operation to be whatever I wanted, in this case `gold`.

But, in doing so, we're actually thrashing the entire cipher block that we're actually manipulating to change the values of the next cipher block. Is it that important tho'? 

Apparently not, because 1) When I did that, I successfully got a gold certificate, but more importantly, 2) the cipher block I was thrashing contained the end of the serial number and the beginning of the holder name, BUT I already discovered that the serial number format was not being checked anyway, and that an holder was not necessary for a certificate to be considered valid. Bingo, I now knew why what I did worked, and I can proudly say that:

Cracker Cavern IV is done!

### Conclusion
As a conclusion, doing those Cracker Cavern challenges was a lot of fun, but saying this was all the fun I had would be incorrect.

I have a huge respect for everything that went behind the scenes, such as the server and client, the ACE savefile, all the map and quests design, the YEET challenges...

All of that was very cool, and once again thanks to TheZZAZZGlitch for having done that.

I finally got my revenge on Glitchland by finishing all the challenges that it had to offer and, while I don't remember a lot of the 2018 event, I always thought good of it, and it was a huge pleasure to be able to come back to the region in all its 3rd gen glory!

## Directory structure
- `dev/` - The directory containing all the programs I've written for this event.
- `cert_gold.bin` - The winning certificate, one that is working for completing the Cracker Cavern IV challenge.
- `challenge3_bytecode.bin` - A raw dump of the bytecode of the script that's calculating and verifying the checksums of the password for the Cracker Cavern III challenge.
- `challenge3_script.txt` - A hand-made disassembly of some parts of the script of Cracker Cavern III, as well as some other stuff related to that challenge.
- `map.pdf` - A map of Glitchland. Very high quality.
- `packets_reverse.txt` - Stuff about some packets that are being exchanged between the server and the client.
- `TODO.txt` - The OG file: the one that contains all my notes from the very beginning of my playthrough.