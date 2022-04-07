import { authenticate } from './auth.ts';
import { appraise_cert, fetch_cert } from './cert.ts';
import { Task, TaskType } from './task.ts';

import * as log from 'https://deno.land/std/log/mod.ts';
import { decode, encode } from 'https://deno.land/std@0.133.0/encoding/base64.ts';

// Setup the logger
await log.setup({
    handlers: {
        console: new log.handlers.ConsoleHandler("DEBUG", {
            formatter: "[{levelName} / {datetime}] {msg}",
        }),
    },

    loggers: {
        // configure default logger available via short-hand methods above.
        default: {
            level: "DEBUG",
            handlers: ["console"],
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

        case TaskType.TASK_GET_CERT:
            fetch_cert(current_task.data, session_token)
                .then(fetched_cert => {
                    // Save the certificate
                    logger.info('Got the certificate.');
                    // Deno.writeFileSync('cert.bin', fetched_cert.cert_data);

                    session_token = fetched_cert.refresh_token;

                    // Appraise the cert
                    task_list.push({
                        type: TaskType.TASK_VERIFY_CERT,
                        data: Deno.readFileSync('cert.bin'),
                    });

                    // Reschedule the runner
                    reset_backoff();
                    setTimeout(runner, backoff_time * 1000);
                })
                .catch(e => {
                    // Log the error
                    logger.error('Unable to fetch the certificate.\n' + e);
                });

            break;

        case TaskType.TASK_VERIFY_CERT:
            appraise_cert(current_task.data, session_token)
                .then(fetched_cert => {
                    // Save the appraisal data
                    logger.info('Appraised the certificate.');
                    Deno.writeFileSync('appraisal.bin', fetched_cert.appraise_data);
                    console.log(new TextDecoder('ascii').decode(fetched_cert.appraise_data));

                    session_token = fetched_cert.refresh_token;

                    // Reschedule the runner
                    reset_backoff();
                    setTimeout(runner, backoff_time * 1000);
                })
                .catch(e => {
                    // Log the error
                    logger.error('Unable to appraise the certificate.\n' + e);
                });

            break;
    }
}

// Load the request from the file
const req_data: Uint8Array = Deno.readFileSync('req.bin');

// Init the task list
task_list.push({
    type: TaskType.TASK_AUTHENTICATE,
});

task_list.push({
    type: TaskType.TASK_GET_CERT,
    data: req_data,
});

// Start the runner
reset_backoff();
runner();
