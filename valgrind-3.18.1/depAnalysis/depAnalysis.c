/*--------------------------------------------------------------------*/
/*--- depAnalysis: The minimal Valgrind tool.        depAnalysis.c ---*/
/*--------------------------------------------------------------------*/

/*
   Created for CS510, Proj1, code is based on Nulgrind.

   This file is part of Nulgrind, the minimal Valgrind tool,
   which does no instrumentation or analysis.

   Copyright (C) 2002-2017 Nicholas Nethercote
      njn@valgrind.org

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <http://www.gnu.org/licenses/>.

   The GNU General Public License is contained in the file COPYING.
*/

#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_debuginfo.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_options.h"
#include "pub_tool_machine.h"
#include "pub_tool_libcfile.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_vki.h"
#include "pub_tool_libcassert.h"

static Bool trace = False;
static HChar *trace_file_name = "output.res"; /* default output file name unless denoted */
static VgFile *fp = NULL;

// static Bool** table;
// static Bool* tempshadow;

static int write_n = 0;
static int read_n = 0;
// Node structure
typedef struct node_s {
    int value;
    struct node_s* next;
} int_node;

int_node* int_list_create(int value) {
    int_node* new_node = (int_node*)VG_(malloc)("Read Number Node", sizeof(int_node));
    if (!new_node) {
        return NULL; 
    }
    new_node->value = value;
    new_node->next = NULL;
    return new_node;
}

int_node* int_list_append(int_node* head, int value) {
    int_node* new_node = int_list_create(value);
    if (!new_node) {
        return head;  // Return existing list if we failed to create a new node
    }

    if (!head) {
        return new_node;  // New node becomes the list if list was empty
    }

    int_node* temp = head;
    while (temp->next) {
        temp = temp->next;
    }
    temp->next = new_node;
    return head;
}

void free_int_node_list(int_node* head) {
    int_node* temp;
    while (head) {
        temp = head;
        head = head->next;
        VG_(free)(temp);
    }
}

typedef enum {
  k_word, // memory address
  k_temp // temporary value
} dep_kind;

typedef 
  struct dep_val_s {
    dep_kind tag;
    union {
      UWord mem_addr;
      IRTemp temp;
    } val;
    
} dep_val;

// Initialize with memory address
dep_val init_dep_val_with_mem_addr(UWord mem_addr) {
    dep_val value;
    value.tag = k_word;
    value.val.mem_addr = mem_addr;
    return value;
}

// Initialize with temporary value
dep_val init_dep_val_with_temp(IRTemp temp) {
    dep_val value;
    value.tag = k_temp;
    value.val.temp = temp;
    return value;
}

typedef struct read_dep_s {
  struct read_dep_s *next;
  dep_val val;
  int_node *read_deps;
} read_dep_node;

read_dep_node* create_read_dep_node(dep_val val, int *read_deps){
  read_dep_node* new_node = (read_dep_node*)VG_(malloc)("Dependency node", sizeof(read_dep_node));
  new_node->val = val;
  new_node->read_deps = read_deps;

  return(new_node);
}

// Function to add an element to the list (backwards)
read_dep_node* add_to_read_dep_list(read_dep_node* head, dep_val dependency, int *read_deps) {
    // Allocate memory for new element
    read_dep_node* new_node = create_read_dep_node(dependency, read_deps);
    new_node->next = head;

    return new_node;
}

// Function to free the read_dep_node list
void free_read_dep_list(read_dep_node* head) {
    read_dep_node* temp;
    while (head) {
        free_int_node_list(head->read_deps);  // free the int_node list
        temp = head;
        head = head->next;
        VG_(free)(temp);
    }
}


static read_dep_node *read_dependencies = NULL;

static void instrument_load_statement(Addr pc, Addr addr)
{
  // VG_(printf)("load:%x\r\n", addr);
  // Place your code here
}

static void instrument_store_statement(Addr pc, Addr addr)
{
  // VG_(printf)("store: %x\r\n", addr);
  // Place your code here
}

static Bool da_process_cmd_line_option(const HChar *arg)
{
  if VG_STR_CLO (arg, "--trace-file", trace_file_name)
  {
  }
  else
  {
    return False;
  }
  return True;
}

static void da_print_usage(void)
{
  VG_(printf)
  (
      "    --trace_file==<name>     file name for trace log [output.txt]\n");
  VG_(printf)
  (
      "   this is me messing with you ;)\n");
}
static void da_print_debug_usage(void)
{
  VG_(printf)
  ("    (none)\n");
}


static void da_post_clo_init(void)
{

}

// It maybe helpful for your work
static Int type2index(IRType ty)
{
  switch (ty)
  {
  case Ity_I1:
    return 0;
  case Ity_I8:
    return 1;
  case Ity_I16:
    return 2;
  case Ity_I32:
    return 3;
  case Ity_I64:
    return 4;
  case Ity_I128:
    return 5;
  case Ity_F32:
    return 6;
  case Ity_F64:
    return 7;
  case Ity_F128:
    return 8;
  case Ity_V128:
    return 9;
  case Ity_V256:
    return 10;
  case Ity_D32:
    return 11;
  case Ity_D64:
    return 12;
  case Ity_D128:
    return 13;
  default:
    return 14;
  }
}

