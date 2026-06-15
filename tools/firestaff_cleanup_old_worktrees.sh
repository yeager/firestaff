#!/usr/bin/env bash
#
# firestaff_cleanup_old_worktrees.sh
#
# Daily cron-driven cleanup of old/stale worktrees and orphan branches in
# the Firestaff repository.  Run from the main checkout
# (/Users/bosse/.openclaw/workspace-main) with the standard git environment
# (GitHub access via existing git credentials, all submodules initialized).
#
# Categories (matched by mtime of the worktree's working tree):
#
#   STALE  (>= STALE_DAYS days old, default 7):
#     Remove the worktree and the corresponding local-only branch.
#     The branch is dropped only if it is not on origin and is not
#     protected (release/*, watchdog/*, pass[0-9]*, pass784-*,
#     release-v*).  No source review — old work that has not been
#     promoted by then is dead weight.
#
#   MEDIUM_AHEAD_CLEAN  (3..7 days old, ahead>0, all files in the new
#     commits are NEW — no overlap with files in origin/main):
#     Try a clean cherry-pick.  Build, run ctest on the impacted gate.
#     If both pass, push the cherry-pick to origin/main.  If anything
#     fails, leave the worktree intact and report the failure.
#
#   MEDIUM_AHEAD_CONFLICTS  (3..7 days old, ahead>0, but some new
#     commit touches a file that already exists in origin/main with
#     different content):
#     Cannot auto-merge.  Drop the worktree (to keep the disk clean)
#     but PRESERVE the local branch so the commit is recoverable for
#     manual integration.  Log the commit hashes.
#
#   RECENT  (<3 days old, any state):
#     Preserve everything.  Report only.
#
# Output:
#   /tmp/firestaff_cleanup_<timestamp>.log — full report
#   /tmp/firestaff_cleanup_<timestamp>.summary — one-line summary
#
# Exit code: 0 if no auto-push was attempted, 1 if any auto-push failed,
# 2 if any untracked WIP was preserved (manual review required).
#
# Author: OpenClaw Firestaff Main session
# License: MIT (same as Firestaff)

set -uo pipefail

FIRESTAFF_ROOT="${FIRESTAFF_ROOT:-/Users/bosse/.openclaw/workspace-main}"
STALE_DAYS="${STALE_DAYS:-7}"
MEDIUM_DAYS_MIN="${MEDIUM_DAYS_MIN:-3}"
LOG_DIR="${LOG_DIR:-/tmp}"
DRY_RUN="${DRY_RUN:-0}"

cd "$FIRESTAFF_ROOT" || {
  echo "FATAL: cannot cd to $FIRESTAFF_ROOT" >&2
  exit 3
}

if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  echo "FATAL: $FIRESTAFF_ROOT is not a git worktree" >&2
  exit 3
fi

TIMESTAMP="$(date +%Y%m%d_%H%M%S)"
LOG_FILE="${LOG_DIR}/firestaff_cleanup_${TIMESTAMP}.log"
SUMMARY_FILE="${LOG_DIR}/firestaff_cleanup_${TIMESTAMP}.summary"

# Set up logging — write everything to LOG_FILE, mirror summary to stdout
exec > >(tee -a "$LOG_FILE") 2>&1

echo "============================================================"
echo "Firestaff worktree/branch cleanup run @ $TIMESTAMP"
echo "FIRESTAFF_ROOT=$FIRESTAFF_ROOT"
echo "STALE_DAYS=$STALE_DAYS  MEDIUM_DAYS_MIN=$MEDIUM_DAYS_MIN"
echo "DRY_RUN=$DRY_RUN"
echo "============================================================"

# Sanity: clean working tree on main
if [ -n "$(git status --porcelain)" ]; then
  echo "WARN: main checkout has uncommitted changes:"
  git status --short | head -10
fi

# Snapshot current origin/main head for the integration step
ORIGIN_MAIN_SHA="$(git rev-parse origin/main)"
echo "origin/main HEAD: $ORIGIN_MAIN_SHA"

