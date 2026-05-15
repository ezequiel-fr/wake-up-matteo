import { config as dotenvConfig } from 'dotenv';

import { isInRange } from '@/utils/helpers';

// Process and init .env vars
dotenvConfig();

// Helpers
function validPort(port: string | undefined) {
    if (!port) return 0;
    return isInRange(Number(port), 1, 2**16);
}

// Main configuration
const config = {
    APP: {
        PORT: validPort(process.env['PORT'])
            || validPort(process.env['APP_PORT'])
            || 5500,
    },
};

export default config;
