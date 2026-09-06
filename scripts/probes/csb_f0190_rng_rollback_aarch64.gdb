# AArch64 native runtime regression observation, not original-media evidence.
set pagination off
set confirm off
set debuginfod enabled off
python
import gdb
checks = 0
failures = 0
def u32(addr):
    return int.from_bytes(gdb.selected_inferior().read_memory(addr, 4).tobytes(), 'little')
class Returned(gdb.FinishBreakpoint):
    def __init__(self, addr, seed):
        super().__init__(gdb.newest_frame(), internal=True)
        self.addr, self.seed = addr, seed
    def stop(self):
        global checks, failures
        if int(gdb.parse_and_eval('$x0')) == 0:
            checks += 1
            after = u32(self.addr)
            if after != self.seed:
                failures += 1
            gdb.write('ROLLBACK_RNG before=%08x after=%08x\n' % (self.seed, after))
        return False
class Entered(gdb.Breakpoint):
    def stop(self):
        # AAPCS64: ninth argument is the RNG pointer in the entry stack slot.
        addr = int(gdb.parse_and_eval('*(unsigned long *)$sp'))
        if addr:
            Returned(addr, u32(addr))
        return False
Entered('*csb_v1_runtime_pack_dead_group_creature', internal=True)
end
run
python
gdb.write('ROLLBACK_RNG checks=%d failures=%d\n' % (checks, failures))
if checks == 0 or failures:
    gdb.execute('quit 1')
end
