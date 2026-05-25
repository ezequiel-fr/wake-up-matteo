import { Router } from 'express';

import * as alarms from '@/controllers/alarms';
import * as weather from '@/controllers/weather';

const routes = Router();

// Controllers routes
routes.get('/', (_req, res) => {
    return res.send('Hello, world!');
});

// Alarm management route
routes.get('/alarms', alarms.getAlarms);
routes.post('/alarms', alarms.createAlarm);
routes.put('/alarms/:id', alarms.updateAlarm);
routes.delete('/alarms/:id', alarms.deleteAlarm);

// Weather data route
routes.get('/weather', weather.getWeather);
routes.get('/weather/forecast', weather.getWeatherForecast);

export default routes;
