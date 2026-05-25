import axios from 'axios';
import { RequestHandler } from 'express';

import { WeatherCodes } from '../constants';

const cache: Record<string, any> = {};

const weatherClient = axios.create({
    baseURL: 'https://api.open-meteo.com/v1/',
    timeout: 5000,
});

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
                current: 'temperature_2m,weather_code,wind_speed_10m',
                daily: 'sunrise',
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
        const place = await resolveLocation('Châlons-en-Champagne');
        const weatherData = await fetchCurrentWeather(place);

        const wc =
            WeatherCodes[weatherData.current.weather_code] ||
            [-1, 'unknown', 'unknown'];

        if (new Date().getHours() >= 18 || new Date().getHours() < 6) {
            weatherData.current.weather_string_code = wc[2];
        } else {
            weatherData.current.weather_string_code = wc[1];
        }

        res.json(weatherData);
    } catch (err) {
        console.error('Error fetching weather data:', err);
        res.status(500).json({ error: 'Failed to fetch weather data' });
    }
};

export const getWeatherForecast: RequestHandler = async (_req, res) => {
    try {
        const place = await resolveLocation('Châlons-en-Champagne');
        const forecastData = await weatherForecast(place);

        res.json(forecastData);
    } catch (err) {
        console.error('Error fetching weather forecast:', err);
        res.status(500).json({ error: 'Failed to fetch weather forecast' });
    }
};