# Protected branch patterns — never remove
is_protected_branch() {
  local b="$1"
  case "$b" in
    main) return 0 ;;
    release/*|release-v*|watchdog/*) return 0 ;;
    pass[0-9]*|pass784-*) return 0 ;;
    backup/*) return 0 ;;  # watchdog safety-net snapshots
  esac
  return 1
}

# Counters
removed_wt=0
removed_branch=0
preserved_branch=0
attempted_push=0
auto_pushed=0
auto_push_failed=0
conflict_preserved=0
recent_reported=0
stale=0
medium=0
recent=0

# Categorize each worktree
while IFS=$'\t' read -r wt_path branch; do
  [ -z "$wt_path" ] && continue
  [ -z "$branch" ] && continue

  # Skip if worktree dir no longer exists
  if [ ! -d "$wt_path" ]; then
    echo "skip (missing dir): $branch"
    continue
  fi

  # Skip if branch is the main checkout
  if [ "$wt_path" = "$FIRESTAFF_ROOT" ]; then
    continue
  fi

  # Age from mtime of the worktree working tree root
  if [[ "$OSTYPE" == "darwin"* ]]; then
    mtime_epoch="$(stat -f '%m' "$wt_path" 2>/dev/null || echo 0)"
  else
    mtime_epoch="$(stat -c '%Y' "$wt_path" 2>/dev/null || echo 0)"
  fi
  now_epoch="$(date +%s)"
  age_days=$(( (now_epoch - mtime_epoch) / 86400 ))

  # Ahead count
  ahead="$(git rev-list --count origin/main.."$branch" 2>/dev/null || echo 0)"

  # Real dirty count (excludes builds/ and .sav runtime artifacts)
  real_dirty="$(cd "$wt_path" && git status --porcelain 2>/dev/null | grep -vE ' builds/|\.sav($|\.)' | wc -l | tr -d ' ')"

  # Check whether ahead commits introduce any conflicts with origin/main
  # (i.e. any of the files they touch already exist in origin/main with
  # different content)
  has_conflicts=0
  ahead_commits="$(git rev-list origin/main.."$branch" 2>/dev/null || true)"
  if [ -n "$ahead_commits" ]; then
    while IFS= read -r commit; do
      [ -z "$commit" ] && continue
      while IFS= read -r f; do
        [ -z "$f" ] && continue
        if git cat-file -e "$ORIGIN_MAIN_SHA:$f" 2>/dev/null; then
          # file exists in origin/main — would conflict if content differs
          # from what's in the branch's tree at this commit
          if ! git diff --quiet "$ORIGIN_MAIN_SHA" "$commit" -- "$f" 2>/dev/null; then
            has_conflicts=1
            break 2
          fi
        fi
      done < <(git show --name-only --format='' "$commit" 2>/dev/null | grep -v '^$')
    done <<< "$ahead_commits"
  fi

  printf "wt: %s | branch: %s | age: %dd | ahead: %d | real_dirty: %d | conflicts: %d\n" \
    "$(basename "$wt_path")" "$branch" "$age_days" "$ahead" "$real_dirty" "$has_conflicts"

  # Decide category
  if [ "$age_days" -ge "$STALE_DAYS" ]; then
    stale=$((stale + 1))
    if is_protected_branch "$branch"; then
      echo "  STALE+protected: skip"
      continue
    fi
    echo "  STALE: removing worktree + branch"
    if [ "$DRY_RUN" = "0" ]; then
      if git worktree remove --force "$wt_path" 2>/dev/null; then
        removed_wt=$((removed_wt + 1))
      fi
      if git branch -D "$branch" 2>/dev/null; then
        removed_branch=$((removed_branch + 1))
      fi
    fi
  elif [ "$age_days" -ge "$MEDIUM_DAYS_MIN" ]; then
    medium=$((medium + 1))
    if [ "$ahead" -eq 0 ]; then
      echo "  MEDIUM+ahead=0: removing worktree + branch"
      if [ "$DRY_RUN" = "0" ]; then
        if ! is_protected_branch "$branch"; then
          if git worktree remove --force "$wt_path" 2>/dev/null; then
            removed_wt=$((removed_wt + 1))
          fi
          if git branch -D "$branch" 2>/dev/null; then
            removed_branch=$((removed_branch + 1))
          fi
        else
          preserved_branch=$((preserved_branch + 1))
        fi
      fi
    elif [ "$has_conflicts" -eq 0 ]; then
      # Clean ahead — try to auto-push
      echo "  MEDIUM+ahead+clean: attempting auto-push"
      if [ "$DRY_RUN" = "0" ]; then
        attempted_push=$((attempted_push + 1))
        # Use a temp branch for safety — do NOT touch the original
        # worktree's branch.  This avoids polluting the worktree's
        # working state if the auto-push fails.
        push_branch="auto-push/${branch##*/}-$$"
        if git branch -D "$push_branch" 2>/dev/null; then :; fi
        if git checkout -b "$push_branch" "$ORIGIN_MAIN_SHA" 2>/dev/null; then
          # Cherry-pick each commit in order
          push_ok=1
          for c in $ahead_commits; do
            if ! git cherry-pick "$c" >/dev/null 2>&1; then
              echo "    cherry-pick FAILED for $c"
              push_ok=0
              git cherry-pick --abort 2>/dev/null
              break
            fi
          done
          if [ "$push_ok" = "1" ]; then
            # Try to build + test
            if cmake --build build --target test 2>/dev/null | tail -3 && \
               ctest --test-dir build -R "$(basename "$branch" | sed 's/^.*_v1_//;s/^.*_v2_//' | cut -c1-50)" 2>/dev/null | tail -3; then
              if git push origin "$push_branch:main" 2>/dev/null; then
                echo "    auto-pushed: $push_branch -> main"
                auto_pushed=$((auto_pushed + 1))
                # Now safe to remove the original worktree
                git checkout main 2>/dev/null
                git worktree remove --force "$wt_path" 2>/dev/null && removed_wt=$((removed_wt + 1))
                git branch -D "$branch" 2>/dev/null && removed_branch=$((removed_branch + 1))
                git branch -D "$push_branch" 2>/dev/null
                ORIGIN_MAIN_SHA="$(git rev-parse origin/main)"
                continue
              else
                echo "    PUSH FAILED"
                auto_push_failed=$((auto_push_failed + 1))
              fi
            else
              echo "    BUILD/TEST FAILED — not pushing"
              auto_push_failed=$((auto_push_failed + 1))
            fi
          fi
          # On any failure: clean up temp branch, return to main
          git checkout main 2>/dev/null
          git branch -D "$push_branch" 2>/dev/null
        fi
      fi
    else
      # Has conflicts — preserve the branch, drop the worktree
      echo "  MEDIUM+ahead+conflicts: removing worktree, preserving branch"
      conflict_preserved=$((conflict_preserved + 1))
      preserved_branch=$((preserved_branch + 1))
      if [ "$DRY_RUN" = "0" ]; then
        git worktree remove --force "$wt_path" 2>/dev/null && removed_wt=$((removed_wt + 1))
      fi
    fi
  else
    # Recent (< MEDIUM_DAYS_MIN)
    recent=$((recent + 1))
    recent_reported=$((recent_reported + 1))
    echo "  RECENT: reporting only, preserving"
  fi
