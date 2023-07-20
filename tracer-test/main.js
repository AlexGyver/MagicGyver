let offs = 100;
let size = 30;
let wait = 80;
let img_w = 20;
let img_h = 14;
let coords = [[0, 0]];

let img_map = [
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
];

function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}


function setup() {
    createCanvas(offs + img_w * size, offs + img_h * size);
    startCrawl();
    show_map();
    main();
}

function draw() { }

function show_map() {
    background(255);

    stroke(255, 0, 0);
    strokeWeight(1);
    for (let i = 1; i < coords.length; i++) {
        line(offs + size / 2 + coords[i - 1][0] * size,
            offs + size / 2 + coords[i - 1][1] * size,
            offs + size / 2 + coords[i][0] * size,
            offs + size / 2 + coords[i][1] * size
        );
    }

    stroke(0);
    strokeWeight(0.3);
    for (let y = 0; y < img_h; y++) {
        for (let x = 0; x < img_w; x++) {
            let v = 1 - img_map[x + y * img_w];
            fill(v * 255);
            rect(offs + x * size, offs + y * size, size, size);
        }
    }
    show_head();

    stroke(255, 0, 0);
    strokeWeight(1);
    for (let i = 1; i < coords.length; i++) {
        line(offs + size / 2 + coords[i - 1][0] * size,
            offs + size / 2 + coords[i - 1][1] * size,
            offs + size / 2 + coords[i][0] * size,
            offs + size / 2 + coords[i][1] * size
        );
    }

    stroke(0);
    strokeWeight(0.3);
}
function show_head() {
    fill(255, 0, 0);
    rect(offs + cr_x * size, offs + cr_y * size, size, size);
}
function show_search(x, y) {
    fill(255, 255, 0);
    rect(offs + x * size, offs + y * size, size, size);
}

let cr_x = 0, cr_y = 0, cr_px = 0, cr_py = 0;
let cr_size = 1;
let cr_head = 0;
let cr_count = 0;
let cr_loop = 0;
let cr_skip = 1;

async function main() {

    async function checkArea() {

        async function check(x, y) {
            await sleep(wait);
            show_search(x, y);

            if (getBuffer(x, y) == 1) {
                moveTo(x, y);
                show_map();
                await sleep(100);
                return true;
            }
            return false;
        }

        async function moveTo(x, y) {
            coords.push([x, y]);
            cr_count++;
            img_map[x + y * img_w] = 0;

            if (cr_count % cr_skip == 0) {
                cr_px = x;
                cr_py = y;
            }
            cr_x = x;
            cr_y = y;
        }

        function getBuffer(x, y) {
            if (x < 0 || x >= img_w || y < 0 || y >= img_h) return 0;
            return img_map[x + y * img_w];
        }

        let checkers = [
            async function () {
                for (let x = cr_x - cr_size; x < cr_x + cr_size; x++) {
                    let v = await check(x, cr_y - cr_size);
                    if (v) return 1;
                }
                return 0;
            },
            async function () {
                for (let y = cr_y - cr_size; y < cr_y + cr_size; y++) {
                    let v = await check(cr_x + cr_size, y);
                    if (v) return 2;
                }
                return 0;
            },
            async function () {
                for (let x = cr_x + cr_size; x > cr_x - cr_size; x--) {
                    let v = await check(x, cr_y + cr_size);
                    if (v) return 3;
                }
                return 0;
            },
            async function () {
                for (let y = cr_y + cr_size; y > cr_y - cr_size; y--) {
                    let v = await check(cr_x - cr_size, y);
                    if (v) return 4;
                }
                return 0;
            }
        ];


        let cr_loop = 1;
        for (let i = 0; i < 4; i++) {
            let v = await checkers[cr_loop ? ((cr_head + i) % 4) : i]();
            if (v) {
                cr_head = v + 2;
                return v;
            }
        }
        return 0;
    }

    while (true) {
        let v = await checkArea();
        if (v) cr_size = 1;
        else cr_size++;

        if (cr_size >= img_w) return;
        let stop = true;
        for (let i = 0; i < img_map.length; i++) {
            if (img_map[i] == 1) {
                stop = false;
                break;
            }
        }
        if (stop) return;
    }
}

function startCrawl() {
    cr_x = 0, cr_y = 0, cr_px = 0, cr_py = 0;
    cr_count = 0;
    cr_size = 1;
}