#include "Semantics.h"

#include <iostream>
#include <variant>

#include "ParserTables.h"
#include "Util.h"

using std::cout, std::endl;

Semantics::Semantics() { file = fopen("out.gen", "w"); }
Semantics::~Semantics() { fclose(file); }

void Semantics::dump_sema_stack() {
  auto s = sema_stack;
  cout << "DUMP SEMA STACK " << endl;
  while (!s.empty()) {
    cout << "NON_TERM: " << s.top().type << endl;
    s.pop();
  }
}

int Semantics::check_types(shared_ptr<SemaObj> t1, shared_ptr<SemaObj> t2) {
  if (t1 == t2)
    return 1;
  else if (t1 == universal_t || t2 == universal_t)
    return 1;
  else if (t1->k_type == UNIVERSAL_ || t2->k_type == UNIVERSAL_)
    return 1;
  else if (t1->k_type == ALIAS_TYPE_ && t2->k_type != ALIAS_TYPE_) {
    return check_types(std::get<VariantObjAlias>(t1->obj).base_type, t2);
  } else if (t1->k_type != ALIAS_TYPE_ && t2->k_type == ALIAS_TYPE_) {
    return check_types(t1, std::get<VariantObjAlias>(t2->obj).base_type);
  } else if (t1->k_type == t2->k_type) {
    if (t1->k_type == ALIAS_TYPE_) {
      return check_types(std::get<VariantObjAlias>(t1->obj).base_type,
                         std::get<VariantObjAlias>(t2->obj).base_type);
    } else if (t1->k_type == ARRAY_TYPE_) {
      if (std::get<VariantObjArray>(t1->obj).num_el ==
          std::get<VariantObjArray>(t2->obj).num_el) {
        return check_types(std::get<VariantObjArray>(t1->obj).el_type,
                           std::get<VariantObjArray>(t2->obj).el_type);
      }
    } else if (t1->k_type == STRUCT_TYPE_) {
      auto f1 = std::get<VariantObjStruct>(t1->obj).fields;
      auto f2 = std::get<VariantObjStruct>(t2->obj).fields;

      auto f1_iter = f1.begin();
      auto f2_iter = f2.begin();
      while (f1_iter != f1.end() && f2_iter != f2.end()) {
        if (!check_types(std::get<VariantObjTypeble>((*f1_iter)->obj).type,
                         std::get<VariantObjTypeble>((*f2_iter)->obj).type))
          return 0;
      }
      return (f1_iter == f1.end() && f2_iter == f2.end());
    }
  }
  return 0;
}

void Semantics::error(int line, string message) {
  err = true;
  report(line, "", message);
}

