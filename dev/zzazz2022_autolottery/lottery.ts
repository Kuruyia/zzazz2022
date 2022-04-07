import { CHARCODES } from './utils.ts';

import { decode, encode } from 'https://deno.land/std@0.133.0/encoding/base64.ts';

/**
 * Data for a lottery play
 */
export interface LotteryData {
    /**
     * New session token
     */
    refresh_token: string,

    /**
     * Winning letters
     * In the form "A, B, C, D, E"
     */
    winning_letters: string,

    /**
     * Score
     * Either "single", "double", "triple", "quadruple" or "quintuple"
     */
    score: string,
}

/**
 * Play the lottery
 */
export const play_lottery = (session_token: string): Promise<LotteryData> => {
    // Result stuff
    let token: string = session_token;

    // Perform the fetch
    return fetch('https://fools2022.online/packet/4923818', {
        body: 'BbW2WXMaAgg=',
        headers: {
            'Content-Type': 'text/plain',
            'User-Agent': 'kuruyia-plays-lottery/1.0.0',
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
            // Decode the response
            const decoded_response: Uint8Array = decode(response);
            let winning_letters_raw: Uint8Array = decoded_response.slice(0x00, 0x0F);
            let score_raw: Uint8Array = decoded_response.slice(0x10, 0x1F);

            // Remove the 0xFF characters
            winning_letters_raw = winning_letters_raw.slice(0x00, winning_letters_raw.findIndex(k => k == 0xFF));
            score_raw = score_raw.slice(0x00, score_raw.findIndex(k => k == 0xFF));

            // Convert to ASCII
            let winning_letters = ''
            let score = ''

            for (const n of winning_letters_raw) {
                winning_letters += (CHARCODES.get(n) || '?');
            }

            for (const n of score_raw) {
                score += (CHARCODES.get(n) || '?');
            }

            return {
                refresh_token: token,
                winning_letters: winning_letters,
                score: score,
            };
        });
}
