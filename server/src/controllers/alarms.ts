import { RequestHandler } from 'express';

export const getAlarms: RequestHandler = (req, res) => {
    try {
        console.log(req.url, req.method);

        return res.sendStatus(200);
    } catch (err) {
        console.error('Error occurred while fetching alarms:', err);
        return res.sendStatus(500);
    }
};

export const createAlarm: RequestHandler = (req, res) => {
    try {
        console.log(req.url, req.method);

        return res.sendStatus(200);
    } catch (err) {
        console.error('Error occurred while creating alarm:', err);
        return res.sendStatus(500);
    }
};

export const updateAlarm: RequestHandler = (req, res) => {
    try {
        console.log(req.url, req.method);

        return res.sendStatus(200);
    } catch (err) {
        console.error('Error occurred while updating alarm:', err);
        return res.sendStatus(500);
    }
};

export const deleteAlarm: RequestHandler = (req, res) => {
    try {
        console.log(req.url, req.method);

        return res.sendStatus(200);
    } catch (err) {
        console.error('Error occurred while deleting alarm:', err);
        return res.sendStatus(500);
    }
};
