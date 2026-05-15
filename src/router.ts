import { join } from 'node:path';

import express, { Router } from 'express';

import config from '@/constants';

const router = Router();

// Home page (temp)

router.get('/', (_req, res) => {
    return res.sendFile(join(config.APP.PUBLIC_DIR, 'index.html'));
});

// Static assets
router.use(express.static(config.APP.PUBLIC_DIR, {
    index: false, // Don't serve index.html for directory requests
    redirect: false, // Don't redirect to trailing slash
}))

export default router;
