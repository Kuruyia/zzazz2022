# Directory - dev/zzazz2022_maputils
This directory contains TypeScript scripts that processes maps dumped from the map dumper.

Those utils are centered about string extraction, and were made because I couldn't for the life of me figure out where Hidden Retreat was hiding (guess its name fits well huh).

To use those scripts, you'll need to create a `maps/` directory and copy all the maps that you dumped into it.

The first script, `filter.ts`, will check all `.bin` files of the `maps/` directory to delete every dumped map that invalid (by checking their file size and SHA1 checksums).

The second script, `string_extractor.ts`, will read every `.bin` file and convert every byte that's in the charset of Pokémon Emerald to ASCII, and save the result in an `.ascii.bin` file. You'll probably want to run `filter.ts` beforehand.

The third script, `name_extractor.ts`, will read every `.ascii.bin` file, and for each of them extract every string that is at least 4 consecutive uppercase letters (thinking that one of those string will certainly be the map name). The result is saved in a `chains.txt` file, which is a JSON object that maps every map ID to the extracted strings. You'll need to run `string_extractor.bin` beforehand.

## Usage
- Copy all your dumped maps in a `maps/` directory.
- To filter all the maps to keep only the valid ones, run `deno run --allow-read --allow-write filter.ts`
- To extract strings from the maps, run `deno run --allow-read --allow-write string_extractor.ts`
- To extract names from the maps, run `deno run --allow-read --allow-write name_extractor.ts`