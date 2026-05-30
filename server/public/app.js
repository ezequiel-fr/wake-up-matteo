// Init global URI
const uri = new URL(location.toString());

uri.pathname = '/';
uri.protocol = location.protocol === 'https:' ? 'wss:' : 'ws:';

const maxReconnectDelay = 20e3;
const storageKey = 'wake-up-matteo-admin';
// const defaultLocation = 'Lille, Hauts-de-France, France';
const defaultLocation = 'Paris, France';
const dayNames = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];
const weatherCodes = new Map([
    [0, 'Clear sky'],
    [1, 'Mostly clear'],
    [2, 'Partly cloudy'],
    [3, 'Cloudy'],
    [45, 'Fog'],
    [48, 'Icy fog'],
    [51, 'Light drizzle'],
    [53, 'Drizzle'],
    [55, 'Dense drizzle'],
    [56, 'Low icy drizzle'],
    [57, 'Icy drizzle'],
    [61, 'Light rain'],
    [63, 'Rain'],
    [65, 'Heavy rain'],
    [66, 'Light icy rain'],
    [67, 'Icy rain'],
    [71, 'Light snow'],
    [73, 'Snow'],
    [75, 'Heavy snow'],
    [77, 'Snow grains'],
    [80, 'Light showers'],
    [81, 'Showers'],
    [82, 'Heavy showers'],
    [85, 'Light snow showers'],
    [86, 'Snow showers'],
    [95, 'Thunderstorm'],
    [96, 'Thunderstorm with light hail'],
    [99, 'Thunderstorm with heavy hail'],
]);

/** @type {WebSocket} */
let ws;
/** @type {number} */
let reconnectAttempt = 0;
/** @type {number | undefined} */
let reconnectTimer;
/** @type {number | undefined} */
let heartbeatTimer;

const state = loadState();

const elements = {
    wsStatus: document.querySelector('#wsStatus'),
    wsStatusDot: document.querySelector('#wsStatusDot'),
    locationForm: document.querySelector('#locationForm'),
    locationInput: document.querySelector('#locationInput'),
    locationName: document.querySelector('#locationName'),
    locationFeedback: document.querySelector('#locationFeedback'),
    localTime: document.querySelector('#localTime'),
    localDate: document.querySelector('#localDate'),
    temperature: document.querySelector('#temperature'),
    weatherLabel: document.querySelector('#weatherLabel'),
    wind: document.querySelector('#wind'),
    sunrise: document.querySelector('#sunrise'),
    sunset: document.querySelector('#sunset'),
    timezone: document.querySelector('#timezone'),
    alarmForm: document.querySelector('#alarmForm'),
    alarmTime: document.querySelector('#alarmTime'),
    alarmLabel: document.querySelector('#alarmLabel'),
    weatherAware: document.querySelector('#weatherAware'),
    alarmEnabled: document.querySelector('#alarmEnabled'),
    dayPicker: document.querySelector('#dayPicker'),
    alarmsList: document.querySelector('#alarmsList'),
    alarmTemplate: document.querySelector('#alarmTemplate'),
    clearAlarms: document.querySelector('#clearAlarms'),
    nextAlarm: document.querySelector('#nextAlarm')
};

function loadState() {
    try {
        return {
            location: defaultLocation,
            timezone: Intl.DateTimeFormat().resolvedOptions().timeZone,
            alarms: [],
            lastTriggered: {},
            ...JSON.parse(localStorage.getItem(storageKey) || '{}')
        };
    } catch (_err) {
        return {
            location: defaultLocation,
            timezone: Intl.DateTimeFormat().resolvedOptions().timeZone,
            alarms: [],
            lastTriggered: {}
        };
    }
}

function saveState() {
    localStorage.setItem(storageKey, JSON.stringify(state));
}

function getReconnectDelay() {
    const delay = Math.min(500 * 2 ** reconnectAttempt, maxReconnectDelay);
    reconnectAttempt += 1;

    return delay;
}

function setConnectionStatus(status) {
    const isOnline = status === 'Online';

    elements.wsStatus.textContent = status;
    elements.wsStatusDot.className = `h-2.5 w-2.5 rounded-full ${isOnline ? 'bg-moss' : 'bg-coral'}`;
}

function clearHeartbeat() {
    clearInterval(heartbeatTimer);
    heartbeatTimer = undefined;
}

function startHeartbeat(socket) {
    clearHeartbeat();

    heartbeatTimer = setInterval(() => {
        if (socket.readyState === WebSocket.OPEN) socket.send('ping');
    }, 25000);
}

