/**
 * Enumeration of the different types of task
 */
export enum TaskType {
    TASK_AUTHENTICATE,
    TASK_FETCH_MAP,
    TASK_WAIT_LONGER,
}

/**
 * Authenticate task
 */
export interface AuthenticateTask {
    type: TaskType.TASK_AUTHENTICATE,
}

/**
 * Fetch map task
 */
export interface FetchMapTask {
    type: TaskType.TASK_FETCH_MAP,
    map_id: number;
    retry_count: number;
    continue_with_next: boolean;
}

/**
 * Wait longer task
 */
export interface WaitLongerTask {
    type: TaskType.TASK_WAIT_LONGER,
    duration: number,
    reschedule: boolean,
}

/**
 * Task union
 */
export type Task = AuthenticateTask | FetchMapTask | WaitLongerTask;