void Semantics::addRule(int rule, int last_snd_token, int last_const_idx,
                        int line, unordered_map<int, Obj> &constants) {
  static shared_ptr<SemaObj> curr_func;
  static int num_funcs = 0;
  static int curr_label = 0;
  static int func_var_pos = 0;

  switch (rule) {
    case R_LDE_0:
    case R_LDE_1:
    case R_DE_0:
    case R_DE_1:
      break;

    case R_T_0:
      sema_stack.push({1, _T, VariantAttrTypeble{int_t}});
      break;

    case R_T_1:
      sema_stack.push({1, _T, VariantAttrTypeble{char_t}});
      break;

    case R_T_2:
      sema_stack.push({1, _T, VariantAttrTypeble{bool_t}});
      break;

    case R_T_3:
      sema_stack.push({1, _T, VariantAttrTypeble{string_t}});
      break;

    case R_T_4: {
      auto idu = sema_stack.top();
      sema_stack.pop();

      auto p = std::get<VariantAttrId>(idu.attr).obj;
      SemaAttr t(0, _T, VariantAttrTypeble{p});

      if (IS_TYPE_KIND(p->k_type) || p->k_type == UNIVERSAL_) {
        if (p->k_type == ALIAS_TYPE_)
          t.size = std::get<VariantObjAlias>(p->obj).n_size;
        else if (p->k_type == ARRAY_TYPE_)
          t.size = std::get<VariantObjArray>(p->obj).n_size;
        else if (p->k_type == STRUCT_TYPE_)
          t.size = std::get<VariantObjStruct>(p->obj).n_size;
      } else {
        t.attr = VariantAttrTypeble{universal_t};
        t.size = 0;
        error(line, "Sema error: Type expected");
      }
      sema_stack.push(t);
      break;
    }

    case R_DT_0: {
      auto t_nonterm = sema_stack.top();
      sema_stack.pop();
      auto num = sema_stack.top();
      sema_stack.pop();
      auto idd = sema_stack.top();
      sema_stack.pop();

      auto p = std::get<VariantAttrId>(idd.attr).obj;
      auto n = std::get<VariantAttrNum>(num.attr).val;
      auto t = std::get<VariantAttrTypeble>(t_nonterm.attr).type;

      p->k_type = ARRAY_TYPE_;
      VariantObjArray obj = {
          .el_type = t, .num_el = n, .n_size = n * t_nonterm.size};
      p->obj = obj;
      break;
    }

    case R_DT_1: {
      auto dc = sema_stack.top();
      sema_stack.pop();
      auto idd = sema_stack.top();
      sema_stack.pop();

      auto p = std::get<VariantAttrId>(idd.attr).obj;
      auto dc_list = std::get<VariantAttrList>(dc.attr).list;

      p->k_type = STRUCT_TYPE_;
      VariantObjStruct obj = {.fields = dc_list, .n_size = dc.size};
      p->obj = obj;

      symtable.endBlock();
      break;
    }

    case R_DT_2: {
      auto t_nonterm = sema_stack.top();
      sema_stack.pop();
      auto idd = sema_stack.top();
      sema_stack.pop();

      auto p = std::get<VariantAttrId>(idd.attr).obj;
      auto t = std::get<VariantAttrTypeble>(t_nonterm.attr).type;

      p->k_type = ALIAS_TYPE_;
      VariantObjAlias obj = {.base_type = t, .n_size = t_nonterm.size};
      p->obj = obj;
      break;
    }

    case R_DC_0: {
      auto t_nonterm = sema_stack.top();
      sema_stack.pop();
      auto li = sema_stack.top();
      sema_stack.pop();
      auto dc1 = sema_stack.top();
      sema_stack.pop();

      auto p_list = std::get<VariantAttrList>(li.attr).list;
      auto dc1_list = std::get<VariantAttrList>(dc1.attr).list;
      auto t = std::get<VariantAttrTypeble>(t_nonterm.attr).type;
      auto n = dc1.size;

      for (auto &p : p_list) {
        if (p->k_type != NO_KIND_DEF_) break;

        p->k_type = FIELD_;
        VariantObjTypeble obj = {
            .type = t, .n_idx = n, .n_size = t_nonterm.size};
        n += t_nonterm.size;
        p->obj = obj;
      }

      auto dc0 = SemaAttr(n, _DC, VariantAttrList{dc1_list});
      sema_stack.push(dc0);
      break;
    }

    case R_DC_1: {
      auto t_nonterm = sema_stack.top();
      sema_stack.pop();
      auto li = sema_stack.top();
      sema_stack.pop();

      auto p_list = std::get<VariantAttrList>(li.attr).list;
      auto t = std::get<VariantAttrTypeble>(t_nonterm.attr).type;
      auto n = 0;

      for (auto &p : p_list) {
        if (p->k_type != NO_KIND_DEF_) break;

        p->k_type = FIELD_;
        VariantObjTypeble obj = {
            .type = t, .n_idx = n, .n_size = t_nonterm.size};
        n += t_nonterm.size;
        p->obj = obj;
      }

      auto dc = SemaAttr(n, _DC, VariantAttrList{p_list});
      sema_stack.push(dc);
      break;
    }

    case R_DF_0:
      symtable.endBlock();
      break;

    case R_LP_0: {
      auto t_nonterm = sema_stack.top();
      sema_stack.pop();
      auto idd = sema_stack.top();
      sema_stack.pop();
      auto lp1 = sema_stack.top();
      sema_stack.pop();

      auto p = std::get<VariantAttrId>(idd.attr).obj;
      auto t = std::get<VariantAttrTypeble>(t_nonterm.attr).type;
      auto lp0_list = std::get<VariantAttrList>(lp1.attr).list;
      auto n = lp1.size;

      p->k_type = PARAM_;
      VariantObjTypeble obj = {.type = t, .n_idx = n, .n_size = t_nonterm.size};
      p->obj = obj;

      auto lp0 = SemaAttr(n + t_nonterm.size, _LP, VariantAttrList{lp0_list});
      sema_stack.push(lp0);
      break;
    }

    case R_LP_1: {
      auto t_nonterm = sema_stack.top();
      sema_stack.pop();
      auto idd = sema_stack.top();
      sema_stack.pop();

      auto p = std::get<VariantAttrId>(idd.attr).obj;
      auto t = std::get<VariantAttrTypeble>(t_nonterm.attr).type;

      p->k_type = PARAM_;
      VariantObjTypeble obj = {.type = t, .n_idx = 0, .n_size = t_nonterm.size};
      p->obj = obj;

      auto lp = SemaAttr(t_nonterm.size, _LP, VariantAttrList{{p}});
      sema_stack.push(lp);
      break;
    }

    case R_B_0: {
      fprintf(file, "END_FUNC\n");
      int currentPos = ftell(file);
      fseek(file, func_var_pos, SEEK_SET);
      fprintf(file, "%02d\n", std::get<VariantObjFunc>(curr_func->obj).n_vars);
      fseek(file, currentPos, SEEK_SET);
      break;
    }

    case R_LDV_0:
    case R_LDV_1:
    case R_LS_0:
    case R_LS_1:
      break;

    case R_DV_0: {
      auto t_nonterm = sema_stack.top();
      sema_stack.pop();
      auto li = sema_stack.top();
      sema_stack.pop();

      auto p_list = std::get<VariantAttrList>(li.attr).list;
      auto t = std::get<VariantAttrTypeble>(t_nonterm.attr).type;
      auto n = std::get<VariantObjFunc>(curr_func->obj).n_vars;

      for (auto &p : p_list) {
        if (p->k_type != NO_KIND_DEF_) break;

        p->k_type = VAR_;
        VariantObjTypeble obj = {
            .type = t, .n_idx = n, .n_size = t_nonterm.size};
        n += t_nonterm.size;
        p->obj = obj;
      }

      auto new_obj = std::get<VariantObjFunc>(curr_func->obj);
      new_obj.n_vars = n;
      curr_func->obj = new_obj;
      break;
    }

    case R_LI_0: {
      auto idd = sema_stack.top();
      sema_stack.pop();
      auto li1 = sema_stack.top();
      sema_stack.pop();

      auto p = std::get<VariantAttrList>(li1.attr).list;
      p.push_back(std::get<VariantAttrId>(idd.attr).obj);

      auto li0 = SemaAttr(-1, _LI, VariantAttrList{p});
      sema_stack.push(li0);
      break;
    }

    case R_LI_1: {
      auto idd = sema_stack.top();
      sema_stack.pop();

      auto p = std::get<VariantAttrId>(idd.attr).obj;

      auto li = SemaAttr(-1, _LI, VariantAttrList{{p}});
      sema_stack.push(li);
      break;
    }

    case R_MT_0: {
      int l = curr_label++;
      auto mt_variant = SemaAttr(0, _MT, VariantAttrLabel{l});
      sema_stack.push(mt_variant);

      fprintf(file, "\tTJMP_FW L%d\n", l);
      break;
    }

    case R_S_0: {
      auto me = sema_stack.top();
      sema_stack.pop();
      auto mt = sema_stack.top();
      sema_stack.pop();
      auto e = sema_stack.top();
      sema_stack.pop();

      int l = std::get<VariantAttrLabel>(me.attr).label;
      auto t = std::get<VariantAttrTypeble>(e.attr).type;

      if (!check_types(t, bool_t)) error(line, "Sema error: Invalid type");

      fprintf(file, "L%d:\n", l);
      break;
    }

    case R_S_1: {
      auto mt = sema_stack.top();
      sema_stack.pop();
      auto e = sema_stack.top();
      sema_stack.pop();

      int l = std::get<VariantAttrLabel>(mt.attr).label;
      auto t = std::get<VariantAttrTypeble>(e.attr).type;

      if (!check_types(t, bool_t)) error(line, "Sema error: Invalid type");

      fprintf(file, "L%d:\n", l);
      break;
    }

    case R_ME_0: {
      auto mt = sema_stack.top();

      int l1 = std::get<VariantAttrLabel>(mt.attr).label;
      int l2 = curr_label++;

      auto me = SemaAttr(-1, _ME, VariantAttrLabel{l2});
      sema_stack.push(me);

      fprintf(file, "\tJMP_FW L%d\nL:%d\n", l2, l1);
      break;
    }

    case R_MW_0: {
      int l = curr_label++;

      auto mw = SemaAttr(-1, _MW, VariantAttrLabel{l});
      sema_stack.push(mw);

      fprintf(file, "L%d:\n", l);
      break;
    }

    case R_S_2: {
      auto mt = sema_stack.top();
      sema_stack.pop();
      auto e = sema_stack.top();
      sema_stack.pop();
      auto mw = sema_stack.top();
      sema_stack.pop();

      int l1 = std::get<VariantAttrLabel>(mw.attr).label;
      int l2 = std::get<VariantAttrLabel>(mt.attr).label;
      auto t = std::get<VariantAttrTypeble>(e.attr).type;

      if (!check_types(t, bool_t)) error(line, "Sema error: Invalid type");

      fprintf(file, "\tJMP_BW L%d\nL%d:\n", l1, l2);
      break;
    }

    case R_S_3: {
      auto e = sema_stack.top();
      sema_stack.pop();
      auto mw = sema_stack.top();
      sema_stack.pop();

      int l = std::get<VariantAttrLabel>(mw.attr).label;
      auto t = std::get<VariantAttrTypeble>(e.attr).type;

      if (!check_types(t, bool_t)) error(line, "Sema error: Invalid type");

      fprintf(file, "\tNOT\n\tTJMP_BW L%d:\n", l);
      break;
    }

    case R_S_4: {
      symtable.endBlock();
      break;
    }

    case R_S_5: {
      auto e = sema_stack.top();
      sema_stack.pop();
      auto lv = sema_stack.top();
      sema_stack.pop();

      auto t = std::get<VariantAttrTypeble>(lv.attr).type;

      if (!check_types(std::get<VariantAttrTypeble>(e.attr).type,
                       std::get<VariantAttrTypeble>(lv.attr).type))
        error(line, "Sema error: Type mismatch");

      fprintf(file, "\tSTORE_REF %d\n",
              std::get<VariantObjAlias>(t->obj).n_size);
      break;
    }

    case R_S_6:
    case R_S_7:
      break;
    case R_S_8: {
      auto e = sema_stack.top();
      sema_stack.pop();

      if (!check_types(std::get<VariantAttrTypeble>(e.attr).type,
                       std::get<VariantObjFunc>(curr_func->obj).ret_type))
        error(line, "Sema error: Type mismatch");

      fprintf(file, "\tRET\n");
      break;
    }

    case R_E_0: {
      auto l = sema_stack.top();
      sema_stack.pop();
      auto e1 = sema_stack.top();
      sema_stack.pop();

      if (!check_types(std::get<VariantAttrTypeble>(l.attr).type, bool_t))
        error(line, "Sema error: Invalid type");
      if (!check_types(std::get<VariantAttrTypeble>(e1.attr).type, bool_t))
        error(line, "Sema error: Invalid type");

      auto e0 = SemaAttr(-1, _E, VariantAttrTypeble{bool_t});
      sema_stack.push(e0);

      fprintf(file, "\tAND\n");
      break;
    }

    case R_E_1: {
      auto l = sema_stack.top();
      sema_stack.pop();
      auto e1 = sema_stack.top();
      sema_stack.pop();

      if (!check_types(std::get<VariantAttrTypeble>(l.attr).type, bool_t))
        error(line, "Sema error: Invalid type");
      if (!check_types(std::get<VariantAttrTypeble>(e1.attr).type, bool_t))
        error(line, "Sema error: Invalid type");

      auto e0 = SemaAttr(-1, _E, VariantAttrTypeble{bool_t});
      sema_stack.push(e0);

      fprintf(file, "\tOR\n");
      break;
    }

    case R_E_2: {
      auto l = sema_stack.top();
      sema_stack.pop();

      auto p = std::get<VariantAttrTypeble>(l.attr).type;

      auto e = SemaAttr(-1, _E, VariantAttrTypeble{p});
      sema_stack.push(e);
      break;
    }

    case R_L_0: {
      auto r = sema_stack.top();
      sema_stack.pop();
      auto l1 = sema_stack.top();
      sema_stack.pop();

      if (!check_types(std::get<VariantAttrTypeble>(r.attr).type,
                       std::get<VariantAttrTypeble>(l1.attr).type))
        error(line, "Sema error: Type mismatch");

      auto l0 = SemaAttr(-1, _L, VariantAttrTypeble{bool_t});
      sema_stack.push(l0);

      fprintf(file, "\tLT\n");
      break;
    }

    case R_L_1: {
      auto r = sema_stack.top();
      sema_stack.pop();
      auto l1 = sema_stack.top();
      sema_stack.pop();

      if (!check_types(std::get<VariantAttrTypeble>(r.attr).type,
                       std::get<VariantAttrTypeble>(l1.attr).type))
        error(line, "Sema error: Type mismatch");

      auto l0 = SemaAttr(-1, _L, VariantAttrTypeble{bool_t});
      sema_stack.push(l0);

      fprintf(file, "\tGT\n");
      break;
    }

    case R_L_2: {
      auto r = sema_stack.top();
      sema_stack.pop();
      auto l1 = sema_stack.top();
      sema_stack.pop();

      if (!check_types(std::get<VariantAttrTypeble>(r.attr).type,
                       std::get<VariantAttrTypeble>(l1.attr).type))
        error(line, "Sema error: Type mismatch");

      auto l0 = SemaAttr(-1, _L, VariantAttrTypeble{bool_t});
      sema_stack.push(l0);

      fprintf(file, "\tLE\n");
      break;
    }

    case R_L_3: {
      auto r = sema_stack.top();
      sema_stack.pop();
      auto l1 = sema_stack.top();
      sema_stack.pop();

      if (!check_types(std::get<VariantAttrTypeble>(r.attr).type,
                       std::get<VariantAttrTypeble>(l1.attr).type))
        error(line, "Sema error: Type mismatch");

      auto l0 = SemaAttr(-1, _L, VariantAttrTypeble{bool_t});
      sema_stack.push(l0);

      fprintf(file, "\tGE\n");
      break;
    }

    case R_L_4: {
      auto r = sema_stack.top();
      sema_stack.pop();
      auto l1 = sema_stack.top();
      sema_stack.pop();

      if (!check_types(std::get<VariantAttrTypeble>(r.attr).type,
                       std::get<VariantAttrTypeble>(l1.attr).type))
        error(line, "Sema error: Type mismatch");

      auto l0 = SemaAttr(-1, _L, VariantAttrTypeble{bool_t});
      sema_stack.push(l0);

      fprintf(file, "\tEQ\n");
      break;
    }

    case R_L_5: {
      auto r = sema_stack.top();
      sema_stack.pop();
      auto l1 = sema_stack.top();
      sema_stack.pop();

      if (!check_types(std::get<VariantAttrTypeble>(r.attr).type,
                       std::get<VariantAttrTypeble>(l1.attr).type))
        error(line, "Sema error: Type mismatch");

      auto l0 = SemaAttr(-1, _L, VariantAttrTypeble{bool_t});
      sema_stack.push(l0);

      fprintf(file, "\tNE\n");
      break;
    }

    case R_L_6: {
      auto r = sema_stack.top();
      sema_stack.pop();

      auto p = std::get<VariantAttrTypeble>(r.attr).type;

      auto l0 = SemaAttr(-1, _L, VariantAttrTypeble{p});
      sema_stack.push(l0);
      break;
    }

    case R_R_0: {
      auto y = sema_stack.top();
      sema_stack.pop();
      auto r1 = sema_stack.top();
      sema_stack.pop();

      auto p = std::get<VariantAttrTypeble>(r1.attr).type;

      if (!check_types(std::get<VariantAttrTypeble>(y.attr).type,
                       std::get<VariantAttrTypeble>(r1.attr).type))
        error(line, "Sema error: Type mismatch");

      if (!check_types(std::get<VariantAttrTypeble>(r1.attr).type, int_t) &&
          !check_types(std::get<VariantAttrTypeble>(r1.attr).type, string_t))
        error(line, "Sema error: Invalid type");

      auto r0 = SemaAttr(-1, _R, VariantAttrTypeble{p});
      sema_stack.push(r0);

      fprintf(file, "\tADD\n");
      break;
    }

    case R_R_1: {
      auto y = sema_stack.top();
      sema_stack.pop();
      auto r1 = sema_stack.top();
      sema_stack.pop();

      auto p = std::get<VariantAttrTypeble>(r1.attr).type;

      if (!check_types(std::get<VariantAttrTypeble>(y.attr).type,
                       std::get<VariantAttrTypeble>(r1.attr).type))
        error(line, "Sema error: Type mismatch");

      if (!check_types(std::get<VariantAttrTypeble>(r1.attr).type, int_t))
        error(line, "Sema error: Invalid type");

      auto r0 = SemaAttr(-1, _R, VariantAttrTypeble{p});
      sema_stack.push(r0);

      fprintf(file, "\tSUB\n");
      break;
    }

    case R_R_2: {
      auto y = sema_stack.top();
      sema_stack.pop();

      auto p = std::get<VariantAttrTypeble>(y.attr).type;

      auto r0 = SemaAttr(-1, _R, VariantAttrTypeble{p});
      sema_stack.push(r0);
      break;
    }

    case R_Y_0: {
      auto f = sema_stack.top();
      sema_stack.pop();
      auto y = sema_stack.top();
      sema_stack.pop();

      auto p = std::get<VariantAttrTypeble>(y.attr).type;

      if (!check_types(std::get<VariantAttrTypeble>(y.attr).type,
                       std::get<VariantAttrTypeble>(f.attr).type))
        error(line, "Sema error: Type mismatch");

      if (!check_types(std::get<VariantAttrTypeble>(y.attr).type, int_t))
        error(line, "Sema error: Invalid type");

      auto y0 = SemaAttr(-1, _Y, VariantAttrTypeble{p});
      sema_stack.push(y0);

      fprintf(file, "\tMUL\n");
      break;
    }

    case R_Y_1: {
      auto f = sema_stack.top();
      sema_stack.pop();
      auto y = sema_stack.top();
      sema_stack.pop();

      auto p = std::get<VariantAttrTypeble>(y.attr).type;

      if (!check_types(std::get<VariantAttrTypeble>(y.attr).type,
                       std::get<VariantAttrTypeble>(f.attr).type))
        error(line, "Sema error: Type mismatch");

      if (!check_types(std::get<VariantAttrTypeble>(y.attr).type, int_t))
        error(line, "Sema error: Invalid type");

      auto y0 = SemaAttr(-1, _Y, VariantAttrTypeble{p});
      sema_stack.push(y0);

      fprintf(file, "\tDIV\n");
      break;
    }

    case R_Y_2: {
      auto f = sema_stack.top();
      sema_stack.pop();

      auto p = std::get<VariantAttrTypeble>(f.attr).type;

      auto y0 = SemaAttr(-1, _Y, VariantAttrTypeble{p});
      sema_stack.push(y0);
      break;
    }

    case R_F_0: {
      auto lv = sema_stack.top();
      sema_stack.pop();

      auto p = std::get<VariantAttrTypeble>(lv.attr).type;
      auto n = std::get<VariantObjAlias>(p->obj).n_size;

      auto f = SemaAttr(-1, _F, VariantAttrTypeble{p});
      sema_stack.push(f);

      fprintf(file, "\tDE_REF %d\n", n);
      break;
    }

    case R_F_1: {
      auto lv = sema_stack.top();
      sema_stack.pop();

      auto p = std::get<VariantAttrTypeble>(lv.attr).type;

      if (!check_types(std::get<VariantAttrTypeble>(lv.attr).type, int_t))
        error(line, "Sema error: Invalid type");

      auto f = SemaAttr(-1, _F, VariantAttrTypeble{int_t});
      sema_stack.push(f);

      fprintf(file, "\tDUP\n\tDUP\n\tDE_REF 1\n");
      fprintf(file, "\tINC\n\tSTORE_REF 1\n\tDE_REF 1\n");
      break;
    }

    case R_F_2: {
      auto lv = sema_stack.top();
      sema_stack.pop();

      auto p = std::get<VariantAttrTypeble>(lv.attr).type;

      if (!check_types(std::get<VariantAttrTypeble>(lv.attr).type, int_t))
        error(line, "Sema error: Invalid type");

      auto f = SemaAttr(-1, _F, VariantAttrTypeble{int_t});
      sema_stack.push(f);

      fprintf(file, "\tDUP\n\tDUP\n\tDE_REF 1\n");
      fprintf(file, "\tDEC\n\tSTORE_REF 1\n\tDE_REF 1\n");
      break;
    }

    case R_F_3: {
      auto lv = sema_stack.top();
      sema_stack.pop();

      auto p = std::get<VariantAttrTypeble>(lv.attr).type;

      if (!check_types(std::get<VariantAttrTypeble>(lv.attr).type, int_t))
        error(line, "Sema error: Invalid type");

      auto f = SemaAttr(-1, _F, VariantAttrTypeble{int_t});
      sema_stack.push(f);

      fprintf(file, "\tDUP\n\tDUP\n\tDE_REF 1\n\tINC\n");
      fprintf(file, "\tSTORE_REF 1\n\tDE_REF 1\n\tDEC\n");
      break;
    }

    case R_F_4: {
      auto lv = sema_stack.top();
      sema_stack.pop();

      auto p = std::get<VariantAttrTypeble>(lv.attr).type;

      if (!check_types(std::get<VariantAttrTypeble>(lv.attr).type, int_t))
        error(line, "Sema error: Invalid type");

      auto f = SemaAttr(-1, _F, VariantAttrTypeble{int_t});
      sema_stack.push(f);

      fprintf(file, "\tDUP\n\tDUP\tDE_REF 1\n\tDEC\n");
      fprintf(file, "\tSTORE_REF 1\tDE_REF 1\n\tINC\n");
      break;
    }

    case R_F_5: {
      auto e = sema_stack.top();
      sema_stack.pop();

      auto p = std::get<VariantAttrTypeble>(e.attr).type;

      auto f = SemaAttr(-1, _F, VariantAttrTypeble{p});
      sema_stack.push(f);
      break;
    }

    case R_F_6: {
      auto le = sema_stack.top();
      sema_stack.pop();
      auto mc = sema_stack.top();
      sema_stack.pop();
      auto idu = sema_stack.top();
      sema_stack.pop();

      auto p = std::get<VariantAttrId>(idu.attr).obj;
      auto mc_type = std::get<VariantAttrMc>(mc.attr).type;

      auto f = SemaAttr(-1, _F, VariantAttrTypeble{mc_type});
      sema_stack.push(f);

      fprintf(file, "\tCALL %d\n", std::get<VariantObjFunc>(p->obj).n_idx);
      break;
    }

    case R_F_7: {
      auto f1 = sema_stack.top();
      sema_stack.pop();

      auto t = std::get<VariantAttrTypeble>(f1.attr).type;

      if (!check_types(std::get<VariantAttrTypeble>(f1.attr).type, int_t))
        error(line, "Sema error: Invalid type");

      auto f = SemaAttr(-1, _F, VariantAttrTypeble{t});
      sema_stack.push(f);

      fprintf(file, "\tNEG\n");
      break;
    }

    case R_F_8: {
      auto f1 = sema_stack.top();
      sema_stack.pop();

      auto t = std::get<VariantAttrTypeble>(f1.attr).type;

      if (!check_types(std::get<VariantAttrTypeble>(f1.attr).type, int_t))
        error(line, "Sema error: Invalid type");

      auto f = SemaAttr(-1, _F, VariantAttrTypeble{t});
      sema_stack.push(f);

      fprintf(file, "\tNOT\n");
      break;
    }

    case R_F_9: {
      auto true_nontem = sema_stack.top();
      sema_stack.pop();

      auto f = SemaAttr(-1, _F, VariantAttrTypeble{bool_t});
      sema_stack.push(f);

      fprintf(file, "\tLOAD_CONST %d\n", last_const_idx);
      break;
    }

    case R_F_10: {
      auto false_nontem = sema_stack.top();
      sema_stack.pop();

      auto f = SemaAttr(-1, _F, VariantAttrTypeble{bool_t});
      sema_stack.push(f);

      fprintf(file, "\tLOAD_CONST %d\n", last_const_idx);
      break;
    }

    case R_F_11: {
      auto char_nontem = sema_stack.top();
      sema_stack.pop();

      auto f = SemaAttr(-1, _F, VariantAttrTypeble{char_t});
      sema_stack.push(f);

      fprintf(file, "\tLOAD_CONST %d\n", last_const_idx);
      break;
    }

    case R_F_12: {
      auto string_nontem = sema_stack.top();
      sema_stack.pop();

      auto f = SemaAttr(-1, _F, VariantAttrTypeble{string_t});
      sema_stack.push(f);

      fprintf(file, "\tLOAD_CONST %d\n", last_const_idx);
      break;
    }

    case R_F_13: {
      auto num_nontem = sema_stack.top();
      sema_stack.pop();

      auto f = SemaAttr(-1, _F, VariantAttrTypeble{int_t});
      sema_stack.push(f);

      fprintf(file, "\tLOAD_CONST %d\n", last_const_idx);
      break;
    }

    case R_LE_0: {
      auto e = sema_stack.top();
      sema_stack.pop();
      auto le1 = sema_stack.top();
      sema_stack.pop();

      auto le1_attr = std::get<VariantAttrLe>(le1.attr);
      auto le0_variant =
          VariantAttrLe{.type = nullptr, .param = {}, .err = false, .n = 0};
      if (!le1_attr.err) {
        if (le1_attr.param.empty()) {
          error(line, "Sema error: To many arguments");
          le0_variant.err = true;
        } else {
          if (!check_types(
                  std::get<VariantObjTypeble>(le1_attr.param[0]->obj).type,
                  std::get<VariantAttrTypeble>(e.attr).type))
            error(line, "Sema error: Invalid type");

          le0_variant.param = vector<shared_ptr<SemaObj>>(
              le1_attr.param.begin() + 1, le1_attr.param.end());
          le0_variant.n = le1_attr.n + 1;
        }
      }

      auto le0 = SemaAttr(-1, _LE, le0_variant);
      sema_stack.push(le0);
      break;
    }

    case R_LE_1: {
      auto e = sema_stack.top();
      sema_stack.pop();
      auto mc = sema_stack.top();
      sema_stack.pop();

      // Stuff it back MC
      sema_stack.push(mc);

      auto mc_attr = std::get<VariantAttrMc>(mc.attr);
      auto le_variant = VariantAttrLe{
          .type = nullptr, .param = {}, .err = mc_attr.err, .n = 0};
      int n = 1;
      if (!mc_attr.err) {
        if (mc_attr.param.empty()) {
          error(line, "Sema error: To many arguments");
          le_variant.err = true;
        } else {
          if (!check_types(
                  std::get<VariantObjTypeble>(mc_attr.param[0]->obj).type,
                  std::get<VariantAttrTypeble>(e.attr).type))
            error(line, "Sema error: Invalid type");

          le_variant.param = vector<shared_ptr<SemaObj>>(
              mc_attr.param.begin() + 1, mc_attr.param.end());
          le_variant.n = n + 1;
        }
      }

      auto le = SemaAttr(-1, _LE, le_variant);
      sema_stack.push(le);
      break;
    }

    case R_LV_0: {
      auto id = sema_stack.top();
      sema_stack.pop();
      auto lv1 = sema_stack.top();
      sema_stack.pop();

      auto t = std::get<VariantAttrTypeble>(lv1.attr).type;
      auto lv_variant = VariantAttrTypeble{.type = nullptr};

      if (t->k_type != STRUCT_TYPE_) {
        if (t->k_type != UNIVERSAL_) {
          error(line, "Sema error: Kind not struct");
        }
        lv_variant.type = universal_t;
      } else {
        auto struct_fields = std::get<VariantObjStruct>(t->obj).fields;
        auto p = struct_fields.begin();

        while (p != struct_fields.end()) {
          if ((*p)->name == std::get<VariantAttrId>(id.attr).name) break;

          p++;
        }

        if (p == struct_fields.end()) {
          lv_variant.type = universal_t;
          error(line, "Sema error: Field not declared");
        } else {
          lv_variant.type = std::get<VariantObjTypeble>((*p)->obj).type;

          shared_ptr<SemaObj> base_type = nullptr;
          if (holds_alternative<VariantObjAlias>(lv_variant.type->obj))
            base_type =
                std::get<VariantObjAlias>(lv_variant.type->obj).base_type;

          VariantObjAlias type = {
              .base_type = base_type,
              .n_size = std::get<VariantObjTypeble>((*p)->obj).n_size};
          type.n_size = std::get<VariantObjTypeble>((*p)->obj).n_size;
          lv_variant.type->obj = type;

          fprintf(file, "\tADD %d\n",
                  std::get<VariantObjTypeble>((*p)->obj).n_idx);
        }
      }

      auto lv = SemaAttr(-1, _LV, lv_variant);
      sema_stack.push(lv);
      break;
    }

    case R_LV_1: {
      auto e = sema_stack.top();
      sema_stack.pop();
      auto lv1 = sema_stack.top();
      sema_stack.pop();

      auto t = std::get<VariantAttrTypeble>(lv1.attr).type;
      auto lv_variant = VariantAttrTypeble{.type = nullptr};

      if (check_types(t, string_t))
        lv_variant.type = char_t;
      else if (t->k_type != ARRAY_TYPE_) {
        if (t->k_type != UNIVERSAL_) {
          error(line, "Sema error: Kind not array");
        }
        lv_variant.type = universal_t;
      } else {
        lv_variant.type = std::get<VariantObjArray>(t->obj).el_type;

        fprintf(file, "\tMUL %d\n",
                std::get<VariantObjTypeble>(lv_variant.type->obj).n_size);
      }

      if (!check_types(std::get<VariantAttrTypeble>(e.attr).type, int_t)) {
        error(line, "Sema error: Invalid index type");
      }

      auto lv = SemaAttr(-1, _LV, lv_variant);
      sema_stack.push(lv);
      break;
    }

    case R_LV_2: {
      auto idu = sema_stack.top();
      sema_stack.pop();

      auto p = std::get<VariantAttrId>(idu.attr).obj;
      auto lv_variant = VariantAttrTypeble{.type = nullptr};

      if (p->k_type != VAR_ && p->k_type != PARAM_) {
        if (p->k_type != UNIVERSAL_) {
          error(line, "Sema error: Kind not var");
        }
        lv_variant.type = universal_t;
      } else {
        lv_variant.type = std::get<VariantObjTypeble>(p->obj).type;

        shared_ptr<SemaObj> base_type = nullptr;
        if (holds_alternative<VariantObjAlias>(lv_variant.type->obj))
          base_type = std::get<VariantObjAlias>(lv_variant.type->obj).base_type;

        VariantObjAlias type = {
            .base_type = base_type,
            .n_size = std::get<VariantObjTypeble>(p->obj).n_size};
        lv_variant.type->obj = type;

        fprintf(file, "\tLOAD_REF %d\n",
                std::get<VariantObjTypeble>(p->obj).n_idx);
      }

      auto lv = SemaAttr(-1, _LV, lv_variant);
      sema_stack.push(lv);
      break;
    }

    case R_NB_0: {
      symtable.newBlock();
      break;
    }

    case R_NF_0: {
      auto idd = sema_stack.top();

      auto p = std::get<VariantAttrId>(idd.attr).obj;
      auto f_obj = VariantObjFunc{};

      p->k_type = FUNCTION_;
      f_obj.n_params = 0;
      f_obj.ret_type = nullptr;
      f_obj.params = {};
      f_obj.n_vars = 0;
      f_obj.n_idx = num_funcs++;
      p->obj = f_obj;

      symtable.newBlock();
      break;
    }

    case R_MF_0: {
      auto t_nonterm = sema_stack.top();
      sema_stack.pop();
      auto lp = sema_stack.top();
      sema_stack.pop();
      auto idd = sema_stack.top();
      sema_stack.pop();

      // Shove it back into the stack
      sema_stack.push(idd);
      sema_stack.push(lp);
      sema_stack.push(t_nonterm);

      auto p = std::get<VariantAttrId>(idd.attr).obj;
      auto t = std::get<VariantAttrTypeble>(t_nonterm.attr).type;
      auto lp_list = std::get<VariantAttrList>(lp.attr).list;
      auto f_obj = std::get<VariantObjFunc>(p->obj);

      p->k_type = FUNCTION_;
      f_obj.ret_type = t;
      f_obj.n_params = lp.size;
      f_obj.params = lp_list;
      f_obj.n_vars = 0;
      p->obj = f_obj;

      curr_func = p;

      fprintf(file, "BEGIN_FUNC %d, %d, ", f_obj.n_idx, f_obj.n_params);
      func_var_pos = ftell(file);
      fprintf(file, "   ");
      break;
    }

    case R_MC_0: {
      auto idu = sema_stack.top();

      auto p = std::get<VariantAttrId>(idu.attr).obj;
      auto mc_variant =
          VariantAttrMc{.type = nullptr, .param = {}, .err = false};

      if (p->k_type != FUNCTION_) {
        error(line, "Sema error: Kind not function");
        mc_variant.type = universal_t;
        mc_variant.param = {};
        mc_variant.err = true;
      } else {
        mc_variant.type = std::get<VariantObjFunc>(p->obj).ret_type;
        mc_variant.param = std::get<VariantObjFunc>(p->obj).params;
        mc_variant.err = false;
      }

      auto mc = SemaAttr(-1, _MC, mc_variant);
      sema_stack.push(mc);
      break;
    }

    case R_IDD_0: {
      int name = last_snd_token;
      SemaObj id(name, NO_KIND_DEF_, std::monostate());

      auto symbol = symtable.search(id);
      if (symbol.has_value()) {
        error(line, "Sema error: Redeclared");
      } else
        symbol = symtable.define(id);

      sema_stack.push(
          {1, _IDD, VariantAttrId{.obj = *symbol.value(), .name = name}});
      break;
    }

    case R_IDU_0: {
      int name = last_snd_token;
      SemaObj id(name, NO_KIND_DEF_, std::monostate());

      auto symbol = symtable.find(id);
      if (!symbol.has_value()) {
        error(line, "Sema error: Not declared");
        symbol = symtable.define(id);
      }

      sema_stack.push(
          {1, _IDU, VariantAttrId{.obj = *symbol.value(), .name = name}});
      break;
    }

    case R_TRUE_0: {
      auto true_variant = VariantAttrBool{.type = bool_t, .val = true};
      auto true_nontem = SemaAttr(-1, _TRUE, true_variant);
      sema_stack.push(true_nontem);
      break;
    }

    case R_FALSE_0: {
      auto false_variant = VariantAttrBool{.type = bool_t, .val = false};
      auto false_nontem = SemaAttr(-1, _FALSE, false_variant);
      sema_stack.push(false_nontem);
      break;
    }

    case R_C_0: {
      char val = std::get<char>(constants[last_const_idx]);
      auto char_variant =
          VariantAttrChar{.type = char_t, .pos = last_snd_token, .val = val};
      auto char_nontem = SemaAttr(-1, _C, char_variant);
      sema_stack.push(char_nontem);
      break;
    }

    case R_STR_0: {
      string val = std::get<string>(constants[last_const_idx]);
      auto string_variant = VariantAttrString{
          .type = string_t, .pos = last_snd_token, .val = val};
      auto string_nontem = SemaAttr(-1, _STR, string_variant);
      sema_stack.push(string_nontem);
      break;
    }

    case R_NUM_0: {
      int val = std::get<int>(constants[last_const_idx]);
      auto num_variant =
          VariantAttrNum{.type = int_t, .pos = last_snd_token, .val = val};
      auto num_nontem = SemaAttr(-1, _NUM, num_variant);
      sema_stack.push(num_nontem);
      break;
    }
  }
}
