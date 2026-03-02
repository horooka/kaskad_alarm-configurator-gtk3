#include "ParamSelect.h"
#include <cstring>

// тип значения для TPassport: 1 = дискретный, 2 = аналоговый
int param_get_value_type(StMachins* m, const SParamData* d) {
    if (!m || !d || d->DirType != pttKlogic)
        return 0;
    unsigned char ptype = 0;
    if (d->type == 0) {
        StDynParam cParam;
        if (m->GetDynParam(static_cast<unsigned char>(d->MaId),
                          static_cast<unsigned short>(d->CntId),
                          d->ParamId, &cParam))
            ptype = cParam.type;
    } else if (d->type == 1) {
        StDynParamControl cParam;
        if (m->GetDynParamControl(static_cast<unsigned char>(d->MaId),
                                  static_cast<unsigned short>(d->CntId),
                                  d->ParamId, &cParam))
            ptype = cParam.type;
    }
    if (ptype == 0)
        return 0;
    return (ptype == 1) ? 2 : 1;  // дискретный -> 2, аналоговый -> 1
}

void params_list_from_tree_child(StMachins* m, unsigned char ma_id,
                                 unsigned short cnt_id,
                                 const StTreeChild* tree_child,
                                 std::vector<SParamData>& out) {
    out.clear();
    if (!m || !tree_child || !tree_child->params)
        return;
    for (unsigned int i = 0; i < tree_child->TreeChHead.paramcount; i++) {
        SParamData pd;
        pd.DirType = pttKlogic;
        pd.MaId = ma_id;
        pd.CntId = cnt_id;
        pd.ParamId = tree_child->params[i];
        pd.type = 0;
        out.push_back(pd);
    }
}

void params_list_control(StMachins* m, unsigned char ma_id,
                         unsigned short cnt_id, std::vector<SParamData>& out) {
    out.clear();
    if (!m)
        return;
    StParamsControl* pc = m->GetParamsControl(ma_id, cnt_id);
    if (!pc || !pc->Params)
        return;
    for (unsigned int i = 0; i < pc->ParHead.paramcount; i++) {
        SParamData pd;
        pd.DirType = pttKlogic;
        pd.MaId = ma_id;
        pd.CntId = cnt_id;
        pd.ParamId = pc->Params[i].id;
        pd.type = 1;
        out.push_back(pd);
    }
}

void param_data_to_passport(const SParamData* d, int value_type, TPassport* p) {
    if (!d || !p)
        return;
    memset(p, 0, sizeof(TPassport));
    p->StationID = static_cast<uint8_t>(d->MaId);
    p->PaspID = static_cast<uint16_t>(d->ParamId);
    p->ValueType = static_cast<uint8_t>(value_type);
    if (d->DirType == pttKlogic) {
        p->TypePasp = (d->type == 0) ? ParamTypeKlogic : ParamTypeKlogicControll;
        p->GroupID = static_cast<uint8_t>(d->CntId);
    }
}

bool passport_to_param_data(const TPassport* p, SParamData* d) {
    if (!p || !d)
        return false;
    memset(d, 0, sizeof(SParamData));
    d->MaId = p->StationID;
    d->ParamId = p->PaspID;
    d->CntId = p->GroupID;
    if (p->TypePasp == ParamTypeKlogic) {
        d->DirType = pttKlogic;
        d->type = 0;
        return true;
    }
    if (p->TypePasp == ParamTypeKlogicControll) {
        d->DirType = pttKlogic;
        d->type = 1;
        return true;
    }
    return false;
}
