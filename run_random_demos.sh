set -e

while true; do
  python3 gen_random_demo.py > demos/generated.c
  make generated 2>&1 > /dev/null
  make run_generated > /tmp/generated.log
  cat /tmp/generated.log

  # Check output formatting (second part is for make's status messages)
  if cat /tmp/generated.log | grep -v -P "(Thread\s.*:\s.*|\[.*\]\s.*)"; then
    echo "Inconsistent log output!"
    exit 1
  else
    :
  fi

  # Should exit normally
  cat /tmp/generated.log | grep "all threads finished"
  echo "**************************************************"
done
