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

/* ============================================================================ */

/* -------------------------- Structure Declarations -------------------------- */

/* ============================================================================ */

typedef struct node_s {
    int value;
    struct node_s* next;
} int_node;

typedef enum {
  k_mem, // memory address
  k_temp, // temporary value
  k_write, // write
  k_reg
} dep_kind;

typedef struct {
  UWord mem_addr;
  UWord size;
} Mem_region;

typedef 
  struct dep_val_s {
    dep_kind tag;
    union {
      Mem_region var;
      UInt temp;
      int write_val;
      Int reg_val;
    } val;
    
} dep_val;

typedef struct read_dep_s {
  struct read_dep_s *next;
  dep_val val;
  int_node *read_deps;
} read_dep_node;



/* ============================================================================ */

/* ---------------------------- Global Variables  ---------------------------- */

/* ============================================================================ */

static Bool trace = False;
static HChar *trace_file_name = "output.res"; /* default output file name unless denoted */
static VgFile *fp = NULL;

static int write_n = 0;
static int read_n = 0;

// Global variable to be used when analysing the dependency of an expression
static read_dep_node *var_deps = NULL;
static read_dep_node *reg_deps = NULL;
static read_dep_node *write_deps = NULL;

static int_node*** shadow_mem;



/* ============================================================================ */

/* ---------------------------- Auxiliary Functions  -------------------------- */

/* ============================================================================ */

/* ---------------------------- Int Lists  -------------------------- */
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
    while (head != NULL) {
        temp = head;
        head = head->next;
        // VG_(printf)("Freeing int_node %d\n", temp->value);
        VG_(free)(temp);
    }
}


// Initialize with memory address
dep_val init_dep_val_with_mem_addr(UWord mem_addr, UWord size) {
    dep_val value;
    value.tag = k_mem;
    value.val.var.mem_addr = mem_addr;
    value.val.var.size = size;
    return value;
}

/* ---------------------------- List Print functions  -------------------------- */

// Print the read_deps int_node list
void print_int_list(int_node *current){
  // VG_(printf)("Read Dependencies: ");

  while (current != NULL) {
      // VG_(printf)(" read():%d", current->value);
      VG_(fprintf)(fp, " read():%d", current->value);
      current = current->next;
  }
}

void print_read_dep_list(read_dep_node* head) {
  read_dep_node* current = head;
  while (current != NULL) {
    if (current->read_deps != NULL)
    {
      // Print the tag and value based on the tag type
      if (current->val.tag == k_mem)
      {
        // VG_(printf)("addr:%lu[%lu]", current->val.val.var.mem_addr, current->val.val.var.size);
      }
      else if (current->val.tag == k_temp)
      {
        // VG_(printf)("t(%u)", current->val.val.temp);
      }
      else if (current->val.tag == k_reg)
      {
        // VG_(printf)("reg(%d)", current->val.val.reg_val);
      }
      else if (current->val.tag == k_write)
      {
        // VG_(printf)("write():%d", current->val.val.write_val);
        VG_(fprintf)(fp, "write():%d", current->val.val.write_val);
      }

      print_int_list(current->read_deps);
      VG_(fprintf)(fp, "\n");
    }

    // VG_(printf)("\n");
    current = current->next;
  }

}
/* ---------------------------- dep_val functions  -------------------------- */

// Initialize with write value
dep_val init_dep_val_with_write(int writeval) {
    dep_val value;
    value.tag = k_write;
    value.val.write_val = writeval;
    return value;
}

// Initialize with temporary value
dep_val init_dep_val_with_temp(UInt temp) {
    dep_val value;
    value.tag = k_temp;
    value.val.temp = temp;
    return value;
}

// Initialize with register value
dep_val init_dep_val_with_reg(Int reg) {
    dep_val value;
    value.tag = k_reg;
    value.val.reg_val = reg;
    return value;
}

/* ---------------------------- dep_node function ---------------------------- */
read_dep_node* create_read_dep_node(dep_val val, int_node *read_deps){
  read_dep_node* new_node = (read_dep_node*)VG_(malloc)("Dependency node", sizeof(read_dep_node));
  new_node->val = val;
  new_node->read_deps = read_deps;
  new_node->next = NULL;

  return(new_node);
}

