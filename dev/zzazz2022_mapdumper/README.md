# Directory - dev/zzazz2022_mapdumper
This directory contains the TypeScript script that will dump all the maps that are available to your account.

This should dump everything in approximately 24h (not a donator) or 12h (donator). Requests have been implemented in respect for the event's [automation rules](https://fools2022.online/automation/).

Please be aware that you might not be able to dump some maps:
- Some YEET maps cannot be dumped before beating another YEET map.
- Cracker Cavern III cannot be dumped before beating Cracker Cavern II.
- Cracker Cavern IV cannot be dumped before beating Cracker Cavern II.
- Cavern Depths cannot be dumped before beating Cracker Cavern IV.
- Like the 2018 event, some maps probably have a variant that is only available when your account has a certain FUN value. Those variants share the same map ID as their normal counterpart, so you won't be able to know if you dumped the normal map or the variant.
- There are certainly requirements for other maps that I did not encounter, but might prevent you from dumping those.

## Usage
- Set your username and password in the `main.ts` file.
- Change the UID of the fetch URL in the `map.ts` file.
- If you did not donate (sad :/), change the URL from `turbopacket` to `packet` in the `map.ts` file.
- If you donated, you can try decreasing the `backoff_time` value in the `reset_backoff()` function of the `main.ts` file.
- Run `deno run --allow-net main.ts`