static IRSB *da_instrument(VgCallbackClosure *closure,
                           IRSB *sbIn,
                           const VexGuestLayout *layout,
                           const VexGuestExtents *vge,
                           const VexArchInfo *archinfo_host,
                           IRType gWordTy, IRType hWordTy)
{

  IRDirty *dirty;
  Int i;
  IRSB *sbOut;
  IRTypeEnv *tyenv = sbIn->tyenv;
  Addr iaddr = 0, dst;
  UInt ilen = 0;
  Bool condition_inverted = False;
  DiEpoch ep = VG_(current_DiEpoch)();

  IRExpr *data = NULL;
  IRExpr *PCdata;
  IRExpr **argv;

  HChar *filename;
  UInt linenum;

  HChar *fnname;
  sbOut = deepCopyIRSBExceptStmts(sbIn);

  if (NULL == fp)
  {
    fp = VG_(fopen)(trace_file_name,
                    VKI_O_CREAT | VKI_O_WRONLY | VKI_O_TRUNC,
                    VKI_S_IRUSR | VKI_S_IWUSR | VKI_S_IRGRP | VKI_S_IROTH);
  }
  VG_(fprintf)(fp, "file written successfuly!");

  for (i = 0; i < sbIn->stmts_used; i++)
  {

    IRStmt *st = sbIn->stmts[i];

    if (!st || st->tag == Ist_NoOp)
      continue;

    switch (st->tag)
    {
    case Ist_IMark:

      if (VG_(get_fnname_if_entry)(ep, st->Ist.IMark.addr, &fnname))
      {

        if (VG_(strcmp)(fnname, "main") == 0)
        {
          trace = True;
        }

        if (VG_(strcmp)(fnname, "exit") == 0)
        {
          trace = False;
        }
      }
      if (trace)
        PCdata = mkIRExpr_HWord(st->Ist.IMark.addr);
      break;
      // You may want to modify this part of code
    case Ist_WrTmp:
      data = st->Ist.WrTmp.data;
      IRType load_type = typeOfIRExpr(sbOut->tyenv, data);

      if (trace && data->tag == Iex_Load)
      {

        argv = mkIRExprVec_2(PCdata, data->Iex.Load.addr);
        dirty = unsafeIRDirty_0_N(2, "ta_ld", VG_(fnptr_to_fnentry)(instrument_load_statement), argv);

        addStmtToIRSB(sbOut, IRStmt_Dirty(dirty));
      }

      break;

    case Ist_Store:

      data = st->Ist.Store.data;

      IRType wrtie_type = typeOfIRExpr(tyenv, data);

      if (trace)
      {
        argv = mkIRExprVec_2(PCdata, st->Ist.Store.addr);
        dirty = unsafeIRDirty_0_N(2, "ta_st", VG_(fnptr_to_fnentry)(instrument_store_statement), argv);

        addStmtToIRSB(sbOut, IRStmt_Dirty(dirty));
      }

      break;
    }
    addStmtToIRSB(sbOut, st);

    /*
    if (trace)
    {
      ppIRStmt(st);
      VG_(printf)("\n");
    }
    */
  }
  return sbOut;
}

#define __NR_read 0
#define __NR_write 1

static void da_fini(Int exitcode)
{

  free_read_dep_list(read_dependencies);

  VG_(fclose)(fp);
  VG_(printf)("We are done");
}

static void ta_pre_call(ThreadId id, UInt syscallno, UWord *args, UInt nargs)
{
  if (trace)
  {
    if (syscallno == __NR_read)
    {
      /*
      VG_(printf)("read( ");
      for(int i = 0; i < nargs; i++)
        VG_(printf)("0x%X, ", args[i]);
      VG_(printf)(" )\n");
      */
      read_n++;
    }
    if (syscallno == __NR_write)
    {
      /*
      VG_(printf)("write( ");
      for(int i = 0; i < nargs; i++)
        VG_(printf)("0x%X, ", args[i]);
      VG_(printf)(" )\n");
      */
      write_n++;
    }
  }
}

static void ta_post_call(ThreadId id, UInt syscallno, UWord *args, UInt nargs)
{
  ;
}

static void da_pre_clo_init(void)
{
  // VG_(printf) ("ENTERED preinit\n");
  VG_(details_name)
  ("depAnalysis");
  VG_(details_version)
  (NULL);
  VG_(details_description)
  ("the normal minimal Valgrind tool for taint analysis");
  VG_(details_copyright_author)
  (
      "Copyright (C) 2002-2017, and GNU GPL'd, by Pedro Abreu. (jk, no copyright here)");
  VG_(details_bug_reports_to)
  (VG_BUGS_TO);
  VG_(details_avg_translation_sizeB)
  (200);

  VG_(basic_tool_funcs)
  (da_post_clo_init,
   da_instrument,
   da_fini);
  VG_(needs_command_line_options)
  (da_process_cmd_line_option,
   da_print_usage,
   da_print_debug_usage);
  VG_(needs_var_info)
  ();
  /* No needs, no core events to track */

  VG_(needs_syscall_wrapper)(ta_pre_call, ta_post_call);
}

VG_DETERMINE_INTERFACE_VERSION(da_pre_clo_init)

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/
