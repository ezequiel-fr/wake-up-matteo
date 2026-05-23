module.exports = {
	apps: [{
        name: 'wake-up-matteo',
        cwd: __dirname,
        script: './dist/index.js',
        // node_args: '-r module-alias/register',
        // exec_mode: 'fork',
        instances: 1,
        autorestart: true,
        watch: false,
        env: {
            APP_ENV: 'development',
        },
        env_production: {
            APP_ENV: 'production',
        },
        max_memory_restart: '128M',
        min_uptime: '10s',
        max_restarts: 10,
        time: true,
        error_file: './logs/err.log',
        out_file: './logs/out.log',
        log_date_format: 'HH:mm:ss DD-MM-YYYY Z'
    }],
};
