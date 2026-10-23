# Intent: usage from Claude Code's status line

Issue: #2. Author: Juwaidy (owner). Status: draft (gates off).

## Problem
The Clawdmeter on the desk is connected over Bluetooth and starts with the PC,
but it shows no usage. Since Claude Code 2.1.207 on Windows, both ways the app
reads limits fail: the sign-in token is no longer readable from
`.credentials.json`, and `claude -p /usage` prints only a cost summary. The
probe (a small Haiku call) never runs because no window is known. The owner
sees a dead meter every day.

## Proposed outcome
While Claude Code is in use, the device shows the real 5-hour and weekly
percentages and reset times within about a minute of each reply, on this
Windows PC, with no setup beyond the app's existing Hooks page.

Check: after one prompt in Claude Code, `clawdmeter --once` prints
`session N%, week M%` matching Claude Code's own numbers.

## Affected users and systems
- The owner's Windows PC (Claude Code CLI and the Claude desktop app's Code tab).
- `clawdmeter-app` (hook command, Hooks page installer, daemon) and
  `clawdmeter-core` (usage sources, engine).
- Each account's `settings.json` (a `statusLine` entry is added).

## Constraints
- **No extra AI calls.** Nothing new may send a prompt to Claude.
- **Must never start a 5-hour window** (existing owner's rule).
- **No data is never a zero** (existing hard rule).
- A status line the user already has must keep working.
- The token is never read, printed or stored by this change.

## Out of scope
- Fixing the Windows token lookup or the `/usage` parser.
- Changing the BLE protocol or firmware.
- Usage while Claude Code is closed (nothing changes then, except resets).

## Open questions
- Does the Claude desktop app's Code tab run a `statusLine` command? (Hooks
  do run there: `events.log` shows its sessions.) To check first, in the plan.
