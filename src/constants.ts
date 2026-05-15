import { config as dotenvConfig } from 'dotenv';
import { resolve } from 'node:path';

import { isInRange } from '@/utils/helpers';

// Process and init .env vars
dotenvConfig();

// Helpers
function validPort(port: string | undefined) {
    if (!port) return 0;
    return isInRange(Number(port), 1, 2**16);
}

// Main configuration
const isProduction =
    process.env['APP_ENV'] === 'production' ||
    process.env['NODE_ENV'] === 'production';

const config = {
    APP: {
        ENV: isProduction ? 'production' : 'development',
        PORT: validPort(process.env['PORT'])
            || validPort(process.env['APP_PORT'])
            || 5500,
        PUBLIC_DIR: resolve(__dirname, '../public'),
    },

    __DEV__: !isProduction,
};

export default config;
