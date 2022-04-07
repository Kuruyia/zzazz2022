/**
 * Enumeration of the different types of task
 */
export enum TaskType {
    TASK_AUTHENTICATE,
    TASK_PLAY_LOTTERY,
    TASK_WAIT_LONGER,
}

/**
 * Authenticate task
 */
export interface AuthenticateTask {
    type: TaskType.TASK_AUTHENTICATE,
}

/**
 * Play lottery task
 */
export interface PlayLotteryTask {
    type: TaskType.TASK_PLAY_LOTTERY,
    reschedule: boolean,
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
export type Task = AuthenticateTask | PlayLotteryTask | WaitLongerTask;
