# Directory - dev/zzazz2022_autolottery
This directory contains the TypeScript script that plays the lottery automatically for you.

Just run it and it will ping the server every 2 hours, so you can hopefully get anything more than a triple lottery match.

## Usage
- Set your username and password in the `main.ts` file.
- Change the UID of the fetch URL in the `lottery.ts` file.
- Run `deno run --allow-net main.ts`