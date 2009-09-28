#!/bin/bash

FAILED=0

../iec16022 -o testsuite-test.text -f Text -c "Aolash3l dee6Ieke OhBohm1C MengaR9m zaHaoQu2 huW3Uer8 ieg7chaJ haiKua1o"
../iec16022 -o testsuite-test.eps -f EPS -c "Aolash3l dee6Ieke OhBohm1C MengaR9m zaHaoQu2 huW3Uer8 ieg7chaJ haiKua1o"
../iec16022 -o testsuite-test.png -f PNG -c "Aolash3l dee6Ieke OhBohm1C MengaR9m zaHaoQu2 huW3Uer8 ieg7chaJ haiKua1o"
../iec16022 -o testsuite-test.bin -f Bin -c "Aolash3l dee6Ieke OhBohm1C MengaR9m zaHaoQu2 huW3Uer8 ieg7chaJ haiKua1o"
../iec16022 -o testsuite-test.hex -f Hex -c "Aolash3l dee6Ieke OhBohm1C MengaR9m zaHaoQu2 huW3Uer8 ieg7chaJ haiKua1o"

diff -b testsuite-test.text testsuite-example.text && echo "Text test passed" || echo "Text test FAILED" || FAILED=1
diff -b testsuite-test.eps testsuite-example.eps && echo "EPS test passed" || echo "EPS test FAILED" || FAILED=1
diff -b testsuite-test.png testsuite-example.png && echo "PNG test passed" || echo "PNG test FAILED" || FAILED=1
diff -b testsuite-test.bin testsuite-example.bin && echo "Bin test passed" || echo "Bin test FAILED" || FAILED=1
diff -b testsuite-test.hex testsuite-example.hex && echo "Hex test passed" || echo "Hex test FAILED" || FAILED=1

rm testsuite-test.*

exit $FAILED
