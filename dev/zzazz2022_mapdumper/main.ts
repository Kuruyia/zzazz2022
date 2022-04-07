import { authenticate } from './auth.ts';
import { fetch_map } from './map.ts';
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

        case TaskType.TASK_FETCH_MAP:
            // Fetch the specified map
            const map_id_str: string = current_task.map_id.toString(16).padStart(4, '0');

            fetch_map(current_task.map_id, session_token)
                .then(fetched_map => {
                    // Save the map data
                    logger.info('Fetched map 0x' + map_id_str);
                    Deno.writeFileSync('dumps/' + map_id_str + '.bin',
                        fetched_map.map_data,
                        {
                            append: false,
                            create: true,
                        });

                    // Remember the map ID
                    Deno.writeTextFileSync('dumps/last.txt', current_task.map_id.toString());

                    // Refresh the token
                    session_token = fetched_map.refresh_token;

                    // Fetch the next map
                    if (current_task.map_id < 0xFFFF && current_task.continue_with_next)
                    {
                        task_list.push({
                            type: TaskType.TASK_FETCH_MAP,
                            map_id: current_task.map_id + 1,
                            retry_count: 0,
                            continue_with_next: current_task.continue_with_next,
                        });
                    }

                    reset_backoff();
                })
                .catch(e => {
                    // Log the error
                    logger.error('Unable to fetch map 0x' + map_id_str + '\n' + e);

                    if (current_task.retry_count < 3) {
                        // Retry the fetch
                        if (e == 401)
                        {
                            // Schedule an authentication
                            task_list.unshift({
                                type: TaskType.TASK_AUTHENTICATE,
                            });
                        }

                        task_list.push({
                            type: TaskType.TASK_FETCH_MAP,
                            map_id: current_task.map_id,
                            retry_count: current_task.retry_count + 1,
                            continue_with_next: current_task.continue_with_next,
                        });
                    } else {
                        // Skip this map
                        logger.warning('Skipping map 0x' + map_id_str + '...');

                        if (current_task.map_id < 0xFFFF && current_task.continue_with_next) {
                            task_list.push({
                                type: TaskType.TASK_FETCH_MAP,
                                map_id: current_task.map_id + 1,
                                retry_count: 0,
                                continue_with_next: current_task.continue_with_next,
                            });
                        }
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

// Create the dump directory
Deno.mkdirSync('dumps', {recursive: true});

// Check where we left off
let last_map_id = 0;

try {
    last_map_id = parseInt(Deno.readTextFileSync('dumps/last.txt'));
} catch (_e) {
    logger.info('Could not find where we left off');
}

// Init the task list
task_list.push({
    type: TaskType.TASK_AUTHENTICATE,
});

task_list.push({
    type: TaskType.TASK_FETCH_MAP,
    map_id: last_map_id,
    retry_count: 0,
    continue_with_next: true,
});

// Start the runner
reset_backoff();
runner();