function scheduleReconnect() {
    if (reconnectTimer) return;

    const delay = getReconnectDelay();
    setConnectionStatus(`Retrying in ${Math.ceil(delay / 1000)}s`);

    reconnectTimer = setTimeout(() => {
        reconnectTimer = undefined;
        createConnection();
    }, delay);
}

function handleMessage(data) {
    if (data === 'pong') return;

    try {
        const message = JSON.parse(data);
        if (message.type === 'dev:reload') location.reload();
    } catch (_err) {
        console.log(data);
    }
}

function createConnection() {
    setConnectionStatus('Connecting');
    ws = new WebSocket(uri);

    ws.onopen = function () {
        reconnectAttempt = 0;
        setConnectionStatus('Online');
        ws.send('ping');
        startHeartbeat(ws);
    };

    ws.onmessage = ev => handleMessage(ev.data);

    ws.onclose = () => {
        clearHeartbeat();
        scheduleReconnect();
    };

    ws.onerror = () => {
        ws.close();
    };
}

function formatInTimezone(date, options) {
    return new Intl.DateTimeFormat('en-GB', {
        timeZone: state.timezone,
        ...options
    }).format(date);
}

function updateClock() {
    const now = new Date();

    elements.localTime.textContent = formatInTimezone(now, {
        hour: '2-digit',
        minute: '2-digit',
        second: '2-digit'
    });
    elements.localDate.textContent = formatInTimezone(now, {
        weekday: 'long',
        day: '2-digit',
        month: 'long',
        year: 'numeric'
    });
}

function setFeedback(message, isError = false) {
    elements.locationFeedback.textContent = message;
    elements.locationFeedback.className = `min-h-6 pt-3 text-sm ${isError ? 'text-coral' : 'text-ink/60'}`;
}

async function fetchWeather(location) {
    const params = new URLSearchParams({
        location,
    });

    const response = await fetch(`/api/weather?${params}`);
    if (!response.ok) throw new Error('Weather lookup failed.');

    return response.json();
}

function formatPlace(place) {
    return [place.name, place.admin1, place.country].filter(Boolean).join(', ');
}

function renderWeather(place, weather) {
    state.location = formatPlace(place);
    state.timezone = weather.timezone || state.timezone;

    saveState();

    elements.locationName.textContent = state.location;
    elements.locationInput.value = state.location;
    elements.temperature.textContent = `${Math.round(weather.current.temperature_2m)}°`;
    elements.weatherLabel.textContent = weatherCodes.get(weather.current.weather_code) || 'Live weather';
    elements.wind.textContent = Math.round(weather.current.wind_speed_10m);
    elements.sunrise.textContent = new Date(weather.daily.sunrise[0]).toLocaleTimeString('en-GB', {
        hour: '2-digit',
        minute: '2-digit'
    });
    elements.sunset.textContent = new Date(weather.daily.sunset[0]).toLocaleTimeString('en-GB', {
        hour: '2-digit',
        minute: '2-digit'
    });
    elements.timezone.textContent = state.timezone;

    setFeedback('Live data updated.');
    updateClock();
}

async function updateLocation(query) {
    const location = query.trim();
    if (!location) return;

    setFeedback('Updating live data...');

    try {
        const weather = await fetchWeather(location);
        renderWeather({ name: location }, weather);
    } catch (err) {
        setFeedback(err.message || 'Unable to update this location.', true);
    }
}

function getSelectedDays() {
    return [...elements.dayPicker.querySelectorAll('input:checked')].map(input => Number(input.value));
}

function createAlarm(form) {
    const days = getSelectedDays();

    return {
        id: crypto.randomUUID(),
        time: elements.alarmTime.value,
        label: elements.alarmLabel.value.trim() || 'Alarm',
        days: days.length ? days : [0, 1, 2, 3, 4, 5, 6],
        weatherAware: elements.weatherAware.checked,
        enabled: elements.alarmEnabled.checked,
        createdAt: new Date().toISOString()
    };
}

function renderAlarms() {
    elements.alarmsList.innerHTML = '';

    if (!state.alarms.length) {
        elements.alarmsList.innerHTML = '<p class="rounded-md border border-dashed border-ink/20 p-4 text-sm text-ink/55">No alarms configured yet.</p>';
        elements.nextAlarm.textContent = 'None';

        return;
    }

    state.alarms
        .slice()
        .sort((a, b) => a.time.localeCompare(b.time))
        .forEach(alarm => {
            const node = elements.alarmTemplate.content.firstElementChild.cloneNode(true);
            node.dataset.id = alarm.id;
            node.querySelector('[data-field="time"]').textContent = alarm.time;
            node.querySelector('[data-field="days"]').textContent = alarm.days.map(day => dayNames[day]).join(' ');
            node.querySelector('[data-field="label"]').textContent = alarm.label;
            node.querySelector('[data-field="meta"]').textContent = `${alarm.enabled ? 'Enabled' : 'Paused'}${alarm.weatherAware ? ' · Weather aware' : ''}`;
            node.classList.toggle('opacity-55', !alarm.enabled);
            node.querySelector('[data-action="toggle"]').textContent = alarm.enabled ? 'Pause' : 'Enable';
            elements.alarmsList.appendChild(node);
        });

    updateNextAlarm();
}

