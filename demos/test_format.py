import os
import lit
from textwrap import dedent


# Run make test_<bla> target for each demo
class MakeTest(lit.formats.base.TestFormat):
    def execute(self, test, litConfig):
        if test.unsupported:
            return lit.Test.UNSUPPORTED, ''

        filename = os.path.basename(test.getSourcePath())
        cmd = ["make", "test_" + os.path.splitext(filename)[0]]
        try:
            timed_out = False
            out, err, exitCode = lit.util.executeCommand(
                cmd, timeout=30)
        except lit.util.ExecuteCommandTimeoutException as e:
            exitCode = 1
            timed_out = True
            out = e.out
            err = e.err
        finally:
            if not exitCode and not err.strip():
                return lit.Test.PASS, ''

            extra = ""
            if timed_out:
                extra = " timed out (UBSAN error?)"

            report = dedent('''\
              Command{}: {}
              stdout:
              {}
              stderr:
              {}''').format(extra, cmd, out, err)
            return lit.Test.FAIL, report

    def getTestsInDirectory(self, testSuite, path_in_suite,
                            litConfig, localConfig):
        source_path = testSuite.getSourcePath(path_in_suite)
        # Lit will recurse folders but we only care about the top level
        if os.path.basename(source_path) != 'demos':
            return

        for directory in sorted(os.listdir(source_path)):
            # __ to ignore __pycache__ etc.
            if os.path.isdir(os.path.join(source_path, directory)) and \
                    directory not in localConfig.excludes and \
                    not directory.startswith("__"):
                test = lit.Test.Test(testSuite,
                                     path_in_suite + (directory,),
                                     localConfig)
                test.unsupported = localConfig.unsupported
                yield test
