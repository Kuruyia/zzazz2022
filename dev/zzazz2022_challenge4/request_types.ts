// Login request parameters
export interface LoginRequest {
    /**
     * Username
     */
    u: string,

    /**
     * Password
     */
    p: string,
}

// Data of the response to a login request
export interface LoginResponseData {
    /**
     * Session token
     */
    session: string,

    /**
     * ID of the user
     */
    uid: number,

    /**
     * Scope of the session token
     */
    scope: string,
}

// Response to a login request
export interface LoginResponse {
    /**
     * Error code
     */
    error: number,

    /**
     * Data of the login response
     */
    data: LoginResponseData,
}
