#ifndef SEMANTICS_H
#define SEMANTICS_H

#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "ParserTables.h"
#include "Scope.h"
#include "Util.h"

using namespace std;

#define IS_TYPE_KIND(k)                                               \
  ((k) == ARRAY_TYPE_ || (k) == STRUCT_TYPE_ || (k) == ALIAS_TYPE_ || \
   (k) == SCALAR_TYPE_)

struct SemaObj;
struct SemaAttr;

struct VariantObjTypeble {
  shared_ptr<SemaObj> type;
  int n_idx;
  int n_size;
};

struct VariantObjFunc {
  shared_ptr<SemaObj> ret_type;
  vector<shared_ptr<SemaObj>> params;
  int n_idx;
  int n_params;
  int n_vars;
};

struct VariantObjArray {
  shared_ptr<SemaObj> el_type;
  int num_el;
  int n_size;
};

struct VariantObjStruct {
  vector<shared_ptr<SemaObj>> fields;
  int n_size;
};

struct VariantObjAlias {
  shared_ptr<SemaObj> base_type;
  int n_size;
};

enum KindType {
  NO_KIND_DEF_ = -1,
  VAR_,
  PARAM_,
  FUNCTION_,
  FIELD_,
  ARRAY_TYPE_,
  STRUCT_TYPE_,
  ALIAS_TYPE_,
  SCALAR_TYPE_,
  UNIVERSAL_
};

typedef std::variant<std::monostate, VariantObjTypeble, VariantObjFunc,
                     VariantObjArray, VariantObjStruct, VariantObjAlias>
    SemaObjVariant;

struct SemaObj {
  int name;
  KindType k_type;
  SemaObjVariant obj;

  SemaObj(int name, KindType k_type, SemaObjVariant obj)
      : name(name), k_type(k_type), obj(obj) {}
};

struct VariantAttrId {
  shared_ptr<SemaObj> obj;
  int name;
};

struct VariantAttrTypeble {
  shared_ptr<SemaObj> type;
};

struct VariantAttrMc {
  shared_ptr<SemaObj> type;
  vector<shared_ptr<SemaObj>> param;
  int err;
};

struct VariantAttrLabel {
  int label;
};

struct VariantAttrLe {
  shared_ptr<SemaObj> type;
  vector<shared_ptr<SemaObj>> param;
  int err;
  int n;
};

struct VariantAttrList {
  vector<shared_ptr<SemaObj>> list;
};

struct VariantAttrBool {
  shared_ptr<SemaObj> type;
  bool val;
};

struct VariantAttrChar {
  shared_ptr<SemaObj> type;
  int pos;
  char val;
};

struct VariantAttrString {
  shared_ptr<SemaObj> type;
  int pos;
  std::string val;
};

struct VariantAttrNum {
  shared_ptr<SemaObj> type;
  int pos;
  int val;
};

typedef std::variant<std::monostate, VariantAttrId, VariantAttrTypeble,
                     VariantAttrMc, VariantAttrLabel, VariantAttrLe,
                     VariantAttrList, VariantAttrBool, VariantAttrChar,
                     VariantAttrString, VariantAttrNum>
    SemaAttrVariant;

struct SemaAttr {
  int size;
  non_term type;
  SemaAttrVariant attr;

  SemaAttr(int size, non_term type, SemaAttrVariant attr)
      : size(size), type(type), attr(attr) {}
};

class Semantics {
 public:
  int err = false;
  SymbolTable symtable;
  stack<SemaAttr> sema_stack;
  shared_ptr<SemaObj> int_t =
      make_shared<SemaObj>(-1, SCALAR_TYPE_, std::monostate());
  shared_ptr<SemaObj> bool_t =
      make_shared<SemaObj>(-1, SCALAR_TYPE_, std::monostate());
  shared_ptr<SemaObj> char_t =
      make_shared<SemaObj>(-1, SCALAR_TYPE_, std::monostate());
  shared_ptr<SemaObj> string_t =
      make_shared<SemaObj>(-1, SCALAR_TYPE_, std::monostate());
  shared_ptr<SemaObj> universal_t =
      make_shared<SemaObj>(-1, SCALAR_TYPE_, std::monostate());
  FILE *file;
  Semantics();
  ~Semantics();
  int check_types(shared_ptr<SemaObj> t1, shared_ptr<SemaObj> t2);
  void dump_sema_stack();
  void error(int line, string message);
  void addRule(int rule, int last_snd_token, int last_const_idx, int line,
               unordered_map<int, Obj> &constants);
};

#endif
