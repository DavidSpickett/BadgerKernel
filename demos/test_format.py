import os
import lit
from textwrap import dedent

# Run make test_<bla> target for each demo
class MakeTest(lit.formats.base.TestFormat):
  def execute(self, test, litConfig):
    filename = os.path.basename(test.getSourcePath())
    cmd = ["make", "test_" + os.path.splitext(filename)[0]]
    out, err, exitCode = lit.util.executeCommand(cmd)

    if not exitCode and not err.strip():
      return lit.Test.PASS,''

    report = dedent('''\
      Command: {}
      stdout:
      {}
      stderr:
      {}''').format(cmd, out, err)
    return lit.Test.FAIL, report

  def getTestsInDirectory(self, testSuite, path_in_suite,
                          litConfig, localConfig):
    source_path = testSuite.getSourcePath(path_in_suite)
    for _, directories, _ in os.walk(source_path):
      for directory in sorted(directories):
        # __ to ignore __pycache__ etc.
        if not directory in localConfig.excludes and \
            not directory.startswith("__"):
          yield lit.Test.Test(
                  testSuite,
                  path_in_suite + (directory,),
                  localConfig)
