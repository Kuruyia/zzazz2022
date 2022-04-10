# Directory - dev/zzazz2022_challenge4
This directory contains the TypeScript script that can make requests to the endpoints that get a certificate and appraise it.

Before using this script for getting a certificate, you must dump the data of a request to the certificate generation endpoint from the HTML5 client, decode it from base64, and save it in a file named `req.bin`.

Before using this script for appraising a certificate, you must dump the data of a request to the appraisal endpoint from the HTML5 client, decode it from base64, and save it in a file named `appraise_template.bin`.

## Usage
- Set your username and password in the `main.ts` file.
- Change the UID of the fetch URLs in the `cert.ts` file.
- Create the `req.bin` and/or `appraise_template.bin` files as described before.
- Run `deno run --allow-net --allow-read --allow-write main.ts`