import { decode, encode } from 'https://deno.land/std@0.133.0/encoding/base64.ts';

/**
 * Data for a map that was fetched
 */
export interface FetchedMap {
    /**
     * New session token
     */
    refresh_token: string,

    /**
     * Map data as a binary string
     */
    map_data: Uint8Array,
}

/**
 * Fetch a map from the server
 */
export const fetch_map = (map_id: number, session_token: string): Promise<FetchedMap> => {
    // Result stuff
    let token: string = session_token;

    // Encode the map ID
    const raw_body: Uint8Array = new Uint8Array([0x01, 0x00, 0x00, 0x00, (map_id >> 8) & 0xFF, map_id & 0xFF, 0x00, 0x00]);

    // Perform the fetch
    return fetch('https://fools2022.online/turbopacket/4923818', {
                body: encode(raw_body),
                headers: {
                    'Content-Type': 'text/plain',
                    'User-Agent': 'kuruyia-dumps-maps/1.0.0',
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
                    map_data: decode(response),
                };
            });
}
