#include <cpu.h>
#include <bus.h>
#include <emu.h>

cpu_context ctx = {0};

void cpu_init()
{
    // 0x100 是游戏程序的入口点
    ctx.regs.pc = 0x100;
}

static void fetch_instruction()
{
    ctx.cur_opcode = bus_read(ctx.regs.pc++);
    ctx.cur_inst = instruction_by_opcode(ctx.cur_opcode);

    if (ctx.cur_inst == NULL)
    {
        printf("Unknown Instruction %02x", ctx.cur_opcode);
        exit(-7);
    }
}

static void fetch_data()
{
    ctx.mem_dest = 0;
    ctx.dest_ist_mem = false;

    switch (ctx.cur_inst->mode)
    {
    case AM_IMP: // 隐含寻址
        return;

    case AM_R: // 寄存器寻址
        ctx.fetched_data = cpu_read_reg(ctx.cur_inst->reg_1);
        return;
    case AM_R_D8: // 立即数寻址，8位
        ctx.fetched_data = bus_read(ctx.regs.pc);
        emu_cycles(1);
        ctx.regs.pc++;
        return;

    case AM_D16: // 立即数寻址，16位
        u16 lo = bus_read(ctx.regs.pc);
        emu_cycles(1);

        u16 hi = bus_read(ctx.regs.pc + 1);
        emu_cycles(1);

        ctx.fetched_data = lo | (hi << 8);

        ctx.regs.pc += 2;
        return;

    default:
        printf('Unknown Addressing Mode! %d\n', ctx.cur_inst->mode);
        exit(-7);
        return;
    }
}

static void execute()
{
    printf("Executing Instruction: %02X   PC: %04X\n", ctx.cur_opcode, ctx.regs.pc);
}

bool cpu_step()
{
    if (!ctx.halted)
    {
        fetch_instruction();
        fetch_data();
        execute();
    }

    return false;
}
