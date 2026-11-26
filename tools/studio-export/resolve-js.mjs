// The Studio TS client ships browser-style ESM: extensionless imports ("./studio") and
// `import * as _m0 from "protobufjs/minimal"` (CommonJS). Patch both for Node.
import { registerHooks } from "node:module";
const shim = new URL("./protobuf-shim.mjs", import.meta.url).href;
registerHooks({
  resolve(specifier, context, next) {
    if (specifier === "protobufjs/minimal" && !context.parentURL?.endsWith("protobuf-shim.mjs")) {
      return { url: shim, shortCircuit: true };
    }
    try {
      return next(specifier, context);
    } catch (e) {
      if (e.code === "ERR_MODULE_NOT_FOUND" && !specifier.endsWith(".js") && (specifier.startsWith(".") || specifier.includes("/"))) {
        return next(specifier + ".js", context);
      }
      throw e;
    }
  },
});
