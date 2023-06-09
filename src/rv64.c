#include <stdlib.h>

#include "rv64.h"

RiscV64Cpu new_cpu(RiscV64ReadFn read, RiscV64WriteFn write, void* user_data) {
    return (RiscV64Cpu) {
        .xs = { 0 },
        .pc = 0,
        .read = read,
        .write = write,
        .user_data = user_data,
    };
}

void step(RiscV64Cpu* cpu) {
    uint32_t instruction = cpu->read(cpu, RiscVDataSizeWord, cpu->pc);
    uint32_t opcode = instruction & 0x7f;

    switch (opcode) {
        // LUI rd, imm
        case 0b0110111: {
            int rd  = (instruction >>  7) & 0b11111;
            int64_t imm = ((int32_t) instruction) & ~0xfff;
            imm = (imm << 32) >> 32;
            cpu->xs[rd] = imm;
            break;
        }

        // LUI rd, imm
        case 0b0010111: {
            int rd  = (instruction >>  7) & 0b11111;
            int64_t imm = ((int32_t) instruction) & ~0xfff;
            imm = (imm << 32) >> 32;
            cpu->xs[rd] = imm + cpu->pc;
            break;
        }

        // alu operation with immediate
        case 0b0010011: {
            int rd  = (instruction >>  7) & 0b11111;
            int rs1 = (instruction >> 15) & 0b11111;
            int64_t imm = ((int32_t) instruction) >> 20;
            int funct = (instruction >> 12) & 3;
            switch (funct) {
                // ADD rd, rs1, rs2
                case 0:
                    cpu->xs[rd] = cpu->xs[rs1] + imm;
                    break;

                // SLL rd, rs1, rs2
                case 1:
                    cpu->xs[rd] = cpu->xs[rs1] << (imm & 63);
                    break;

                // SLT rd, rs1, rs2
                case 2:
                    cpu->xs[rd] = (int64_t) cpu->xs[rs1] < imm;
                    break;

                // SLTU rd, rs1, rs2
                case 3:
                    cpu->xs[rd] = cpu->xs[rs1] < (uint64_t) imm;
                    break;

                // XOR rd, rs1, rs2
                case 4:
                    cpu->xs[rd] = cpu->xs[rs1] ^ imm;
                    break;

                // SRL rd, rs1, rs2
                case 5:
                    if (imm & 0b010000000000)
                        cpu->xs[rd] = (int64_t) cpu->xs[rs1] >> (int64_t) (imm & 63);
                    else cpu->xs[rd] = cpu->xs[rs1] >> (imm & 63);
                    break;

                // OR rd, rs1, rs2
                case 6:
                    cpu->xs[rd] = cpu->xs[rs1] | imm;
                    break;

                // AND rd, rs1, rs2
                case 7:
                    cpu->xs[rd] = cpu->xs[rs1] & imm;
                    break;

                default:
                    break;
            }
            break;
        }

        // alu operation without immediate
        case 0b0110011: {
            int rd  = (instruction >>  7) & 0b11111;
            int rs1 = (instruction >> 15) & 0b11111;
            int rs2 = (instruction >> 20) & 0b11111;
            int funct3 = (instruction >> 12) & 3;
            int funct7 = (instruction >> 25) & 7;
            int funct = (funct7 << 3) | funct3;
            switch (funct) {
                // ADD rd, rs1, rs2
                case 0:
                    cpu->xs[rd] = cpu->xs[rs1] + cpu->xs[rs2];
                    break;

                // SUB rd, rs1, rs2
                case 256:
                    cpu->xs[rd] = cpu->xs[rs1] - cpu->xs[rs2];
                    break;

                // SLL rd, rs1, rs2
                case 1:
                    cpu->xs[rd] = cpu->xs[rs1] << (cpu->xs[rs2] & 63);
                    break;

                // SLT rd, rs1, rs2
                case 2:
                    cpu->xs[rd] = (int64_t) cpu->xs[rs1] < (int64_t) cpu->xs[rs2];
                    break;

                // SLTU rd, rs1, rs2
                case 3:
                    cpu->xs[rd] = cpu->xs[rs1] < cpu->xs[rs2];
                    break;

                // XOR rd, rs1, rs2
                case 4:
                    cpu->xs[rd] = cpu->xs[rs1] ^ cpu->xs[rs2];
                    break;

                // SRL rd, rs1, rs2
                case 5:
                    cpu->xs[rd] = cpu->xs[rs1] >> (cpu->xs[rs2] & 63);
                    break;

                // SRA rd, rs1, rs2
                case 261:
                    cpu->xs[rd] = (int64_t) cpu->xs[rs1] >> (int64_t) (cpu->xs[rs2] & 63);
                    break;

                // OR rd, rs1, rs2
                case 6:
                    cpu->xs[rd] = cpu->xs[rs1] | cpu->xs[rs2];
                    break;

                // AND rd, rs1, rs2
                case 7:
                    cpu->xs[rd] = cpu->xs[rs1] & cpu->xs[rs2];
                    break;

                default:
                    break;
            }
            break;
        }

        default:
            break;
    }

    cpu->xs[0] = 0;
    cpu->pc += 4;
}
