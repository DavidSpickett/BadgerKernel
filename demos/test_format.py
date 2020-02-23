import os
import lit
from textwrap import dedent

# Run make test_<bla> target for each demo
class MakeTest(lit.formats.base.TestFormat):
  def execute(self, test, litConfig):
    filename = os.path.basename(test.getSourcePath())
    # Workaround "error: opening build log: No such file or directory"
    # in Azure by running single threaded.
    cmd = ["ninja", "test_" + os.path.splitext(filename)[0], "-j", "1"]
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
    for _, _, filenames in os.walk(source_path):
      for filename in sorted(filenames):
        if not filename in localConfig.excludes and \
           os.path.splitext(filename)[1] == '.c':
          yield lit.Test.Test(
                  testSuite,
                  path_in_suite + (filename,),
                  localConfig)
