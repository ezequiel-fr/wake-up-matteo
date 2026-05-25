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

export enum WeatherIcon {
   CLEAR_DAY = 'clear-day',
   CLEAR_NIGHT = 'clear-night',
   CLOUDY_1_DAY = 'cloudy-1-day',
   CLOUDY_1_NIGHT = 'cloudy-1-night',
   CLOUDY_2_DAY = 'cloudy-2-day',
   CLOUDY_2_NIGHT = 'cloudy-2-night',
   CLOUDY_3_DAY = 'cloudy-3-day',
   CLOUDY_3_NIGHT = 'cloudy-3-night',
//    CLOUDY = 'cloudy',
//    DUST = 'dust',
   FOG_DAY = 'fog-day',
   FOG_NIGHT = 'fog-night',
//    FOG = 'fog',
   FROST_DAY = 'frost-day',
   FROST_NIGHT = 'frost-night',
//    FROST = 'frost',
//    HAIL = 'hail',
//    HAZE_DAY = 'haze-day',
//    HAZE_NIGHT = 'haze-night',
//    HAZE = 'haze',
//    HURRICANE = 'hurricane',
   ISOLATED_THUNDERSTORMS_DAY = 'isolated-thunderstorms-day',
   ISOLATED_THUNDERSTORMS_NIGHT = 'isolated-thunderstorms-night',
//    ISOLATED_THUNDERSTORMS = 'isolated-thunderstorms',
//    RAIN_AND_SLEET_MIX = 'rain-and-sleet-mix',
//    RAIN_AND_SNOW_MIX = 'rain-and-snow-mix',
   RAINY_1_DAY = 'rainy-1-day',
   RAINY_1_NIGHT = 'rainy-1-night',
//    RAINY_1 = 'rainy-1',
   RAINY_2_DAY = 'rainy-2-day',
   RAINY_2_NIGHT = 'rainy-2-night',
//    RAINY_2 = 'rainy-2',
   RAINY_3_DAY = 'rainy-3-day',
   RAINY_3_NIGHT = 'rainy-3-night',
//    RAINY_3 = 'rainy-3',
   SCATTERED_THUNDERSTORMS_DAY = 'scattered-thunderstorms-day',
   SCATTERED_THUNDERSTORMS_NIGHT = 'scattered-thunderstorms-night',
//    SCATTERED_THUNDERSTORMS = 'scattered-thunderstorms',
   SEVERE_THUNDERSTORM = 'severe-thunderstorm',
//    SNOW_AND_SLEET_MIX = 'snow-and-sleet-mix',
   SNOWY_1_DAY = 'snowy-1-day',
   SNOWY_1_NIGHT = 'snowy-1-night',
//    SNOWY_1 = 'snowy-1',
   SNOWY_2_DAY = 'snowy-2-day',
   SNOWY_2_NIGHT = 'snowy-2-night',
//    SNOWY_2 = 'snowy-2',
   SNOWY_3_DAY = 'snowy-3-day',
   SNOWY_3_NIGHT = 'snowy-3-night',
//    SNOWY_3 = 'snowy-3',
//    THUNDERSTORMS = 'thunderstorms',
//    TORNADO = 'tornado',
//    TROPICAL_STORM = 'tropical-storm',
//    WIND = 'wind'
}

export const WeatherCodes = [
    [0, WeatherIcon.CLEAR_DAY, WeatherIcon.CLEAR_NIGHT],
    [1, WeatherIcon.CLOUDY_1_DAY, WeatherIcon.CLOUDY_1_NIGHT],
    [2, WeatherIcon.CLOUDY_2_DAY, WeatherIcon.CLOUDY_2_NIGHT],
    [3, WeatherIcon.CLOUDY_3_DAY, WeatherIcon.CLOUDY_3_NIGHT],
    [45, WeatherIcon.FOG_DAY, WeatherIcon.FOG_NIGHT],
    [48, WeatherIcon.FROST_DAY, WeatherIcon.FROST_NIGHT],
    [51, WeatherIcon.RAINY_1_DAY, WeatherIcon.RAINY_1_NIGHT],
    [53, WeatherIcon.RAINY_2_DAY, WeatherIcon.RAINY_2_NIGHT],
    [55, WeatherIcon.RAINY_3_DAY, WeatherIcon.RAINY_3_NIGHT],
    [56, WeatherIcon.FROST_DAY, WeatherIcon.FROST_NIGHT],
    [57, WeatherIcon.FROST_DAY, WeatherIcon.FROST_NIGHT],
    [61, WeatherIcon.RAINY_1_DAY, WeatherIcon.RAINY_1_NIGHT],
    [63, WeatherIcon.RAINY_2_DAY, WeatherIcon.RAINY_2_NIGHT],
    [65, WeatherIcon.RAINY_3_DAY, WeatherIcon.RAINY_3_NIGHT],
    [66, WeatherIcon.FROST_DAY, WeatherIcon.FROST_NIGHT],
    [67, WeatherIcon.FROST_DAY, WeatherIcon.FROST_NIGHT],
    [71, WeatherIcon.SNOWY_1_DAY, WeatherIcon.SNOWY_1_NIGHT],
    [73, WeatherIcon.SNOWY_2_DAY, WeatherIcon.SNOWY_2_NIGHT],
    [75, WeatherIcon.SNOWY_3_DAY, WeatherIcon.SNOWY_3_NIGHT],
    [77, WeatherIcon.SNOWY_1_DAY, WeatherIcon.SNOWY_1_NIGHT],
    [80, WeatherIcon.RAINY_1_DAY, WeatherIcon.RAINY_1_NIGHT],
    [81, WeatherIcon.RAINY_2_DAY, WeatherIcon.RAINY_2_NIGHT],
    [82, WeatherIcon.RAINY_3_DAY, WeatherIcon.RAINY_3_NIGHT],
    [85, WeatherIcon.SNOWY_1_DAY, WeatherIcon.SNOWY_1_NIGHT],
    [86, WeatherIcon.SNOWY_2_DAY, WeatherIcon.SNOWY_2_NIGHT],
    [
        95,
        WeatherIcon.ISOLATED_THUNDERSTORMS_DAY,
        WeatherIcon.ISOLATED_THUNDERSTORMS_NIGHT,
    ],
    [
        96,
        WeatherIcon.SCATTERED_THUNDERSTORMS_DAY,
        WeatherIcon.SCATTERED_THUNDERSTORMS_NIGHT,
    ],
    [99, WeatherIcon.SEVERE_THUNDERSTORM, WeatherIcon.SEVERE_THUNDERSTORM],
] as [number, WeatherIcon, WeatherIcon][];

export type Config = typeof config;
export default config;