void read_dep_node_erasetmp(UInt tmp)
{
  read_dep_node *h = var_deps;
  read_dep_node *t = h;

  while(h){
    if(h->val.tag == k_temp && h->val.val.temp == tmp){
      t->next = h->next;
      free_int_node_list(h->read_deps);
      VG_(free)(h);
      h = t;
    }
    t = h;
    h = h->next;
  }
}

void read_dep_node_erase_reg(Int reg)
{
  if(reg_deps == NULL) return;
  read_dep_node *h = reg_deps;
  read_dep_node *prev = NULL;

  // VG_(printf)("erasing reg(%d) from list: \n", reg);
  // print_read_dep_list(reg_deps);
  // VG_(printf)("\n");

  while(h){
    if(h->val.tag == k_reg && h->val.val.reg_val == reg){
      read_dep_node *del = h;
      if(prev == NULL){
        reg_deps = h->next;
        h = reg_deps;
      }
      else{
        prev->next = h->next;
        h = h->next;
      }
      free_int_node_list(del->read_deps);
      VG_(free)(del);
    }
    else {
      prev = h;
      h = h->next;
    }
  }
}


// Function to add an element to the list (backwards)
read_dep_node *read_dep_node_prepend(read_dep_node *head, dep_val dependency, int_node *read_deps)
{
  // Allocate memory for new element
  read_dep_node *new_node = create_read_dep_node(dependency, read_deps);
  new_node->next = head;

  return new_node;
}

