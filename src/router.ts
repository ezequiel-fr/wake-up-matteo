import { join, resolve } from 'node:path';

import express, { Router } from 'express';

const router = Router();

// Home page (temp)
const PUBLIC_DIR = resolve(__dirname, '../public');

router.get('/', (_req, res) => {
    console.log(join(PUBLIC_DIR, 'index.html'));
    return res.sendFile(join(PUBLIC_DIR, 'index.html'));
});

// Static assets
router.use(express.static(PUBLIC_DIR, {
    index: false, // Don't serve index.html for directory requests
    redirect: false, // Don't redirect to trailing slash
}))

export default router;
