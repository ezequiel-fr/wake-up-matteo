export function isInRange(x: number, min: number, max: number, strict = false) {
    if (Number.isNaN(x)) return false;
    if (!Number.isFinite(x)) return false;

    if (strict) return min < x && x < max;
    return min <= x && x <= max;
}
