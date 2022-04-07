/**
 * Enumeration of the different types of task
 */
export enum TaskType {
    TASK_AUTHENTICATE,
    TASK_GET_CERT,
    TASK_VERIFY_CERT,
}

/**
 * Authenticate task
 */
export interface AuthenticateTask {
    type: TaskType.TASK_AUTHENTICATE,
}

/**
 * Get the certificate task
 */
export interface GetCertTask {
    type: TaskType.TASK_GET_CERT,
    data: Uint8Array,
}

/**
 * Verify the certificate task
 */
export interface VerifyCertTask {
    type: TaskType.TASK_VERIFY_CERT,
    data: Uint8Array,
}

/**
 * Task union
 */
export type Task = AuthenticateTask | GetCertTask | VerifyCertTask;
