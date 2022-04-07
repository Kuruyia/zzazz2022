import { authenticate } from './auth.ts';
import { play_lottery } from './lottery.ts';
import { Task, TaskType } from './task.ts';

import * as log from 'https://deno.land/std/log/mod.ts';

// Setup the logger
await log.setup({
    handlers: {
        console: new log.handlers.ConsoleHandler("DEBUG", {
            formatter: "[{levelName} / {datetime}] {msg}",
        }),

        file: new log.handlers.FileHandler("INFO", {
            filename: "./logs.log",
            formatter: "[{levelName} / {datetime}] {msg}",
        }),
    },

    loggers: {
        // configure default logger available via short-hand methods above.
        default: {
            level: "DEBUG",
            handlers: ["console", "file"],
        },
    },
});

let logger = log.getLogger();

// Credentials
const LOGIN_USERNAME = 'Kuruyia';
const LOGIN_PASSWORD = '';

// Task list
let task_list: Array<Task> = [];

// Requests stuff
let session_token = '';
let backoff_time: number;
let backoff_retries: number;

// Backoff management
const reset_backoff = (): void => {
    backoff_time = 1;
    backoff_retries = 0;
}

const increase_backoff = (): void => {
    ++backoff_retries;
    backoff_time = Math.exp(backoff_retries);
}

// Task runner
const runner = (): void => {
    // Extract the task
    const current_task: Task | undefined = task_list.shift();

    // If there are no task left, stop the runner
    if (!current_task)
    {
        return;
    }

    // Execute the task
    switch (current_task.type) {
        case TaskType.TASK_AUTHENTICATE:
            // Authenticate the user
            authenticate(LOGIN_USERNAME, LOGIN_PASSWORD)
                .then(token => {
                    // Remember the token
                    logger.info('User authenticated!');
                    session_token = token;

                    // Reschedule the runner
                    reset_backoff();
                    setTimeout(runner, backoff_time * 1000);
                })
                .catch(e => {
                    logger.critical('Unable to authenticate...\n' + e);
                });

            break;

        case TaskType.TASK_PLAY_LOTTERY:
            // Play the lottery
            play_lottery(session_token)
                .then(lottery_data => {
                    // Log the lottery data
                    logger.info('Played the lottery: ' + lottery_data.winning_letters);
                    logger.info('Score: ' + lottery_data.score);

                    // Refresh the token
                    session_token = lottery_data.refresh_token;

                    // Reschedule a play if asked
                    if (current_task.reschedule)
                    {
                        task_list.push({
                            type: TaskType.TASK_WAIT_LONGER,
                            reschedule: false,
                            duration: 60 * 60 * 2,
                        });

                        task_list.push({
                            type: TaskType.TASK_AUTHENTICATE,
                        });

                        task_list.push({
                            type: TaskType.TASK_PLAY_LOTTERY,
                            reschedule: current_task.reschedule,
                        });
                    }

                    reset_backoff();
                })
                .catch(e => {
                    // Log the error
                    logger.error('Unable to play the lottery\n' + e);

                    if (current_task.reschedule) {
                        // Retry playing
                        if (e == 401)
                        {
                            // Schedule an authentication
                            task_list.unshift({
                                type: TaskType.TASK_AUTHENTICATE,
                            });
                        }

                        task_list.push({
                            type: TaskType.TASK_PLAY_LOTTERY,
                            reschedule: current_task.reschedule,
                        });
                    }

                    increase_backoff();
                })
                .finally(() => {
                    if (backoff_retries < 6)
                    {
                        setTimeout(runner, backoff_time * 1000);
                    }
                });

            break;

        case TaskType.TASK_WAIT_LONGER:
            // Wait longer than usual
            logger.info('Waiting for ' + current_task.duration + 's...');

            if (current_task.reschedule) {
                task_list.push(current_task);
            }

            reset_backoff();
            setTimeout(runner, current_task.duration * 1000);

            break;
    }
}

// Init the task list
task_list.push({
    type: TaskType.TASK_AUTHENTICATE,
});

task_list.push({
    type: TaskType.TASK_PLAY_LOTTERY,
    reschedule: true,
});

// Start the runner
reset_backoff();
runner();
