# Spec: usage from Claude Code's status line

Issue: #2. From intent.md (2026-10-23). Status: draft (gates off).

## Areas of concern
1. **Desktop Code tab may not run `statusLine`.** Most of the owner's work is
   there. If it doesn't, this source only covers the CLI. Decided by a spike
   (plan step 1) before any code; if it fails, stop and report.
2. **One `statusLine` per settings.json.** The user may already have one.
   Rule: keep it working. Decision: wrap it (we run theirs, pass stdin, print
   its output) and restore it on removal. Owner decides if wrapping is
   unwanted; default is wrap.
3. **An empty status line hides Claude Code's footer hints** (`esc to
   interrupt`, per the docs). With no prior status line we print a short
   usage line instead of nothing. Owner can veto the text.
4. **The probe is an AI call.** "No extra AI calls" means: when a fresh status
   line reading exists, the engine uses it and never probes. The probe stays
   only as today's fallback when no status line data exists; turning it off
   entirely is the owner's call (carried below).
5. **Stale data vs "no data is never a zero".** A status line reading only
   updates when Claude replies. After `five_hour.resets_at` passes with no new
   reading, the session shows "not started" (`sr = -1`, the existing
   `Source::NotStarted` path), never 0 % as a reading. Week keeps its last
   value until its own reset.
6. **Never start a window.** The status line command makes no network or
   Claude call; it only reads stdin and writes a file. Holds by construction.

## Requirements
1. `clawdmeter hook status` reads the status line JSON on stdin and, when
   `rate_limits.five_hour` or `rate_limits.seven_day` is present, writes the
   account's reading (pct, resets_at, read_at) to
   `<state>/statusline/<account key>.json` (atomic write). The account is
   `account_of(transcript_path)`, as for hook events.
2. It never prints the prompt, token or transcript; it makes no network or
   process call except the user's own previous status line command (req 4).
3. It always exits 0 and finishes under 1 s on bad or partial input.
4. With a saved previous status line command, it runs that command with the
   same stdin and prints its stdout unchanged. Without one, it prints
   `5h N% · wk M%` (or nothing extra when `rate_limits` is absent).
5. The Hooks page install adds `statusLine: {type: command, command:
   "<exe> hook status"}` to each account's settings.json, saving any previous
   `statusLine` object in the state dir; remove restores it exactly.
   Invalid JSON or an unknown layout writes nothing (as hooks today).
6. The engine treats a status line reading as `ProbePlan::Known` when its
   `read_at` is newer than the account's last reading and its five-hour
   `resets_at` (if any) is in the future. Then no probe and no `/usage` run.
7. A status line reading feeds the rate math like a normal `ok` reading
   (not `rp:1`).
8. With no status line file, behaviour is exactly as today.
9. `clawdmeter --once` also reads the status line file first.

## Design
- **Hook side** (`clawdmeter-app/src/hooks/`): new `Action::Status` in
  `mod.rs`; a small `statusline.rs` with `parse(&Value) -> Option<Reading>`
  (pure) and `write`. Previous command chaining lives here too.
- **Installer** (`hooks/install.rs`): `merge_statusline` / `remove_statusline`
  beside `merge` / `remove`, same error type and rules. Called from the same
  places as hook install (`ui/settings.rs`, `tray/mod.rs`).
- **Core** (`clawdmeter-core`): `statusline.rs` reads
  `<state>/statusline/*.json` into `UsageReading { session, week, .. }`.
  `engine.rs` `begin_poll`: before `self.source(d, now)`, a fresh status line
  reading → `ProbePlan::Known(r)`. Daemon loads files on each poll tick.
- **Paths**: `Paths::statusline_dir()` = `<state>/statusline`.
- **Docs**: `desktop/CLAUDE.md` "Usage sources" gets the new first source;
  `docs/user-guide.md` Hooks section mentions the status line.
- No protocol, firmware or payload key change.

## Rules applied
- Repo CLAUDE.md hard rules: no data is never a zero (concern 5); the probe
  must never start a window (concern 6); tests never run real `reg`/
  `systemctl` (tests use temp dirs and fake commands only).
- Hook stdout can reach the model: the status line command's stdout goes to
  the status bar, not the model, but still carries only percentages.
- PR rules: template, agent review on Opus, hardware note, human involvement.

## Open questions carried from intent.md
- Desktop Code tab runs `statusLine`? → plan step 1 spike. Owner: Claude.
- Turn the Haiku probe off entirely when the status line is installed? →
  Owner: Juwaidy. Default in this plan: keep it as fallback only.
