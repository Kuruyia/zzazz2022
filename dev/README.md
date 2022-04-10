# Directory - dev/

This directory contains all the programs I've written around this event, be it for completing challenges or just for fun.

Most of them are TypeScript scripts that are to be run with the [Deno runtime](https://deno.land), although the ones pertaining to Cracker Cavern III are C++ programs.

Please note that for all the scripts that interact with the server, the URLs are hard-coded for my UID. It should be changed if used with someone else's account.

## Directory structure
- `zzazz2022_autolottery` - A script that will automatically play the lottery for you, every two hours.
- `zzazz2022_challenge3` - The first program written, that emulates enough map script opcodes to run the Cracker Cavern III password script.
- `zzazz2022_challenge3_taketwo` - The revised program for craking the Cracker Cavern III password.
- `zzazz2022_challenge4` - A script for interacting with the endpoints for Cracker Cavern IV. Most stuff done for this challenge were modifying data by hand.
- `zzazz2022_mapdumper` - The first script I've written that interacts with the server. Dumps all 65536 maps.
- `zzazz2022_maputils` - Some utilities that were written for working with the dumped maps. Mostly string extraction.