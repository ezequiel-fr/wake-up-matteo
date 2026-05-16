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

function getMongoURI(): string {
    // Check first if the user provided the credentials to a standard or a localhost connection
    if (process.env['USER'] && process.env['PASSWORD']) {
        return `mongodb+srv://${process.env['USER']}:${process.env['PASSWORD']}@`
            + `${process.env['HOST']}/${process.env['DB']}`;
    } else if (process.env['URI'] && process.env['URI'].trim() !== '') {
        return process.env['URI'];
    } else {
        return `mongodb://${process.env['HOST']}/${process.env['DB']}`;
    }
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

    MONGO: {
        USER: process.env['MONGO_USER'] || "",
        PASSWORD: process.env['MONGO_PASSWORD'] || "",
        HOST: process.env['MONGO_HOST'] || "",
        DB: process.env['MONGO_DB'] || "",
        URI: process.env['MONGO_URI'] || getMongoURI(),
    },

    __DEV__: !isProduction,
};

export default config;
