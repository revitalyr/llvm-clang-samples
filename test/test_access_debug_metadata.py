import unittest
from tools import SamplesTestCase


OUTPUT_FACT = '''\
Found MDNode:
  Has MDNode operand:
    1 operands
'''

OUTPUT_GLOBALS = '''\
Found MDNode:
  Has MDNode operand:
    1 operands
'''

OUTPUT_SIMPLESWITCH = '''\
Found MDNode:
  Has MDNode operand:
    1 operands
'''

OUTPUT_THREADIDX = '''\
./build/access_debug_metadata: ./inputs/threadidx.ll:12:18: error: expected comma after load's type
  %0 = load i32* getelementptr inbounds (%struct.uint3* @threadIdx, i64 0, i32 1), align 4, !tbaa !1
                 ^
'''

OUTPUT_TWODOUBLEFOO = '''\
Found MDNode:
  Has MDNode operand:
    1 operands
'''

OUTPUT_TYPES = '''\
./build/access_debug_metadata: ./inputs/types.ll:8:50: error: expected '('
define void @structadder(%struct.mystruct* byval nocapture readonly align 8 %a, %struct.mystruct* byval nocapture readonly align 8 %b, %struct.mystruct* %out) #0 {
                                                 ^
'''

PROG = 'access_debug_metadata'

class TestAccessDebugMetadata(SamplesTestCase):
    def test_diamond_cfg(self):
        self.assertSampleOutput([PROG], 'diamond-cfg.ll', '')

    def test_fact(self):
        self.assertSampleOutput([PROG], 'fact.ll', OUTPUT_FACT)

    def test_globals(self):
        self.assertSampleOutput([PROG], 'globals.ll', OUTPUT_GLOBALS)

    def test_loopy_cfg(self):
        self.assertSampleOutput([PROG], 'loopy-cfg.ll', '')

    def test_simpleswitch(self):
        self.assertSampleOutput([PROG], 'simpleswitch.ll', OUTPUT_SIMPLESWITCH)

    @unittest.expectedFailure
    def test_threadidx(self):
        self.assertSampleOutput([PROG], 'threadidx.ll', OUTPUT_THREADIDX)

    def test_twodoublefoo(self):
        self.assertSampleOutput([PROG], 'twodoublefoo.ll', OUTPUT_TWODOUBLEFOO)

    @unittest.expectedFailure
    def test_types(self):
        self.assertSampleOutput([PROG], 'types.ll', OUTPUT_TYPES)

if __name__ == '__main__':
  unittest.main()
