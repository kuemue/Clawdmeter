# Plan: usage from Claude Code's status line

Issue: #2. From spec.md (2026-10-23). Accepted by: gates off.

## Files that change
- `desktop/crates/clawdmeter-app/src/hooks/mod.rs` (changed): `Action::Status`, route to `statusline`.
- `desktop/crates/clawdmeter-app/src/hooks/statusline.rs` (new): parse stdin, write the reading, chain the previous command, print the short line.
- `desktop/crates/clawdmeter-app/src/hooks/install.rs` (changed): `merge_statusline` / `remove_statusline`, save and restore the previous `statusLine`.
- `desktop/crates/clawdmeter-app/src/ui/settings.rs`, `tray/mod.rs` (changed): call the status line install/remove next to the hook install.
- `desktop/crates/clawdmeter-core/src/statusline.rs` (new): read `<state>/statusline/*.json` into `UsageReading`; freshness check.
- `desktop/crates/clawdmeter-core/src/paths.rs` (changed): `statusline_dir()`.
- `desktop/crates/clawdmeter-core/src/engine.rs` (changed): `begin_poll` prefers a fresh status line reading as `ProbePlan::Known`.
- `desktop/crates/clawdmeter-app/src/daemon.rs` (changed): load status line readings each tick and in `once`.
- `desktop/crates/clawdmeter-core/src/lib.rs` (changed): `pub mod statusline`.
- `desktop/CLAUDE.md`, `docs/user-guide.md` (changed): the new source.

## Order of work
1. **Spike (no commit):** add a throwaway `statusLine` that appends stdin to a temp file in the owner's `~/.claude/settings.json` (back up first, restore after). Send one prompt in the CLI and one in the desktop Code tab. Confirm `rate_limits` arrives in each. If the Code tab gets nothing, stop and report to the owner before step 2.
2. Core `statusline.rs`: pure `parse(&Value) -> Option<UsageReading>` and file read; unit tests with fixture JSON (both windows, one window, none, past `resets_at`, garbage).
3. `paths.rs` `statusline_dir()` + test.
4. Hook `statusline.rs` + `Action::Status`: tests with a temp state dir: writes file, atomic, bad input writes nothing, exit 0, short line printed, chained fake command's stdout passed through.
5. Installer: `merge_statusline` / `remove_statusline` tests: no file, existing `statusLine` saved and restored byte-equal, ours not doubled, invalid JSON writes nothing.
6. Engine: fresh reading → `Known`; stale/older → today's path; past five-hour reset → `NotStarted`; tests in `engine.rs` style (`plans(&mut e, T0 + …)`).
7. Daemon + `--once` wiring; settings/tray install calls.
8. Docs.
9. Live check on this PC: install from the Hooks page, prompt once, `clawdmeter --once`, then look at the device.

## Risks
- **Riskiest: step 1.** If the desktop Code tab ignores `statusLine`, the fix only helps the CLI. Checked first.
- Wrapping a user's status line could slow or break their bar: chained command gets the same 300 ms-debounced stdin; we add only a file write.
- Multiple sessions write the same account file at once: atomic rename, last one wins (they carry the same server numbers).
- Not doing: Windows token lookup, `/usage` parser repair, turning the probe off (owner decides), any firmware or protocol change.

## Proof
- `cd desktop && cargo fmt --check && cargo clippy --workspace --all-targets -- -D warnings && cargo test --workspace --locked`
- Windows core check per `desktop/CLAUDE.md`.
- `clawdmeter --once` after one prompt prints `session N%, week M%` matching Claude Code.
- Photo or screenshot of the device showing the meters; Hooks page screenshot.
- No new `claude -p` or network call: grep the diff for `Command::new` / `ureq` in the new code (only the chained user command).

## As built: step 1 spike (2026-10-23)
- CLI (`claude` 2.1.207, interactive): runs `statusLine` on every update. It sent no `rate_limits` because the CLI said "Login expired · Please run /login".
- Desktop Code tab: a new session replied, and `statusLine` never ran.
- Stopped here, as the plan says. The status line can't feed the meter for desktop-app work, which is most of the owner's use.
- Likely real cause of the token miss: the CLI sign-in in `~/.claude/.credentials.json` has expired. A fresh `/login` should bring back the existing OAuth source, and `claude -p /usage` renews the token after that. Next: owner runs `/login` and we re-check before building anything.