// Function to add an element to the list
read_dep_node* read_dep_node_append(read_dep_node* head, dep_val dependency, int_node *read_deps) {
  read_dep_node *tmp = head;
  while(tmp != NULL && tmp->next != NULL){
    tmp = tmp->next;
  }

  // Allocate memory for new element
  read_dep_node *new_node = create_read_dep_node(dependency, read_deps);

  if(tmp)
    tmp->next = new_node;
  else
    head = new_node;

  return head;
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

int_node* deep_copy_int_node_list(int_node* head) {
    if (!head) return NULL;

    int_node* new_node = (int_node*)VG_(malloc)("Read Number Node", sizeof(int_node));
    if (!new_node) return NULL;  // Handle memory allocation failure

    new_node->value = head->value;
    new_node->next = deep_copy_int_node_list(head->next);

    return new_node;
}

// Function to deep copy a read_dep_node list
read_dep_node* deep_copy_read_dep_list(read_dep_node* head) {
    if (!head) return NULL;

    read_dep_node* new_node = (read_dep_node*)VG_(malloc)("Read Dependency Node", sizeof(read_dep_node));
    if (!new_node) return NULL;  // Handle memory allocation failure

    new_node->val = head->val;
    new_node->read_deps = deep_copy_int_node_list(head->read_deps);  // Deep copy the int_node list
    new_node->next = deep_copy_read_dep_list(head->next);

    return new_node;
}

static int_node* get_shadow_mem(UWord addr){
  UWord up = (((addr)&(0xFFFF0000)) >> 16);
  int_node **lookup = shadow_mem[up];
  if (lookup == NULL)
    return NULL;
  else {
    UWord low = (addr) & (0x0000FFFF);
    return lookup[low];
  }
}

// Sets the shadow mem with a deep copy of deps at addr
static void set_shadow_mem(UWord addr, int_node *deps) { 
  UWord up = (((addr)&(0xFFFF0000)) >> 16);
  UWord low = (addr)&(0x0000FFFF);

  if(shadow_mem[up] == NULL){ // on-demand allocation
    shadow_mem[up] = (int_node**)VG_(malloc)("Memory shadow", 0xFFFF * sizeof(int_node*));
    // VG_(printf)("Allocating shadow memory in i: %lu\n", up);
    for(UWord i = 0; i < 0x0000FFFF; i++)
      shadow_mem[up][i] = NULL;
  }

  if(shadow_mem[up][low] != NULL){
    free_int_node_list(shadow_mem[up][low]);
  }
  // VG_(printf)("Set shadow memory in j: %lu\n", low);
  shadow_mem[up][low] = deps;
}

void init_shadow_mem(){
  shadow_mem = (int_node***)VG_(malloc)("Memory shadow", 0xFFFF * sizeof(int_node**));
  for(int i = 0; i < 0xFFFF; i++){
    shadow_mem[i] = NULL;
  }
}

void free_shadow_mem(){
  // VG_(printf)("Begin free of shadow mem\n");
  for(UWord i = 0; i < 0x0000FFFF; i++){
    // VG_(message)(Vg_DebugMsg, "!\n");
    // VG_(printf)("Begin free of shadow mem i:%lu j:%lu\n", i, j);
    if (shadow_mem[i] != NULL){
      for(UWord j = 0; j < 0x0000FFFF; j++){
        // VG_(message)(Vg_DebugMsg, "?");
        // VG_(printf)("Begin free of shadow mem i:%lu j:%lu\n", i, j);
        if(shadow_mem[i][j] != NULL)
          free_int_node_list(shadow_mem[i][j]);
      }
      VG_(free)(shadow_mem[i]);
      // VG_(printf)("Freed all index i:%lu\n", i);
    }
  }
  // VG_(printf)("End free of shadow mem\n");
  VG_(free)(shadow_mem);
  // VG_(printf)("All Memory was Freed\n");
}

// Helper function to check if a value exists in a list
int exists_in_int_node(int_node *head, int value)
{
  while (head)
  {
    if (head->value == value)
    {
      return 1; // Value exists
    }
    head = head->next;
  }
  return 0; // Value doesn't exist
}

int_node* int_node_merge(int_node* head, int_node* l){
  if(l == NULL) return head;

  // VG_(printf)("read_dep was not null\n");
  while(l){
    // VG_(printf)("Copying dependency\n");
    if(exists_in_int_node(head, l->value) == 0){
      // VG_(printf)("Creating a new depedency with read():%d\n", l->value);
      head = int_list_append(head, l->value);
    }
    int_node* tmp = l;
    l = l->next;
    VG_(free)(tmp);
  }

  return head;
}

int_node* find_read_dep_by_mem_addr(UWord target_addr)
{
  // VG_(printf)("Finding read on target: 0x%X\n", target_addr);
  int_node *head = get_shadow_mem(target_addr);
  // VG_(printf)("Found 0x%X -> ", target_addr);
  // print_int_list(head);
  // VG_(printf)("|");
  // VG_(printf)("\n");

  return deep_copy_int_node_list(head);

  
  /*
  // VG_(printf)("Finding read on target: %lu\n", target_addr);
  while (head != NULL)
  {
    // VG_(printf)("Searching in the %d-th node\n", i);
    if (head->val.tag == k_mem)
    {
      Mem_region memreg = head->val.val.var;
      // Search in the whole size of the memory that was written
      for(UWord i = 0; i < memreg.size; i++){
        // VG_(printf)("Searching for read dependency in region %lu\n", memreg.mem_addr+i);
        if(memreg.mem_addr + i == target_addr){
          // VG_(printf)("Found a read dependency in region %lu\n", memreg.mem_addr+i);
          new_node = int_node_merge(new_node, head->read_deps);
        }
      }
    }
    i++;
    head = head->next;
  }
  return new_node;
  */
}

int_node* find_read_dep_by_mem_addrs(UWord target_addr, UWord size){
  int_node *tmp = NULL;
  int_node *head = NULL;
  // VG_(printf)("Finding mem addr at 0x%X with size %lu\n", target_addr, size);
  for (UWord i = 0; i < size; i++){
    tmp = find_read_dep_by_mem_addr(target_addr+i);
    // VG_(printf)("Search is over, begin final merge\n");
    head = int_node_merge(head, tmp);
  }
  // if(head != NULL)
  //   VG_(printf)("Returning a non_empty head for find_by_mem_addres\n");


  return head;
}

int_node* find_read_dep_tmp(UInt tmp){
 read_dep_node *head = var_deps;
 while(head){
  if(head->val.tag == k_temp && head->val.val.temp == tmp){
    return (head->read_deps);
  }
  head = head->next;
 }
 return NULL;
}

int_node* find_read_dep_reg(Int reg){
 read_dep_node *head = reg_deps;
 while(head){
  if(head->val.tag == k_reg && head->val.val.reg_val == reg){
    return (head->read_deps);
  }
  head = head->next;
 }
 return NULL;
}



/* ============================================================================ */

/* ---------------------------- Dependency Analysis  -------------------------- */

/* ============================================================================ */

static void instrument_assgn_tmp_load(UInt tmp_n, Addr addr)
{
  // VG_(printf)("t(%u) = load:0x%X\n", tmp_n, addr);

  int_node *l = get_shadow_mem(addr);
  if (l == NULL) return;

  dep_val val = init_dep_val_with_temp(tmp_n);
  // VG_(printf)("t(%u) = load:0x%X", tmp_n, addr);
  // print_int_list(l);
  // VG_(printf)("\n");
  l = deep_copy_int_node_list(l);
  var_deps = read_dep_node_append(var_deps, val, l);
}

/*
static void instrument_load(Addr addr)
{
  int_node *l = get_shadow_mem(addr);
  if (l == NULL) return;

  dep_val val = init_dep_val_with_temp(tmp_n);
  //VG_(printf)("t(%u) = load:0x%X\n", tmp_n, addr);
  expr_deps = deep_copy_int_node_list(l);
  // var_deps = read_dep_node_append(var_deps, val, l);
}
*/

int_node* get_deps_tmp(UInt tmp)
{
  int_node *l = find_read_dep_tmp(tmp);
  if (l == NULL) return NULL;

  return (deep_copy_int_node_list(l));
}

int_node* get_deps_reg(Int reg)
{
  int_node *l = find_read_dep_reg(reg);
  if (l == NULL) return NULL;

  return (deep_copy_int_node_list(l));
}

static void instrument_assgn_tmp_const(UInt tmp)
{
  // VG_(printf)("t(%u) = UNOP Const\n", tmp);
  read_dep_node_erasetmp(tmp);
}

static void instrument_assgn_tmp_unop(UInt tmp_lhs, UInt tmp_rhs)
{
  int_node *l = get_deps_tmp(tmp_rhs);
  if (l == NULL) return;

  // VG_(printf)("t(%u) = UNOP t(%u)\n", tmp_lhs, tmp_rhs);
  dep_val val = init_dep_val_with_temp(tmp_lhs);
  var_deps = read_dep_node_prepend(var_deps, val, l);
}

static void instrument_assgn_tmp_binop(UInt tmp_lhs, UInt tmp1, UInt tmp2)
{
  int_node *l1 = get_deps_tmp(tmp1);
  int_node *l2 = get_deps_tmp(tmp2);
  int_node *l = int_node_merge(l1, l2);
  if(l == NULL) return;

  // VG_(printf)("t(%u) = t(%u), t(%u)\n", tmp_lhs, tmp1, tmp2);
  dep_val val = init_dep_val_with_temp(tmp_lhs);
  var_deps = read_dep_node_prepend(var_deps, val, l);
}
static void instrument_assgn_tmp_triop(UInt tmp_lhs, UInt tmp1, UInt tmp2, UInt tmp3)
{
  int_node *l1 = get_deps_tmp(tmp1);
  int_node *l2 = get_deps_tmp(tmp2);
  int_node *l3 = get_deps_tmp(tmp3);
  int_node *l = int_node_merge(l1, l2);
  l = int_node_merge(l, l3);
  if(l == NULL) return;

  // VG_(printf)("t(%u) = t(%u), t(%u), t(%u)\n", tmp_lhs, tmp1, tmp2, tmp3);

  dep_val val = init_dep_val_with_temp(tmp_lhs);
  var_deps = read_dep_node_prepend(var_deps, val, l);
}
static void instrument_assgn_tmp_qop(UInt tmp_lhs, UInt tmp1, UInt tmp2, UInt tmp3, UInt tmp4)
{
  int_node *l1 = get_deps_tmp(tmp1);
  int_node *l2 = get_deps_tmp(tmp2);
  int_node *l3 = get_deps_tmp(tmp3);
  int_node *l4 = get_deps_tmp(tmp4);
  int_node *l = int_node_merge(l1, l2);
  l = int_node_merge(l, l3);
  l = int_node_merge(l, l4);
  if(l == NULL) return;

  // VG_(printf)("t(%u) = t(%u), t(%u), t(%u), t(%u)\n", tmp_lhs, tmp1, tmp2, tmp3, tmp4);

  dep_val val = init_dep_val_with_temp(tmp_lhs);
  l = deep_copy_int_node_list(l);
  var_deps = read_dep_node_append(var_deps, val, l);
}


int_node* get_deps_mem (Addr m)
{
  int_node *l = get_shadow_mem(m);
  if (l == NULL) return NULL;
  l = deep_copy_int_node_list(l);
  return l;
}


static void instrument_store_const(Addr addr)
{

  // VG_(printf)("ST(0x%X) = CONST\n", addr);
  set_shadow_mem(addr, NULL);
}

static void instrument_store_tmp(Addr addr, UInt tmp)
{

  int_node *l = get_deps_tmp(tmp);
  if (l == NULL) return;

  // VG_(printf)("ST(0x%X) = t(%u)\n", addr, tmp);
  // print_int_list(l);
  set_shadow_mem(addr, l);
}

static void instrument_put_tmp(Int reg, UInt tmp)
{
  int_node *l = get_deps_tmp(tmp);
  if (l == NULL) return;

  // VG_(printf)("PUT(0x%X) = t(%u)\n", reg, tmp);
  // print_int_list(l);
  read_dep_node_erase_reg(reg);

  dep_val val = init_dep_val_with_reg(reg);
  reg_deps = read_dep_node_prepend(reg_deps, val, l);

}

static void instrument_put_cnst(Int reg)
{
  // VG_(printf)("PUT(0x%X) = Const\n", reg);
  read_dep_node_erase_reg(reg);
}


static void instrument_assgn_tmp_get(UInt tmp, Int reg)
{
  int_node *l = get_deps_reg(reg);
  if (l == NULL) return;

  // VG_(printf)("t(%u) = GET(0x%X)\n", tmp, reg);

  dep_val val = init_dep_val_with_temp(tmp);
  var_deps = read_dep_node_prepend(var_deps, val, l);

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
  init_shadow_mem();

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

  for (i = 0; i < sbIn->stmts_used; i++)
  {
    dirty = NULL;
    IRStmt *st = sbIn->stmts[i];

    if (!st || st->tag == Ist_NoOp)
      continue;

    switch (st->tag)
    {
    case Ist_IMark:

      // Restart the count of the intermediate variables

      if (VG_(get_fnname_if_entry)(ep, st->Ist.IMark.addr, &fnname))
      {

        if (VG_(strcmp)(fnname, "read") == 0)
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
      if(!trace) break;

      data = st->Ist.WrTmp.data;
      IRTemp tmp = st->Ist.WrTmp.tmp;
      IRType load_type = typeOfIRExpr(tyenv, data);

      IRExpr *e = IRExpr_Const(IRConst_U64(tmp));
      IRExpr *arg1, *arg2, *arg3, *arg4;
      switch(data->tag){
        case Iex_RdTmp:
          argv = mkIRExprVec_2(e, IRExpr_Const(IRConst_U64(data->Iex.RdTmp.tmp)));
          dirty = unsafeIRDirty_0_N(2, "ta_ld", VG_(fnptr_to_fnentry)(instrument_assgn_tmp_unop), argv);
          addStmtToIRSB(sbOut, IRStmt_Dirty(dirty));
          break;
        case Iex_Load:
          argv = mkIRExprVec_2(e, data->Iex.Load.addr);
          dirty = unsafeIRDirty_0_N(2, "ta_ld", VG_(fnptr_to_fnentry)(instrument_assgn_tmp_load), argv);
          addStmtToIRSB(sbOut, IRStmt_Dirty(dirty));
          break;
        case Iex_Get:
          argv = mkIRExprVec_2(e, IRExpr_Const(IRConst_U64(data->Iex.Get.offset)));
          dirty = unsafeIRDirty_0_N(2, "ta_ld", VG_(fnptr_to_fnentry)(instrument_assgn_tmp_get), argv);
          addStmtToIRSB(sbOut, IRStmt_Dirty(dirty));
          break;
        case Iex_Unop:
          arg1 = data->Iex.Unop.arg;
          if(arg1->tag == Iex_RdTmp){
            argv = mkIRExprVec_2(e, IRExpr_Const(IRConst_U64(arg1->Iex.RdTmp.tmp)));
            dirty = unsafeIRDirty_0_N(2, "ta_unop", VG_(fnptr_to_fnentry)(instrument_assgn_tmp_unop), argv);
            addStmtToIRSB(sbOut, IRStmt_Dirty(dirty));
          }
          if(arg1->tag == Iex_Const){
            argv = mkIRExprVec_1(e);
            dirty = unsafeIRDirty_0_N(1, "ta_unop", VG_(fnptr_to_fnentry)(instrument_assgn_tmp_const), argv);
            addStmtToIRSB(sbOut, IRStmt_Dirty(dirty));
          }
          break;
        case Iex_Binop:
          arg1 = data->Iex.Binop.arg1;
          arg2 = data->Iex.Binop.arg2;
          // Both Temporaries
          if(arg1->tag == Iex_RdTmp && arg2->tag == Iex_RdTmp){
            argv = mkIRExprVec_3(e,
                                 IRExpr_Const(IRConst_U64(arg1->Iex.RdTmp.tmp)),
                                 IRExpr_Const(IRConst_U64(arg2->Iex.RdTmp.tmp)));

            dirty = unsafeIRDirty_0_N(3, "ta_binop", VG_(fnptr_to_fnentry)(instrument_assgn_tmp_binop), argv);
            addStmtToIRSB(sbOut, IRStmt_Dirty(dirty));
          }
          // arg1 is temporary, arg2 is const
          else if(arg1->tag == Iex_RdTmp && arg2->tag == Iex_Const){
            argv = mkIRExprVec_2(e, IRExpr_Const(IRConst_U64(arg1->Iex.RdTmp.tmp)));
            dirty = unsafeIRDirty_0_N(2, "ta_unop", VG_(fnptr_to_fnentry)(instrument_assgn_tmp_unop), argv);
            addStmtToIRSB(sbOut, IRStmt_Dirty(dirty));
          }
          // arg1 is const, arg2 is temporary
          else if(arg1->tag == Iex_Const && arg2->tag == Iex_RdTmp){
            argv = mkIRExprVec_2(e, IRExpr_Const(IRConst_U64(arg2->Iex.RdTmp.tmp)));
            dirty = unsafeIRDirty_0_N(2, "ta_unop", VG_(fnptr_to_fnentry)(instrument_assgn_tmp_unop), argv);
            addStmtToIRSB(sbOut, IRStmt_Dirty(dirty));
          }
          // both are const
          else if(arg1->tag == Iex_Const && arg2->tag == Iex_Const){
            argv = mkIRExprVec_1(e);
            dirty = unsafeIRDirty_0_N(1, "ta_unop", VG_(fnptr_to_fnentry)(instrument_assgn_tmp_const), argv);
            addStmtToIRSB(sbOut, IRStmt_Dirty(dirty));
          }
          // rhs is temporary
          break;
        case Iex_Triop:
          arg1 = data->Iex.Triop.details->arg1;
          arg2 = data->Iex.Triop.details->arg2;
          arg3 = data->Iex.Triop.details->arg3;
          if(arg1->tag == Iex_RdTmp && arg2->tag == Iex_RdTmp && arg3->tag == Iex_RdTmp){
            argv = mkIRExprVec_4(e, IRExpr_Const(IRConst_U64(arg1->Iex.RdTmp.tmp)),
                                 IRExpr_Const(IRConst_U64(arg2->Iex.RdTmp.tmp)),
                                 IRExpr_Const(IRConst_U64(arg3->Iex.RdTmp.tmp)));
            dirty = unsafeIRDirty_0_N(4, "ta_triop", VG_(fnptr_to_fnentry)(instrument_assgn_tmp_triop), argv);
            addStmtToIRSB(sbOut, IRStmt_Dirty(dirty));
          }
          break;
        case Iex_Qop:
          arg1 = data->Iex.Qop.details->arg1;
          arg2 = data->Iex.Qop.details->arg2;
          arg3 = data->Iex.Qop.details->arg3;
          arg4 = data->Iex.Qop.details->arg4;
          if(arg1->tag == Iex_RdTmp && arg2->tag == Iex_RdTmp && arg3->tag == Iex_RdTmp && arg4->tag == Iex_RdTmp){
            argv = mkIRExprVec_5(e, IRExpr_Const(IRConst_U64(arg1->Iex.RdTmp.tmp)),
                                 IRExpr_Const(IRConst_U64(arg2->Iex.RdTmp.tmp)),
                                 IRExpr_Const(IRConst_U64(arg3->Iex.RdTmp.tmp)),
                                 IRExpr_Const(IRConst_U64(arg4->Iex.RdTmp.tmp)));

            dirty = unsafeIRDirty_0_N(5, "ta_qop", VG_(fnptr_to_fnentry)(instrument_assgn_tmp_qop), argv);
            addStmtToIRSB(sbOut, IRStmt_Dirty(dirty));
          }
          break;
        default:
          break;

      }

      break;

    case Ist_Store:
      if(!trace) break;

      data = st->Ist.Store.data;
      IRExpr* addr = st->Ist.Store.addr;

      IRType write_type = typeOfIRExpr(tyenv, data);

      if (data->tag == Iex_Const)
      {
        argv = mkIRExprVec_1(addr);
        dirty = unsafeIRDirty_0_N(1, "ta_st_cnst", VG_(fnptr_to_fnentry)(instrument_store_const), argv);
        addStmtToIRSB(sbOut, IRStmt_Dirty(dirty));
      }

      else if (data->tag == Iex_RdTmp)
      {
        IRExpr *tmp = IRExpr_Const(IRConst_U64(data->Iex.RdTmp.tmp));
        argv = mkIRExprVec_2(addr, tmp);
        dirty = unsafeIRDirty_0_N(2, "ta_st_rdtmp", VG_(fnptr_to_fnentry)(instrument_store_tmp), argv);
        addStmtToIRSB(sbOut, IRStmt_Dirty(dirty));
      }

      break;
    case Ist_Put:
      if(!trace) break;
      data = st->Ist.Put.data; // Temporary and Const
      Int offset = st->Ist.Put.offset;
      IRExpr *reg = IRExpr_Const(IRConst_U64(offset));

      if(data->tag == Iex_RdTmp){
        IRExpr *tmp = IRExpr_Const(IRConst_U64(data->Iex.RdTmp.tmp));
        argv = mkIRExprVec_2(reg, tmp);
        dirty = unsafeIRDirty_0_N(2, "ta_st_cnst", VG_(fnptr_to_fnentry)(instrument_put_tmp), argv);
        addStmtToIRSB(sbOut, IRStmt_Dirty(dirty));
      }
      else if(data->tag == Iex_Const){
        argv = mkIRExprVec_1(reg);
        dirty = unsafeIRDirty_0_N(1, "ta_st_cnst", VG_(fnptr_to_fnentry)(instrument_put_cnst), argv);
        addStmtToIRSB(sbOut, IRStmt_Dirty(dirty));
      }

      break;
    default: break;
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
  // VG_(printf)("Finished instrumenting\n");

  return sbOut;
}

#define __NR_read 0
#define __NR_write 1

static void da_fini(Int exitcode)
{

  // VG_(printf)("Printing read_dep_list\n");
  // print_read_dep_list(reg_deps);
  // print_read_dep_list(var_deps);
  // print_read_dep_list(write_deps);
  print_read_dep_list(write_deps);

  // VG_(printf)("Before free var_deps\n");
  free_read_dep_list(reg_deps);
  free_read_dep_list(var_deps);
  // VG_(printf)("Before write_deps\n");
  free_read_dep_list(write_deps);
  // VG_(printf)("After frees\n");

  free_shadow_mem();

  // VG_(printf)("Closing file file\n");
  VG_(fclose)(fp);
}

static void ta_pre_call(ThreadId id, UInt syscallno, UWord *args, UInt nargs)
{
  if (trace)
  {
    if (syscallno == __NR_read)
    {
      read_n++;
      for(int i = 0; i < args[2]; i++){
        int_node *int_list = int_list_create(read_n);
        set_shadow_mem(args[1]+i, int_list);
      }
    }
    if (syscallno == __NR_write)
    {
      write_n++;

      // VG_(printf)("Finding read addr...\n");
      int_node *read_dep = find_read_dep_by_mem_addrs(args[1], args[2]);
      dep_val depval = init_dep_val_with_write(write_n);
      write_deps = read_dep_node_append(write_deps, depval, read_dep);
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
