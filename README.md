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

Jackpot! I now know exactly where the script is located, and how to interpret all of this, I can now just poke around and see that the entire script section that does something with the keyboard input is located from address `0x201835E` to `0x02018994`.

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
- Password "AA" leads to checksum 0x00
- Password "BA" leads to checksum 0x09
- Password "CA" leads to checksum 0x12 (0x09 * 2)
- Password "AB" leads to checksum 0x45
- Password "BB" leads to checksum 0x4E (0x09 + 0x45)

We can clearly see that character #1 has a "weight" of 0x09, and character #2 has a "weight" of 0x45. That is pretty much what was happening with this algorithm.

This is how I replaced 1.5kB worth of script with a few multiply bois. So I [implemented that](https://github.com/Kuruyia/zzazz2022/blob/main/dev/zzazz2022_challenge3_taketwo/main.cpp), verified that it was giving the correct checksums for known input, and let it do its bruteforce shenanigans.

What about performance? We're talking 8M checksums/s... oh wait, forgot the `-O3` flag, more like 50M checksum/s now. Yes, 50 _millions_. A bit better than the previous 1.6k checksums/s I'll admit.

So I left it running for quite a while, and it finally gave me a valid password. Quite a while being approximately 12 seconds of course.

So I got back to Cracker Cavern III, talked to the NPC, entered the password `l72SkAAAAA`, let it verify the password client-slide and server-side, and bingo, Cracker Cavern III was finally done! Now it's time to catch some zzzZZZzzz.