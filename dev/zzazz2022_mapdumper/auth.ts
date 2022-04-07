import { LoginRequest, LoginResponse } from './request_types.ts';

/**
 * Authenticate with the server using the specified credentials
 */
export const authenticate = (username: string, password: string): Promise<string> => {
    const login_request: LoginRequest = {
        u: username,
        p: password,
    };

    return fetch('https://fools2022.online/login', {
        body: JSON.stringify(login_request),
        headers: {
            'Content-Type': 'application/json',
            'User-Agent': 'kuruyia-dumps-maps/1.0.0',
            'X-Disclaimer': 'thanks for letting us do that <3',
        },
        method: 'POST',
    })
    .then(response => {
        if (!response.ok) {
            throw new Error(`HTTP Error, status = ${response.status}`);
        }

        return response.json();
    })
    .then((login_response: LoginResponse) => {
        if (login_response.error != 0) {
            throw new Error(`Login error: = ${login_response.error}`);
        }

        return login_response.data.session;
    });
};
