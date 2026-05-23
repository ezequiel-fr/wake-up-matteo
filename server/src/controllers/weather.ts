import axios from 'axios';

const weatherClient = axios.create({
    baseURL: 'https://api.open-meteo.com/v1/forecast',
    timeout: 5000,
});


console.log(weatherClient);
