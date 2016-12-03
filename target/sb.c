#include <ir/ir.h>
#include <target/util.h>

static const char* SB_REG_NAMES[] = {
  "A", "B", "C", "D", "BP", "SP", "PC"
};

static void init_state_sb(Data* data) {
  reg_names = SB_REG_NAMES;

  for (int i = 0; i < 7; i++) {
    emit_line("VAR %s=0", reg_names[i]);
  }

  emit_line("DIM MEM[1<<20]");
  for (int mp = 0; data; data = data->next, mp++) {
    if (data->v) {
      emit_line("MEM[%d]=%d", mp, data->v);
    }
  }
  emit_line("GOTO@MAIN");
}


static void sb_emit_func_prologue(int func_id) {
  emit_line("DEF F%d", func_id);
  emit_line("WHILE %d<=PC&&PC<%d", func_id * CHUNKED_FUNC_SIZE, (func_id + 1) * CHUNKED_FUNC_SIZE);
  emit_line("GOTO\"@\"+STR$(PC+1)");
 
}

static void sb_emit_func_epilogue(void) {
  emit_line("@_");
  emit_line("INC PC");
  emit_line("WEND");
  emit_line("END");
}

static void sb_emit_pc_change(int pc) {
  emit_line("GOTO@_");
  emit_line("@%d", pc + 1);
}

static void sb_emit_inst(Inst* inst) {
  switch (inst->op) {
  case MOV:
    emit_line("%s=%s", reg_names[inst->dst.reg], src_str(inst));
    break;

  case ADD:
    emit_line("%s=(%s+%s)AND " UINT_MAX_STR,
              reg_names[inst->dst.reg],
              reg_names[inst->dst.reg],
              src_str(inst));
    break;

  case SUB:
    emit_line("%s=(%s-%s)AND " UINT_MAX_STR,
              reg_names[inst->dst.reg],
              reg_names[inst->dst.reg],
              src_str(inst));
    break;

  case LOAD:
    emit_line("%s=MEM[%s]",
              reg_names[inst->dst.reg], src_str(inst));
    break;

  case STORE:
    emit_line("MEM[%s]=%s", src_str(inst), reg_names[inst->dst.reg]);
    break;

  case PUTC:
    emit_line("?CHR$(%s);", src_str(inst));
    break;

  case GETC:
    emit_line("%s=GETC()",
              reg_names[inst->dst.reg]);
    break;

  case EXIT:
    emit_line("PC=-2");
    break;

  case DUMP:
    break;

  case EQ:
  case NE:
  case LT:
  case GT:
  case LE:
  case GE:
    emit_line("%s=%s",
              reg_names[inst->dst.reg], cmp_str(inst, "1"));
    break;

  case JEQ:
  case JNE:
  case JLT:
  case JGT:
  case JLE:
  case JGE:
  case JMP:
    emit_line("IF %s THEN PC=%s-1",
              cmp_str(inst, "1"), value_str(&inst->jmp));
    break;

  default:
    error("oops");
  }
}

void target_sb(Module* module) {
  init_state_sb(module->data);

  /*int num_funcs = */emit_chunked_main_loop(module->text,
                                         sb_emit_func_prologue,
                                         sb_emit_func_epilogue,
                                         sb_emit_pc_change,
                                         sb_emit_inst);

  emit_line("");
  emit_line("DEF GETC()WHILE 1VAR INK$=INKEY$()IF LEN(INK$)THEN RETURN ASC(INK$)ENDIF:WEND:END");
  emit_line("@MAIN");
  emit_line("CALL\"F\"+STR$(PC DIV %dOR 0)", CHUNKED_FUNC_SIZE);
  emit_line("IF PC<0 THEN END");
  emit_line("GOTO@MAIN");
}
