import { Router } from 'express';

import * as alarms from '@/controllers/alarms';

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
routes.get('/weather', (_req, res) => {
    return res.send('Weather data');
});

export default routes;
