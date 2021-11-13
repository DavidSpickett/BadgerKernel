import os
import lit
from textwrap import dedent


# Run test.sh for each demo
class MakeTest(lit.formats.base.TestFormat):
    def execute(self, test, litConfig):
        if test.unsupported:
            return lit.Test.UNSUPPORTED, ''

        filename = os.path.basename(test.getSourcePath())
        test_script = os.path.join(test.getSourcePath(), "test.sh")
        cmd = ["bash", test_script]
        try:
            timed_out = False
            out, err, exitCode = lit.util.executeCommand(cmd, timeout=30)
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
            full_path = os.path.join(source_path, directory)
            # __ to ignore __pycache__
            # No test.sh means the test was disabled by cmake for this config.
            if not os.path.isdir(full_path) or \
                directory in localConfig.excludes or \
                    directory.startswith("__"):
                continue

            test = lit.Test.Test(testSuite,
                                 path_in_suite + (directory,),
                                 localConfig)
            has_test_script = "test.sh" in os.listdir(full_path)
            test.unsupported = localConfig.unsupported or not has_test_script
            yield test
