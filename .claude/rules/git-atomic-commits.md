# Atomic, progressive commits and pushes

When making changes, commit and push progressively as the work goes, not in one
batch at the end.

- One commit per logical change (a feature, a fix, a refactor, a docs update).
  Don't mix unrelated changes in one commit.
- Each commit should leave the project in a working state: the firmware still
  builds (`pio run -e usb`).
- Push after each commit, so the remote always reflects the progress.
- Use Conventional Commit messages: `feat:`, `fix:`, `refactor:`, `docs:`,
  `build:`, `chore:`, with an optional scope, e.g. `feat(ftms): ...`.
- Never commit secrets: `secrets.ini` and `include/secrets.h` stay ignored.
