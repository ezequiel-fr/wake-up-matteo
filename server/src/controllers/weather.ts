import axios from 'axios';
import { RequestHandler } from 'express';

import { WeatherCodes } from '../constants';

const cache: Record<string, any> = {};

const weatherClient = axios.create({
    baseURL: 'https://api.open-meteo.com/v1/',
    timeout: 5000,
});

const parseWeatherDateTime = (value?: string) => {
    if (!value) return null;

    const match = value.match(
        /^(\d{4})-(\d{2})-(\d{2})[T ](\d{2}):(\d{2})(?::(\d{2}))?$/,
    );

    if (!match) return null;

    const [, year, month, day, hour, minute, second = '0'] = match;

    return Date.UTC(
        Number(year),
        Number(month) - 1,
        Number(day),
        Number(hour),
        Number(minute),
        Number(second),
    );
};

const resolveWeatherStringCode = (
    weatherCode: number,
    currentTime?: string,
    sunrise?: string,
    sunset?: string,
    isDayFallback = false,
) => {
    const wc = WeatherCodes[weatherCode] || [-1, 'unknown', 'unknown'];

    const currentTimeValue = parseWeatherDateTime(currentTime);
    const sunriseValue = parseWeatherDateTime(sunrise);
    const sunsetValue = parseWeatherDateTime(sunset);

    const isDay =
        currentTimeValue !== null &&
        sunriseValue !== null &&
        sunsetValue !== null
            ? currentTimeValue >= sunriseValue && currentTimeValue < sunsetValue
            : isDayFallback;

    return isDay ? wc[1] : wc[2];
};

const resolveLocation = async (query: string) => {
    try {
        const res = await axios.get('https://geocoding-api.open-meteo.com/v1/search', {
            params: {
                count: 1,
                format: 'json',
                language: 'fr',
                name: query,
            },
        });
        if (!res.data.results?.length) throw new Error('Location not found');

        return res.data.results?.[0];
    } catch (err) {
        console.error('Error resolving location:', err);
        throw new Error('Failed to resolve location');
    }
};

const fetchCurrentWeather = async (place: any) => {
    try {
        const key = `${place.latitude},${place.longitude}`;

        // return cached if not expired
        const entry = cache[key];
        if (entry && entry.expires > Date.now()) {
            return entry.data;
        }

        // get current weather data
        const { data } = await weatherClient.get('/forecast', {
            params: {
                latitude: place.latitude,
                longitude: place.longitude,
                current: 'temperature_2m,weather_code,wind_speed_10m,is_day',
                daily: 'sunrise,sunset',
                forecast_days: 1,
                timezone: 'auto',
            },
        });

        // cache for 15 minutes
        cache[key] = {
            data,
            expires: Date.now() + 15 * 60 * 1000,
        };

        return data;
    } catch (err) {
        console.error('Error fetching weather data:', err);
        throw new Error('Failed to fetch weather data');
    }
}

const weatherForecast = async (place: any) => {
    try {
        // forecast for 7 days
        const { data } = await weatherClient.get('/forecast', {
            params: {
                latitude: place.latitude,
                longitude: place.longitude,
                daily: 'temperature_2m_max,temperature_2m_min,precipitation_sum',
                timezone: 'Europe/Paris',
                forecast_days: 7,
            },
        });

        return data;
    } catch (err) {
        console.error('Error fetching weather forecast:', err);
        throw new Error('Failed to fetch weather forecast');
    }
};

export const getWeather: RequestHandler = async (_req, res) => {
    try {
        const place = await resolveLocation('Nancy');
        const weatherData = await fetchCurrentWeather(place);

        weatherData.current.weather_string_code = resolveWeatherStringCode(
            weatherData.current.weather_code,
            weatherData.current.time,
            weatherData.daily?.sunrise?.[0],
            weatherData.daily?.sunset?.[0],
            Boolean(weatherData.current.is_day),
        );

        res.json(weatherData);
    } catch (err) {
        console.error('Error fetching weather data:', err);
        res.status(500).json({ error: 'Failed to fetch weather data' });
    }
};

export const getWeatherForecast: RequestHandler = async (_req, res) => {
    try {
        const place = await resolveLocation('Nancy');
        const forecastData = await weatherForecast(place);

        res.json(forecastData);
    } catch (err) {
        console.error('Error fetching weather forecast:', err);
        res.status(500).json({ error: 'Failed to fetch weather forecast' });
    }
};
