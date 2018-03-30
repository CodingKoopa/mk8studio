#!/bin/bash -ex

# Lints the code with ClangFormat.

if find . -type f -not -path "./.git/*" -exec grep -nr '\s$' {} +; then
  echo "Error: Trailing whitespace found. Quitting."
  exit 1
fi

# If this job is being triggered by a PR.
if [[ "$TRAVIS_EVENT_TYPE" = "pull_request" ]]; then
  # Get a list of modified files.
  FILES_TO_LINT="$(git diff --name-only --diff-filter=ACMRTUXB "$TRAVIS_COMMIT_RANGE" | \
    grep '^Source/[^.]*[.]\(cpp\|h\)$' || true)"
# If this job is being triggered by a branch push.
else
  # Get a list of all files.
  FILES_TO_LINT="$(find Source/ -name '*.cpp' -or -name '*.h')"
fi

ERROR=false

# Turn off tracing for this because it's too verbose.
set +x
# Iterate over the files to line.
for FILE in $FILES_TO_LINT; do
  DIFF=$(diff -u "$FILE" <(clang-format "$FILE") || true)
  if [[ ! -z "$DIFF" ]]; then
    echo "Error: $FILE is not compliant to coding style, here is the fix:"
    echo "$DIFF"
    ERROR=true
  fi
done
# Reenable tracing.
set -x

if $ERROR; then
  exit 1
fi
