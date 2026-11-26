#!/usr/bin/env node
// Dump the live keymap (including ZMK Studio changes) from the keyboard over USB.
//
// Usage:  cd tools/studio-export && npm install && npm run export [-- /dev/cu.usbmodemXXXX]
// The LEFT half must be connected via USB. If Studio is locked, the script waits
// until you press the &studio_unlock key.
//
// Output: studio-keymap.json (raw layers + behavior names), plus a readable dump on stdout.
//
// Note: the `serialport` npm package delays reads on macOS (responses only arrive after the
// next write), so this talks to the CDC-ACM tty directly with Node's tty streams.

import fs from "node:fs";
import tty from "node:tty";
import { create_rpc_connection, call_rpc } from "@zmkfirmware/zmk-studio-ts-client";

const debug = !!process.env.DEBUG;

function findPort() {
  if (process.argv[2]) return process.argv[2];
  const ports = fs.readdirSync("/dev").filter((f) => f.startsWith("cu.usbmodem"));
  if (ports.length === 0) {
    console.error("No /dev/cu.usbmodem* found. Is the LEFT half connected via USB?");
    process.exit(1);
  }
  if (ports.length > 1) console.error(`Several ports found (${ports.join(", ")}), using the first.`);
  return "/dev/" + ports[0];
}

const path = findPort();
console.error(`Connecting to ${path} ...`);
const { O_RDWR, O_NOCTTY, O_NONBLOCK } = fs.constants;
const fd = fs.openSync(path, O_RDWR | O_NOCTTY | O_NONBLOCK); // opening asserts DTR
const input = new tty.ReadStream(fd);
input.setRawMode(true);

const readable = new ReadableStream({
  start(controller) {
    input.on("data", (d) => {
      if (debug) console.error("<", d.toString("hex"));
      controller.enqueue(new Uint8Array(d));
    });
  },
});
const writable = new WritableStream({
  write(chunk) {
    if (debug) console.error(">", Buffer.from(chunk).toString("hex"));
    fs.writeSync(fd, chunk);
  },
});
const conn = create_rpc_connection({ label: path, abortController: new AbortController(), readable, writable });

const rpc = async (req) => {
  let timer;
  const resp = await Promise.race([
    call_rpc(conn, req),
    new Promise((_, rej) => (timer = setTimeout(() => rej(new Error("RPC timeout")), 5000))),
  ]).finally(() => clearTimeout(timer));
  if (resp.meta) throw new Error(`RPC meta error: ${JSON.stringify(resp.meta)}`);
  return resp;
};

const done = (code) => {
  input.destroy();
  process.exit(code);
};

// LockState: 0 = LOCKED, 1 = UNLOCKED
let lock = (await rpc({ core: { getLockState: true } })).core?.getLockState;
if (lock !== 1) {
  console.error("Studio is LOCKED. Press the &studio_unlock key (top-left on the base layer) ... (60s)");
  const deadline = Date.now() + 60_000;
  while (lock !== 1 && Date.now() < deadline) {
    await new Promise((r) => setTimeout(r, 1000));
    lock = (await rpc({ core: { getLockState: true } })).core?.getLockState;
  }
  if (lock !== 1) {
    console.error("Still locked, giving up.");
    done(2);
  }
  console.error("Unlocked.");
}

const info = (await rpc({ core: { getDeviceInfo: true } })).core?.getDeviceInfo;
const keymap = (await rpc({ keymap: { getKeymap: true } })).keymap.getKeymap;
const unsaved = (await rpc({ keymap: { checkUnsavedChanges: true } })).keymap.checkUnsavedChanges;

const ids = (await rpc({ behaviors: { listAllBehaviors: true } })).behaviors.listAllBehaviors.behaviors;
const behaviors = {};
for (const id of ids) {
  const d = (await rpc({ behaviors: { getBehaviorDetails: { behaviorId: id } } })).behaviors.getBehaviorDetails;
  behaviors[id] = d.displayName;
}

const out = { device: info, unsavedChanges: unsaved, behaviors, keymap };
fs.writeFileSync("studio-keymap.json", JSON.stringify(out, null, 2));

const hex = (n) => "0x" + (n >>> 0).toString(16);
for (const layer of keymap.layers) {
  console.log(`\n# layer id=${layer.id} "${layer.name}"`);
  layer.bindings.forEach((b, i) => {
    console.log(`${String(i).padStart(2)}: ${behaviors[b.behaviorId] ?? "?" + b.behaviorId} ${hex(b.param1)} ${hex(b.param2)}`);
  });
}
if (unsaved) console.error("\nNote: there are UNSAVED Studio changes; they are included above.");
console.error("\nWrote studio-keymap.json");
done(0);
