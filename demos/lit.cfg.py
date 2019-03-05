# Configuration file for the 'lit' test runner.

import os
import sys
import re
sys.path.append(os.path.dirname(os.path.abspath(__file__)))
from test_format import MakeTest

def get_build_config():
  # Read CMakeCache.txt to find our build settings
  found = dict()
  expected = set(['BUILD_PLATFORM', 'OPT_LEVEL', 'UBSAN'])
  var_regex = re.compile("(?P<var>.+):.+=(?P<value>.+)")
  with open("CMakeCache.txt", 'r') as cf:
    for line in cf.read().splitlines():
      m = var_regex.match(line)
      if m:
        var_name = m.group('var')
        if var_name in expected:
          found[var_name] = m.group('value')
          expected.remove(var_name)

          if not expected:
            break

    if expected:
      raise RuntimeError("Couldn't find the build config.")

  return found.values()

platform, opt_level, ubsan = get_build_config()
config.name = '{}_O{}_UBSAN_{}'.format(platform, opt_level, ubsan)
config.test_format = MakeTest()
config.suffixes = ['.log']
config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = os.path.join(config.test_source_root, '..')
# No timer support for Arm
config.excludes = ['timer.c'] if platform.lower() == 'arm' else []
