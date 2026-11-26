// protobufjs/minimal is CommonJS; `import * as _m0` in Node only exposes `default`.
// Re-export its members as named exports so the Studio client finds them.
import m from "protobufjs/minimal.js";
export default m;
export const { Writer, Reader, util, configure, roots, rpc, BufferWriter, BufferReader } = m;
