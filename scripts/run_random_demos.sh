#!/bin/bash
set -e pipefail

while true; do
  python3 ./scripts/gen_random_demo.py > demos/generated/generated.c
  ninja generated > /dev/null 2>&1
  ninja run_generated > /tmp/generated.log
  cat /tmp/generated.log

  # Check output formatting (second part is for ninja's status messages)
  if grep -v -P "(Thread\s.*:\s.*|\[.*\]\s.*)" /tmp/generated.log; then
    echo "Inconsistent log output!"
    exit 1
  else
    :
  fi

  # Should exit normally
  grep "all threads finished" /tmp/generated.log
  echo "**************************************************"
done
