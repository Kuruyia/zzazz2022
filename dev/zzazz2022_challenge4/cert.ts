import { decode, encode } from 'https://deno.land/std@0.133.0/encoding/base64.ts';

/**
 * Data for a certificate that was fetched
 */
export interface FetchedCertificate {
    /**
     * New session token
     */
    refresh_token: string,

    /**
     * Certificate data as a binary string
     */
    cert_data: Uint8Array,
}

/**
 * Fetch a certificate from the server
 */
export const fetch_cert = (data: Uint8Array, session_token: string): Promise<FetchedCertificate> => {
    // Result stuff
    let token: string = session_token;

    // Perform the fetch
    return fetch('https://fools2022.online/packet/4923818', {
        body: encode(data),
        headers: {
            'Content-Type': 'text/plain',
            'User-Agent': 'kuruyia-does-aes/1.0.0',
            'X-Disclaimer': 'thanks for letting us do that <3',
            'X-FoolsSessionToken': session_token,
        },
        method: 'POST',
    })
        .then(response => {
            // if (!response.ok) {
            //     throw response.status;
            // }

            token = (response.headers.get('X-FoolsRefreshToken') || session_token);
            return response.text();
        })
        .then(response => {
            console.log(response)
            console.log(decode(response))
            return {
                refresh_token: token,
                cert_data: decode(response),
            };
        });
}

/**
 * Data for a certificate that was appraised
 */
export interface AppraisedCertificate {
    /**
     * New session token
     */
    refresh_token: string,

    /**
     * Appraisal data as a binary string
     */
    appraise_data: Uint8Array,
}

/**
 * Appraise a certificate with the server
 */
export const appraise_cert = (cert: Uint8Array, session_token: string): Promise<AppraisedCertificate> => {
    // Result stuff
    let token: string = session_token;

    // Get the template data
    let template: Uint8Array = Deno.readFileSync('appraise_template.bin');

    // Populate the template
    for (let i = 0; i < cert.length; ++i) {
        template[0x04 + i] = cert[i];
    }

    // Perform the fetch
    return fetch('https://fools2022.online/packet/4923818', {
        body: encode(template),
        headers: {
            'Content-Type': 'text/plain',
            'User-Agent': 'kuruyia-does-aes/1.0.0',
            'X-Disclaimer': 'thanks for letting us do that <3',
            'X-FoolsSessionToken': session_token,
        },
        method: 'POST',
    })
        .then(response => {
            if (!response.ok) {
                throw response.status;
            }

            token = (response.headers.get('X-FoolsRefreshToken') || session_token);
            return response.text();
        })
        .then(response => {
            return {
                refresh_token: token,
                appraise_data: decode(response),
            };
        });
}