function updateNextAlarm() {
    const enabled = state.alarms.filter(alarm => alarm.enabled);
    if (!enabled.length) return elements.nextAlarm.textContent = 'None';

    const now = new Date();
    const today = Number(formatInTimezone(now, { weekday: 'short' })
        .replace('Sun', '0')
        .replace('Mon', '1')
        .replace('Tue', '2')
        .replace('Wed', '3')
        .replace('Thu', '4')
        .replace('Fri', '5')
        .replace('Sat', '6'));
    const currentTime = formatInTimezone(now, { hour: '2-digit', minute: '2-digit' });
    const nextToday = enabled
        .filter(alarm => alarm.days.includes(today) && alarm.time >= currentTime)
        .sort((a, b) => a.time.localeCompare(b.time))[0];

    elements.nextAlarm.textContent = nextToday ? `${nextToday.time} · ${nextToday.label}` : 'Tomorrow';
}

function playAlarm() {
    const context = new AudioContext();
    const oscillator = context.createOscillator();
    const gain = context.createGain();

    oscillator.type = 'sine';
    oscillator.frequency.value = 880;

    gain.gain.setValueAtTime(0.001, context.currentTime);
    gain.gain.exponentialRampToValueAtTime(0.25, context.currentTime + 0.05);
    gain.gain.exponentialRampToValueAtTime(0.001, context.currentTime + 1.4);

    oscillator.connect(gain).connect(context.destination);
    oscillator.start();
    oscillator.stop(context.currentTime + 1.5);
}

function checkAlarms() {
    const now = new Date();
    const time = formatInTimezone(now, { hour: '2-digit', minute: '2-digit' });
    const day = Number(formatInTimezone(now, { weekday: 'short' })
        .replace('Sun', '0')
        .replace('Mon', '1')
        .replace('Tue', '2')
        .replace('Wed', '3')
        .replace('Thu', '4')
        .replace('Fri', '5')
        .replace('Sat', '6'));
    const stamp = `${formatInTimezone(now, { year: 'numeric', month: '2-digit', day: '2-digit' })}-${time}`;

    state.alarms.forEach(alarm => {
        if (!alarm.enabled || alarm.time !== time || !alarm.days.includes(day)) return;
        if (state.lastTriggered[alarm.id] === stamp) return;

        state.lastTriggered[alarm.id] = stamp;
        saveState();
        playAlarm();
        alert(`${alarm.label} · ${alarm.time}`);
    });

    updateNextAlarm();
}

elements.locationForm.addEventListener('submit', ev => {
    ev.preventDefault();
    updateLocation(elements.locationInput.value);
});

elements.alarmForm.addEventListener('submit', ev => {
    ev.preventDefault();
    state.alarms.push(createAlarm(ev.currentTarget));

    saveState();
    renderAlarms();

    elements.alarmForm.reset();
    elements.alarmTime.value = '07:30';
    elements.alarmEnabled.checked = true;
    elements.weatherAware.checked = true;
});

elements.alarmsList.addEventListener('click', ev => {
    const button = ev.target.closest('button');
    if (!button) return;

    const row = button.closest('[data-id]');
    const alarm = state.alarms.find(item => item.id === row.dataset.id);
    if (!alarm) return;

    if (button.dataset.action === 'delete') {
        state.alarms = state.alarms.filter(item => item.id !== alarm.id);
    }

    if (button.dataset.action === 'toggle') {
        alarm.enabled = !alarm.enabled;
    }

    saveState();
    renderAlarms();
});

elements.clearAlarms.addEventListener('click', () => {
    state.alarms = [];
    saveState();
    renderAlarms();
});

// Main process
(() => {
    createConnection();

    elements.locationInput.value = state.location;
    elements.locationName.textContent = state.location;
    elements.timezone.textContent = state.timezone;

    renderAlarms();
    updateClock();
    updateLocation(state.location);

    setInterval(updateClock, 1000);
    setInterval(checkAlarms, 15000);
    setInterval(() => updateLocation(state.location), 300000);
})();
