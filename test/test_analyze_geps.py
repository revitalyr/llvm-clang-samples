import unittest
from tools import SamplesTestCase

OUTPUT_THREADIDX = '''\
./build/analyze_geps: ./inputs/threadidx.ll:12:18: error: expected comma after load's type
  %0 = load i32* getelementptr inbounds (%struct.uint3* @threadIdx, i64 0, i32 1), align 4, !tbaa !1
                 ^
'''

OUTPUT_TYPES = '''/
./build/analyze_geps: ./inputs/types.ll:8:50: error: expected '('
define void @structadder(%struct.mystruct* byval nocapture readonly align 8 %a, %struct.mystruct* byval nocapture readonly align 8 %b, %struct.mystruct* %out) #0 {
                                                 ^
'''

PROG = 'analyze_geps'

class TestAccessDebugMetadata(SamplesTestCase):
    @unittest.expectedFailure
    def test_usage(self):
        self.assertSampleOutput([PROG], '', 'Usage: build/analyze_geps <IR file>')

    def test_diamond_cfg(self):
        self.assertSampleOutput([PROG], 'diamond-cfg.ll', '')

    def test_fact(self):
        self.assertSampleOutput([PROG], 'fact.ll', '')

    def test_globals(self):
        self.assertSampleOutput([PROG], 'globals.ll', '')

    def test_loopy_cfg(self):
        self.assertSampleOutput([PROG], 'loopy-cfg.ll', '')

    def test_simpleswitch(self):
        self.assertSampleOutput([PROG], 'simpleswitch.ll', '')

    @unittest.expectedFailure
    def test_threadidx(self):
        self.assertSampleOutput([PROG], 'threadidx.ll', OUTPUT_THREADIDX)

    @unittest.expectedFailure
    def test_types(self):
        self.assertSampleOutput([PROG], 'types.ll', OUTPUT_TYPES)

if __name__ == '__main__':
  unittest.main()
