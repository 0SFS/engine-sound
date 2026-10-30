// Build/verify helpers shared by the extracted legacy piston and turbine kernels.
import { execFileSync } from "node:child_process";
import { createHash } from "node:crypto";
import { mkdirSync, readFileSync, readdirSync, writeFileSync } from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

const sharedSourceDirectory = fileURLToPath(new URL("../src/", import.meta.url));
const sha256 = bytes => createHash("sha256").update(bytes).digest("hex");
const requiredExports = [
  "memory", "osfs_voice_abi_version", "osfs_voice_init", "osfs_voice_reset",
  "osfs_voice_input_ptr", "osfs_voice_output_ptr", "osfs_voice_profile_ptr",
  "osfs_voice_profile_size", "osfs_voice_commit_profile", "osfs_voice_process",
];
const compilerFlags = [
  "-O3", "-std=c++17", "--no-entry", "-sSTANDALONE_WASM=1",
  "-sINITIAL_MEMORY=4194304", "-sSTACK_SIZE=131072", "-sFILESYSTEM=0",
  "-fno-exceptions", "-fno-rtti", "-fno-fast-math",
];

function readManifest(root) {
  return JSON.parse(readFileSync(path.join(root, "public/0sfs-mod.json"), "utf8"));
}

function artifactPaths(root, manifest) {
  const wasm = path.resolve(root, "public", manifest.audioRenderer.wasm.path);
  return { wasm, provenance: wasm.replace(/\.wasm$/, ".provenance.json") };
}

function sourceInventory(root) {
  const directories = [
    { directory: path.join(root, "src"), prefix: "src/" },
    { directory: sharedSourceDirectory, prefix: "@0sfs/engine-sound/src/" },
  ];
  return directories.flatMap(({ directory, prefix }) => readdirSync(directory)
    .filter(name => /\.(h|cpp)$/.test(name))
    .map(name => ({ file: prefix + name, sha256: sha256(readFileSync(path.join(directory, name))) })))
    .sort((a, b) => a.file.localeCompare(b.file));
}

export async function buildFamily(url) {
  const root = fileURLToPath(url);
  const version = execFileSync("emcc", ["--version"], { encoding: "utf8" }).match(/\d+\.\d+\.\d+/)?.[0];
  if (version !== "6.0.9") throw new Error("Pinned emcc 6.0.9 is required");
  const manifest = readManifest(root);
  const paths = artifactPaths(root, manifest);
  mkdirSync(path.dirname(paths.wasm), { recursive: true });
  execFileSync("emcc", [
    ...compilerFlags, "-I", sharedSourceDirectory, path.join(root, "src/voice.cpp"), "-o", paths.wasm,
  ], { stdio: "inherit" });
  const bytes = readFileSync(paths.wasm);
  const module = await WebAssembly.compile(bytes);
  const provenance = {
    abi: "osfs-engine-voice-v1",
    toolchain: { compiler: "emcc", version, flags: compilerFlags },
    sources: sourceInventory(root),
    wasm: { bytes: bytes.length, sha256: sha256(bytes) },
    exports: WebAssembly.Module.exports(module).map(entry => entry.name),
  };
  writeFileSync(paths.provenance, JSON.stringify(provenance, null, 2) + "\n");
  manifest.audioRenderer.wasm.sha256 = provenance.wasm.sha256;
  writeFileSync(path.join(root, "public/0sfs-mod.json"), JSON.stringify(manifest, null, 2) + "\n");
  return verifyFamily(url);
}

export async function verifyFamily(url) {
  const root = fileURLToPath(url);
  const manifest = readManifest(root);
  const paths = artifactPaths(root, manifest);
  const provenance = JSON.parse(readFileSync(paths.provenance, "utf8"));
  const bytes = readFileSync(paths.wasm);
  const digest = sha256(bytes);
  if (digest !== provenance.wasm.sha256 || digest !== manifest.audioRenderer.wasm.sha256) {
    throw new Error("Voice artifact, provenance and manifest hashes differ");
  }
  if (JSON.stringify(sourceInventory(root)) !== JSON.stringify(provenance.sources)) {
    throw new Error("Voice source provenance drift; run npm run build:wasm");
  }
  const module = await WebAssembly.compile(bytes);
  if (WebAssembly.Module.imports(module).length !== 0) throw new Error("Voice module must have no imports");
  const names = new Set(WebAssembly.Module.exports(module).map(entry => entry.name));
  for (const name of requiredExports) if (!names.has(name)) throw new Error("Missing " + name);
  const instance = new WebAssembly.Instance(module, {});
  instance.exports._initialize?.();
  if (instance.exports.osfs_voice_abi_version() !== 1) throw new Error("Voice ABI mismatch");
  return { artifact: manifest.id.split("/").at(-1), sha256: digest, bytes: bytes.length };
}