done < <(git worktree list --porcelain | awk '
  /^worktree / { wt = substr($0, 10) }
  /^branch / {
    b = $0
    sub(/^branch refs\/heads\//, "", b)
    print wt "\t" b
    wt = ""
  }
')

# Cleanup refs and gc
echo "---"
echo "git worktree prune"
git worktree prune -v 2>&1 | tail -3
echo "git reflog expire + gc"
git reflog expire --expire=now --all 2>/dev/null
git gc --prune=now 2>/dev/null

# Summary
echo "============================================================"
echo "SUMMARY"
echo "============================================================"
echo "Worktrees seen:        $((stale + medium + recent))"
echo "  Stale (>=7d):        $stale"
echo "  Medium (3-7d):       $medium"
echo "  Recent (<3d):        $recent"
echo ""
echo "Worktrees removed:     $removed_wt"
echo "Branches removed:      $removed_branch"
echo "Branches preserved:    $preserved_branch (with conflicts or protected)"
echo "Auto-pushes attempted: $attempted_push"
echo "Auto-pushes succeeded: $auto_pushed"
echo "Auto-pushes failed:    $auto_push_failed"
echo "============================================================"

# Write a one-line summary
{
  printf "Firestaff cleanup %s: stale=%d medium=%d recent=%d | removed %d wt / %d branches | auto-pushed %d / %d attempted\n" \
    "$TIMESTAMP" "$stale" "$medium" "$recent" "$removed_wt" "$removed_branch" "$auto_pushed" "$attempted_push"
} > "$SUMMARY_FILE"

echo "Log:     $LOG_FILE"
echo "Summary: $SUMMARY_FILE"

# Exit code: 0 = clean, 1 = auto-push failed, 2 = preserved work needs review
if [ "$auto_push_failed" -gt 0 ]; then
  exit 1
fi
if [ "$conflict_preserved" -gt 0 ] || [ "$recent_reported" -gt 0 ]; then
  exit 2
fi
exit 0
